#include "augs/drawing/drawing.h"
#include "augs/templates/introspect.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/organization/all_component_includes.h"

#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/aabb_highlighter.h"


void aabb_highlighter::update(const augs::delta dt) {
	timer += dt.in_milliseconds();
	timer = fmod(timer, cycle_duration_ms);
}

bool aabb_highlighter::is_hoverable(const const_entity_handle e) {
	if (!e.is_inferred_state_activated()) {
		return false;
	}

	if (const auto maybe_render = e.find<components::render>()) {
		switch (maybe_render->layer) {
			case render_layer::SMALL_DYNAMIC_BODY: return true;
			case render_layer::DYNAMIC_BODY: return true;
			case render_layer::CAR_INTERIOR: return true;
			case render_layer::CAR_WHEEL: return true;
			default: return false;
		}
	
		return false;
	}

	return false;
}

void aabb_highlighter::draw(const aabb_highlighter_drawing_input in) const {
	ltrb aabb;

	const auto aabb_expansion_lambda = [&aabb, in](const const_entity_handle e) {
		if (!is_hoverable(e)) {
			return false;
		}

		const auto new_aabb = e.get_aabb(in.interp);

		if (aabb.good() && new_aabb.good()) {
			aabb.contain(new_aabb);
		}
		else if (new_aabb.good()) {
			aabb = new_aabb;
		}
		
		return true;
	};

	aabb_expansion_lambda(in.subject);
	in.subject.for_each_child_entity_recursive(aabb_expansion_lambda);

	const auto lesser_dimension = std::min(aabb.w(), aabb.h());
	
	float length_decrease = 0.f;

	if (lesser_dimension < scale_down_when_aabb_no_bigger_than) {
		length_decrease = std::min(smallest_length - 4, scale_down_when_aabb_no_bigger_than - lesser_dimension);
	}
	
	const auto adjusted_biggest_length = biggest_length - length_decrease;
	const auto adjusted_smallest = smallest_length - length_decrease;

	const int current_length{ static_cast<int>(augs::interp(adjusted_biggest_length, adjusted_smallest, timer / cycle_duration_ms)) };
	const int gap_animated_expansion{ static_cast<int>(current_length - adjusted_smallest) };

	const vec2 gap = {
		base_gap + gap_animated_expansion + length_decrease,
		base_gap + gap_animated_expansion + length_decrease
	};
	
	aabb.expand_from_center(gap);

	const auto as = vec2i(aabb.get_size());
	const auto ap = vec2(in.camera.to_screen_space(in.screen_size, aabb.get_position()));

	if (aabb.good()) {
		in.output
			.aabb(ltrb(ap, vec2(vec2i { current_length, 1 })), cyan)
			.aabb(ltrb(ap, vec2(vec2i { 1, current_length })), cyan)

			.aabb(ltrb(ap + vec2(vec2i(as.x - current_length, 0)), vec2(vec2i { current_length, 1 })), cyan)
			.aabb(ltrb(ap + vec2(vec2i(as.x - 1, 0)), vec2(vec2i { 1, current_length })), cyan)

			.aabb(ltrb(ap + vec2(vec2i(0, as.y - current_length)), vec2(vec2i { 1, current_length })), cyan)
			.aabb(ltrb(ap + vec2(vec2i(0, as.y - 1)), vec2(vec2i { current_length, 1 })), cyan)

			.aabb(ltrb(ap + vec2(vec2i(as.x - current_length, as.y - 1)), vec2(vec2i { current_length, 1 })), cyan)
			.aabb(ltrb(ap + vec2(vec2i(as.x - 1, as.y - current_length)), vec2(vec2i { 1, current_length })), cyan)
		;
	}
}
