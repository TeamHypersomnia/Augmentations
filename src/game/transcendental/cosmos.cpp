#include <sol2/sol.hpp>

#include "cosmos.h"

#include "augs/readwrite/streams.h"

#include "game/stateless_systems/movement_system.h"
#include "game/stateless_systems/visibility_system.h"
#include "game/stateless_systems/pathfinding_system.h"
#include "game/stateless_systems/animation_system.h"
#include "game/stateless_systems/input_system.h"
#include "game/stateless_systems/gun_system.h"
#include "game/stateless_systems/crosshair_system.h"
#include "game/stateless_systems/rotation_copying_system.h"
#include "game/stateless_systems/position_copying_system.h"
#include "game/stateless_systems/missile_system.h"
#include "game/stateless_systems/destroy_system.h"
#include "game/stateless_systems/particles_existence_system.h"
#include "game/stateless_systems/behaviour_tree_system.h"
#include "game/stateless_systems/car_system.h"
#include "game/stateless_systems/driver_system.h"
#include "game/stateless_systems/item_system.h"
#include "game/stateless_systems/force_joint_system.h"
#include "game/stateless_systems/intent_contextualization_system.h"
#include "game/stateless_systems/trace_system.h"
#include "game/stateless_systems/melee_system.h"
#include "game/stateless_systems/sentience_system.h"
#include "game/stateless_systems/destruction_system.h"
#include "game/stateless_systems/sound_existence_system.h"
#include "game/stateless_systems/hand_fuse_system.h"

#include "game/transcendental/logic_step.h"
#include "game/transcendental/entity_handle.h"

#include "game/organization/all_messages_includes.h"
#include "game/organization/all_component_includes.h"

#include "game/transcendental/cosmic_delta.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/inventory/inventory_utils.h"

#include "augs/readwrite/lua_readwrite.h"
#include "augs/readwrite/byte_readwrite.h"
#include "generated/introspectors.h"

const cosmos cosmos::empty;

void cosmos::clear() {
	*this = empty;
}

void cosmos::complete_reinference() {
	auto scope = measure_scope(profiler.complete_reinference);
	
	destroy_inferred_state_completely();
	create_inferred_state_completely();
}

