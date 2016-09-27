#include "physics_component.h"
#include "substance_component.h"

#include <Box2D\Box2D.h>

#include "augs/graphics/renderer.h"
#include "fixtures_component.h"

#include "augs/math/vec2.h"
#include "game/transcendental/cosmos.h"
#include "game/temporary_systems/physics_system.h"
#include "augs/ensure.h"
#include "game/transcendental/entity_handle.h"

typedef components::physics P;

components::physics::physics(components::transform t) {
	set_transform(t);
}

void components::physics::set_transform(const components::transform& t) {
	t.to_si_space().to_box2d_transforms(transform, sweep);
}

template<bool C>
bool basic_physics_synchronizer<C>::is_constructed() const {
	return handle.get_cosmos().temporary_systems.get<physics_system>().is_constructed_rigid_body(handle);
}

template<bool C>
maybe_const_ref_t<C, rigid_body_cache>& basic_physics_synchronizer<C>::get_cache() const {
	return handle.get_cosmos().temporary_systems.get<physics_system>().get_rigid_body_cache(handle);
}

void component_synchronizer<false, P>::resubstantiation() const {
	handle.get_cosmos().partial_resubstantiation<physics_system>(handle);
}

void component_synchronizer<false, P>::set_body_type(components::physics::type t) const {
	component.body_type = t;
	resubstantiation();
}


void component_synchronizer<false, P>::set_activated(bool flag) const {
	component.activated = flag;
	resubstantiation();
}


void component_synchronizer<false, P>::set_velocity(vec2 pixels) const {
	component.velocity = pixels * PIXELS_TO_METERSf;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearVelocity(component.velocity);
}

void component_synchronizer<false, P>::set_angular_velocity(float degrees) const {
	component.angular_velocity = DEG_TO_RADf * degrees;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearVelocity(component.velocity);
}

void component_synchronizer<false, P>::set_linear_damping(float damping) const {
	component.linear_damping = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearDamping(damping);
}


void component_synchronizer<false, P>::set_angular_damping(float damping) const {
	component.angular_damping = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetAngularDamping(damping);
}


void component_synchronizer<false, P>::set_linear_damping_vec(vec2 damping) const {
	component.linear_damping_vec = damping;

	if (!is_constructed())
		return;

	get_cache().body->SetLinearDampingVec(damping);
}

void component_synchronizer<false, P>::apply_force(vec2 pixels) const {
	apply_force(pixels, vec2(0, 0), true);
}

void component_synchronizer<false, P>::apply_force(vec2 pixels, vec2 center_offset, bool wake) const {
	ensure(is_constructed());

	if (pixels.is_epsilon(2.f))
		return;

	vec2 force = pixels * PIXELS_TO_METERSf;
	vec2 location = get_cache().body->GetWorldCenter() + (center_offset * PIXELS_TO_METERSf);

	force *= handle.get_cosmos().get_fixed_delta().in_seconds();

	get_cache().body->ApplyLinearImpulse(force, location, wake);
	component.angular_velocity = get_cache().body->GetAngularVelocity();
	component.velocity = get_cache().body->GetLinearVelocity();

	if (renderer::get_current().debug_draw_forces && force.non_zero()) {
		auto& lines = renderer::get_current().logic_lines;
		lines.draw_green(location * METERS_TO_PIXELSf + force * METERS_TO_PIXELSf, location * METERS_TO_PIXELSf);
	}
}

void component_synchronizer<false, P>::apply_impulse(vec2 pixels) const {
	apply_impulse(pixels, vec2(0, 0), true);
}

void component_synchronizer<false, P>::apply_impulse(vec2 pixels, vec2 center_offset, bool wake) const {
	ensure(is_constructed());

	if (pixels.is_epsilon(2.f))
		return;

	vec2 force = pixels * PIXELS_TO_METERSf;
	vec2 location = get_cache().body->GetWorldCenter() + (center_offset * PIXELS_TO_METERSf);

	get_cache().body->ApplyLinearImpulse(force, location, true);
	component.angular_velocity = get_cache().body->GetAngularVelocity();
	component.velocity = get_cache().body->GetLinearVelocity();

	if (renderer::get_current().debug_draw_forces && force.non_zero()) {
		auto& lines = renderer::get_current().logic_lines;
		lines.draw_green(location * METERS_TO_PIXELSf + force * METERS_TO_PIXELSf, location * METERS_TO_PIXELSf);
	}
}

void component_synchronizer<false, P>::apply_angular_impulse(float imp) const {
	ensure(is_constructed());
	get_cache().body->ApplyAngularImpulse(imp, true);
	component.angular_velocity = get_cache().body->GetAngularVelocity();
}

template<bool C>
float basic_physics_synchronizer<C>::get_mass() const {
	ensure(is_constructed());
	return get_cache().body->GetMass();
}

template<bool C>
float basic_physics_synchronizer<C>::get_angle() const {
	return component.sweep.a * RAD_TO_DEGf;
}

template<bool C>
float basic_physics_synchronizer<C>::get_angular_velocity() const {
	return component.angular_velocity * RAD_TO_DEGf;
}

template<bool C>
float basic_physics_synchronizer<C>::get_inertia() const {
	ensure(is_constructed());
	return get_cache().body->GetInertia();
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_position() const {
	return METERS_TO_PIXELSf * component.transform.p;
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_mass_position() const {
	ensure(is_constructed());
	return METERS_TO_PIXELSf * get_cache().body->GetWorldCenter();
}

template<bool C>
vec2 basic_physics_synchronizer<C>::velocity() const {
	return component.velocity * METERS_TO_PIXELSf;
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_world_center() const {
	ensure(is_constructed());
	return METERS_TO_PIXELSf * get_cache().body->GetWorldCenter();
}

template<bool C>
vec2 basic_physics_synchronizer<C>::get_aabb_size() const {
	ensure(is_constructed());
	b2AABB aabb;
	aabb.lowerBound.Set(FLT_MAX, FLT_MAX);
	aabb.upperBound.Set(-FLT_MAX, -FLT_MAX);

	b2Fixture* fixture = get_cache().body->GetFixtureList();

	while (fixture != nullptr) {
		aabb.Combine(aabb, fixture->GetAABB(0));
		fixture = fixture->GetNext();
	}

	return vec2(aabb.upperBound.x - aabb.lowerBound.x, aabb.upperBound.y - aabb.lowerBound.y);
}

template<bool C>
P::type basic_physics_synchronizer<C>::get_body_type() const {
	return component.body_type;
}

template<bool C>
bool basic_physics_synchronizer<C>::is_activated() const {
	return component.activated;
}

void component_synchronizer<false, P>::set_transform(entity_id id) const {
	set_transform(handle.get_cosmos()[id].logic_transform());
}

void component_synchronizer<false, P>::set_transform(const components::transform& transform) const {
	component.set_transform(transform);

	if (!is_constructed())
		return;

	get_cache().body->m_xf = component.transform;
	get_cache().body->m_sweep = component.sweep;
}

template<bool C>
std::vector<basic_entity_handle<C>> basic_physics_synchronizer<C>::get_fixture_entities() const {
	return handle.get_fixture_entities();
}

template<bool C>
bool basic_physics_synchronizer<C>::test_point(vec2 v) const {
	ensure(is_constructed());
	return get_cache().body->TestPoint(v * PIXELS_TO_METERSf);
}

template class basic_physics_synchronizer<false>;
template class basic_physics_synchronizer<true>;