void cosmos::destroy_inferred_state_completely() {
	inferential.~all_inferential_systems();
	new (&inferential) all_inferential_systems;

	const auto n = significant.entity_pool.capacity();

	inferential.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

void cosmos::create_inferred_state_completely() {
	for (const auto& ordered_pair : guid_to_id) {
		create_inferred_state_for(operator[](ordered_pair.second));
	}

	inferential.for_each([this](auto& sys) {
		sys.create_additional_inferred_state(significant.meta.global);
	});
}

void cosmos::destroy_inferred_state_of(const const_entity_handle h) {
	auto destructor = [h](auto& sys) {
		sys.destroy_inferred_state_of(h);
	};

	inferential.for_each(destructor);
}

void cosmos::create_inferred_state_for(const const_entity_handle h) {
	if (h.is_inferred_state_activated()) {
		auto constructor = [h](auto& sys) {
			sys.create_inferred_state_for(h);
		};

		inferential.for_each(constructor);
	}
}

cosmos::cosmos(const cosmic_pool_size_type reserved_entities) {
	reserve_storage_for_entities(reserved_entities);
	significant.meta.global.si.set_pixels_per_meter(100.f);
}

/* TODO: Make comparisons somehow work with debug name pointers */
#if !(DEBUG_TRACK_ENTITY_NAME && STATICALLY_ALLOCATE_ENTITIES_NUM)
bool cosmos::operator==(const cosmos& b) const {
	return significant == b.significant;
}

bool cosmos::operator!=(const cosmos& b) const {
	return !operator==(b);
}
#endif

cosmos& cosmos::operator=(const cosmos_significant_state& b) {
	{
		auto scope = measure_scope(profiler.duplication);
		significant = b;
	}

	refresh_for_new_significant_state();
	return *this;
}

#if COSMOS_TRACKS_GUIDS

void cosmos::assign_next_guid(const entity_handle new_entity) {
	const auto this_guid = significant.meta.next_entity_guid.value++;

	guid_to_id[this_guid] = new_entity;
	new_entity.get<components::guid>().value = this_guid;
}

void cosmos::clear_guid(const entity_handle cleared) {
	guid_to_id.erase(get_guid(cleared));
}

void cosmos::remap_guids() {
	guid_to_id.clear();

	for_each_entity_id([this](const entity_id id) {
		guid_to_id[get_guid(operator[](id))] = id;
	});
}

#endif

void cosmos::refresh_for_new_significant_state() {
#if COSMOS_TRACKS_GUIDS
	remap_guids();
#endif
	complete_reinference();
}

void cosmos::complete_reinference(const const_entity_handle h) {
	destroy_inferred_state_of(h);
	create_inferred_state_for(h);
}

void cosmos::reserve_storage_for_entities(const cosmic_pool_size_type n) {
	get_entity_pool().reserve(n);
	reserve_all_components(n);

	inferential.for_each([n](auto& sys) {
		sys.reserve_caches_for_entities(n);
	});
}

std::wstring cosmos::summary() const {
	return typesafe_sprintf(L"Entities: %x\n", get_entities_count());
}

rng_seed_type cosmos::get_rng_seed_for(const entity_id id) const {
	rng_seed_type transform_hash = 0;
	const auto tr = operator[](id).get_logic_transform();
	transform_hash = static_cast<rng_seed_type>(std::abs(tr.pos.x)*100.0);
	transform_hash += static_cast<rng_seed_type>(std::abs(tr.pos.y)*100.0);
	transform_hash += static_cast<rng_seed_type>(std::abs(tr.rotation)*100.0);

	return operator[](id).get_guid() + transform_hash;
}

double cosmos::get_total_seconds_passed(const double view_interpolation_ratio) const {
	return get_total_seconds_passed() + get_fixed_delta().per_second(view_interpolation_ratio);
}

double cosmos::get_total_seconds_passed() const {
	return significant.meta.now.step * get_fixed_delta().in_seconds<double>();
}

decltype(augs::stepped_timestamp::step) cosmos::get_total_steps_passed() const {
	return significant.meta.now.step;
}

augs::stepped_timestamp cosmos::get_timestamp() const {
	return significant.meta.now;
}

augs::delta cosmos::get_fixed_delta() const {
	return significant.meta.delta;
}

void cosmos::set_steps_per_second(const unsigned steps) {
	significant.meta.delta = augs::delta::steps_per_second(steps);
}

unsigned cosmos::get_steps_per_second() const {
	return get_fixed_delta().in_steps_per_second();
}

entity_handle cosmos::allocate_new_entity() {
	if (get_entity_pool().full()) {
		throw std::runtime_error("Entities should be controllably reserved to avoid invalidation of entity_handles.");
	}

	return { *this, get_entity_pool().allocate() };
}

entity_handle cosmos::create_entity(const std::string& name) {
	return create_entity(to_wstring(name));
}

entity_handle cosmos::create_entity(const std::wstring& name_str) {
	auto new_entity = allocate_new_entity();
	new_entity.set_name(name_str);

#if COSMOS_TRACKS_GUIDS
	assign_next_guid(new_entity);
#endif
	return new_entity;
}

#if COSMOS_TRACKS_GUIDS
entity_handle cosmos::create_entity_with_specific_guid(const entity_guid specific_guid) {
	const auto new_entity = allocate_new_entity();

	guid_to_id[specific_guid] = new_entity;
	new_entity.get<components::guid>().value = specific_guid;
	return new_entity;
}
#endif

entity_handle cosmos::clone_entity(const entity_id source_entity_id) {
	entity_handle source_entity = operator[](source_entity_id);

	if (source_entity.dead()) {
		return operator[](entity_id());
	}

	ensure(
		!source_entity.has<components::child>() 
		&& "Cloning of entities with child component is not yet supported"
	);

	const auto new_entity = allocate_new_entity();
	
	clone_all_components_except<
		/*
			These components will be cloned shortly,
			with due care to each of them.
		*/
		components::fixtures,
		components::child,

		/*
			Let us keep the inferred state of the new entity disabled for a while,
			to avoid unnecessary reinference.
		*/
		components::all_inferred_state
	>(new_entity, source_entity);

#if COSMOS_TRACKS_GUIDS
	assign_next_guid(new_entity);
#endif

	if (new_entity.has<components::item>()) {
		new_entity.get<components::item>().current_slot.unset();
	}

	new_entity.make_cloned_child_entities_recursive(source_entity_id);
	
	if (source_entity.has<components::fixtures>()) {
		/*
			Copy all properties from the component of the source entity except the owner_body field.
			Exactly as if we were creating that component by hand.
		*/

		components::fixtures fixtures = source_entity.get<components::fixtures>().get_raw_component();
		const auto owner_of_the_source = fixtures.owner_body;
		fixtures.owner_body = entity_id();

		new_entity += fixtures;

		/*
			Only now assign the owner_body in a controllable manner.
		*/

		const bool source_owns_itself = owner_of_the_source == source_entity;

		if (source_owns_itself) {
			/*
				If the fixtures of the source entity were owned by the same entity,
				let the cloned entity also own itself
			*/
			new_entity.set_owner_body(new_entity);
		}
		else {
			/*
				If the fixtures of the source entity were owned by a different entity,
				let the cloned entity also be owned by that different entity
			*/
			new_entity.set_owner_body(owner_of_the_source);
		}
	}

	if (source_entity.is_inferred_state_activated()) {
		new_entity.get<components::all_inferred_state>().set_activated(true);
	}

	return new_entity;
}

void cosmos::delete_entity(const entity_id e) {
	const auto handle = operator[](e);
	
	if (handle.dead()) {
		return;
	}

	const bool should_deactivate_inferred_state_to_avoid_repeated_reinference 
		= handle.is_inferred_state_activated()
	;

	if (should_deactivate_inferred_state_to_avoid_repeated_reinference) {
		handle.get<components::all_inferred_state>().set_activated(false);
	}

#if COSMOS_TRACKS_GUIDS
	clear_guid(handle);
#endif
	// now manipulation of an entity without all_inferred_state component won't trigger redundant reinference

	const auto maybe_fixtures = handle.find<components::fixtures>();

	if (maybe_fixtures != nullptr) {
		const auto owner_body = maybe_fixtures.get_owner_body();

		const bool should_release_dependency = owner_body != handle;

		if (should_release_dependency) {
			maybe_fixtures.set_owner_body(handle);
		}
	}

	free_all_components(operator[](e));
	get_entity_pool().free(e);

	/*
		Unregister that id as a parent from the relational system
	*/

	inferential.get<relational_system>().handle_deletion_of_potential_parent(e);
}

void cosmos::advance_and_queue_destructions(const logic_step step) {
	auto& cosmos = step.cosm;
	const auto& delta = step.get_delta();
	auto& performance = profiler;
	
	auto logic_scope = measure_scope(performance.logic);

	physics_system::contact_listener listener(step.cosm);
	
	perform_transfers(step.input.entropy.transfer_requests, step);
	
	performance.entropy_length.measure(step.input.entropy.length());

	sentience_system().cast_spells(step);

	input_system().make_input_messages(step);

	intent_contextualization_system().contextualize_crosshair_action_intents(step);
	intent_contextualization_system().contextualize_use_button_intents(step);
	intent_contextualization_system().contextualize_movement_intents(step);

	movement_system().set_movement_flags_from_input(step);
	movement_system().apply_movement_forces(step.cosm);

	gun_system().consume_gun_intents(step);
	gun_system().launch_shots_due_to_pressed_triggers(step);

	car_system().set_steering_flags_from_intents(step);
	car_system().apply_movement_forces(step);

	melee_system().consume_melee_intents(step);
	melee_system().initiate_and_update_moves(step);

	force_joint_system().apply_forces_towards_target_entities(step);
	item_system().handle_throw_item_intents(step);
	hand_fuse_system().detonate_fuses(step);

	{
		auto scope = measure_scope(performance.physics);

		listener.during_step = true;
		inferential.get<physics_system>().step_and_set_new_transforms(step);
		listener.during_step = false;
	}

	rotation_copying_system().update_rotations(step.cosm);
	position_copying_system().update_transforms(step);

	trace_system().lengthen_sprites_of_traces(step);

	crosshair_system().generate_crosshair_intents(step);
	crosshair_system().apply_crosshair_intents_to_base_offsets(step);
	crosshair_system().apply_base_offsets_to_crosshair_transforms(step);

//	item_system().translate_gui_intents_to_transfer_requests(step);
	item_system().start_picking_up_items(step);
	item_system().pick_up_touching_items(step);

	missile_system().detonate_colliding_missiles(step);
	missile_system().detonate_expired_missiles(step);

	destruction_system().generate_damages_from_forceful_collisions(step);
	destruction_system().apply_damages_and_split_fixtures(step);
	
	sentience_system().regenerate_values_and_advance_spell_logic(step);
	sentience_system().apply_damage_and_generate_health_events(step);
	inferential.get<physics_system>().post_and_clear_accumulated_collision_messages(step);
	sentience_system().cooldown_aimpunches(step);

	driver_system().release_drivers_due_to_requests(step);
	driver_system().assign_drivers_who_touch_wheels(step);
	driver_system().release_drivers_due_to_ending_contact_with_wheel(step);

	particles_existence_system().game_responses_to_particle_effects(step);

	sound_existence_system().create_sounds_from_game_events(step);
	// gui_system().translate_game_events_for_hud(step);

	{
		auto scope = measure_scope(performance.visibility);
		visibility_system().respond_to_visibility_information_requests(step);
	}

	{
		auto scope = measure_scope(performance.ai);
		behaviour_tree_system().evaluate_trees(step);
	}

	{
		auto scope = measure_scope(performance.pathfinding);
		pathfinding_system().advance_pathfinding_sessions(step);
	}

	auto& transfers = step.transient.messages.get_queue<item_slot_transfer_request>();
	perform_transfers(transfers, step);

	particles_existence_system().displace_streams_and_destroy_dead_streams(step);
	sound_existence_system().destroy_dead_sounds(step);

	trace_system().destroy_outdated_traces(step);

	const size_t queued_before_marking_num = step.transient.messages.get_queue<messages::queue_destruction>().size();

	destroy_system().mark_queued_entities_and_their_children_for_deletion(step);

	trace_system().spawn_finishing_traces_for_deleted_entities(step);

	listener.~contact_listener();

	movement_system().generate_movement_events(step);

	animation_system().game_responses_to_animation_messages(step);

	animation_system().handle_animation_messages(step);
	animation_system().progress_animation_states(step);

	//position_copying_system().update_transforms(step);
	//rotation_copying_system().update_rotations(step.cosm);

	profiler.raycasts.measure(inferential.get<physics_system>().ray_casts_since_last_step);

	++significant.meta.now.step;

	const size_t queued_at_end_num = step.transient.messages.get_queue<messages::queue_destruction>().size();

	ensure_eq(queued_at_end_num, queued_before_marking_num);
}

void cosmos::perform_deletions(const logic_step step) {
	destroy_system().perform_deletions(step);
}

namespace augs {
	void write_object(augs::stream& into, const cosmos& cosm) {
		auto& profiler = cosm.profiler;

		{
			augs::output_stream_reserver reserver;

			{
				auto scope = measure_scope(profiler.size_calculation_pass);
				write(reserver, cosm.significant);
			}

			auto scope = measure_scope(profiler.memory_allocation_pass);
			into.reserve(into.get_write_pos() + reserver.size());
		}

		{
			auto scope = measure_scope(profiler.serialization_pass);
			write(into, cosm.significant);
		}
	}

	void read_object(augs::stream& from, cosmos& cosm) {
		auto& profiler = cosm.profiler;

		auto refresh_when_done = augs::make_scope_guard([&cosm]() {
			cosm.refresh_for_new_significant_state();
		});

		auto scope = measure_scope(profiler.deserialization_pass);
		read(from, cosm.significant);
	}

	void write_object(sol::table ar, const cosmos& cosm) {
#if TODO
		write(ar, cosm.significant.meta);
		
		for (const auto& ent : cosm.get_entity_pool()) {

			for_each_component_type(
				[&](auto c) {
				using component_type = decltype(c);


			});
		}
#endif
	}

	void read_object(sol::table ar, cosmos& cosm) {

	}
}
