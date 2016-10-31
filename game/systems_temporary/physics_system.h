#pragma once
#include "3rdparty/Box2D/Box2D.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/physics_component.h"
#include "game/components/transform_component.h"

#include "game/messages/collision_message.h"

#include "game/detail/convex_partitioned_shape.h"

#include <set>

class cosmos;
class logic_step;

struct rigid_body_cache {
	b2Body* body = nullptr;
	std::vector<int> correspondent_colliders_caches;
};

struct colliders_cache {
	std::vector<std::vector<b2Fixture*>> fixtures_per_collider;
	int correspondent_rigid_body_cache = -1;
};

class physics_system {
	std::vector<colliders_cache> colliders_caches;
	std::vector<rigid_body_cache> rigid_body_caches;

	std::pair<size_t, size_t> map_fixture_pointer_to_indices(b2Fixture* f, const const_entity_handle&);
	convex_partitioned_shape::convex_poly::destruction_data& map_fixture_pointer_to_convex_poly(b2Fixture* f, const entity_handle&);

	void reserve_caches_for_entities(size_t n);
	void fixtures_construct(const_entity_handle);
	void construct(const_entity_handle);
	void destruct(const_entity_handle);

	friend class cosmos;
	friend class component_synchronizer<false, components::physics>;
	friend class component_synchronizer<true, components::physics>;
	friend class component_synchronizer<false, components::fixtures>;
	friend class component_synchronizer<true, components::fixtures>;
	template<bool> friend class basic_physics_synchronizer;
	template<bool> friend class basic_fixtures_synchronizer;

	bool is_constructed_rigid_body(const_entity_handle) const;
	bool is_constructed_colliders(const_entity_handle) const;

	rigid_body_cache& get_rigid_body_cache(entity_id);
	colliders_cache& get_colliders_cache(entity_id);
	const rigid_body_cache& get_rigid_body_cache(entity_id) const;
	const colliders_cache& get_colliders_cache(entity_id) const;

	std::vector<messages::collision_message> accumulated_messages;
public:
	struct raycast_output {
		vec2 intersection, normal;
		bool hit = false;
		unversioned_entity_id what_entity;
	};

	struct edge_edge_output {
		vec2 intersection;
		bool hit;
	};

	struct query_output {
		struct queried_result {
			b2Fixture* fixture;
			b2Vec2 location;

			bool operator<(const queried_result& b) const { return fixture < b.fixture; }
		};

		std::set<b2Body*> bodies;
		std::set<unversioned_entity_id> entities;
		std::set<queried_result> details;

		query_output& operator+=(const query_output& b) {
			bodies.insert(b.bodies.begin(), b.bodies.end());
			entities.insert(b.entities.begin(), b.entities.end());
			details.insert(b.details.begin(), b.details.end());

			return *this;
		}
	};

	struct query_aabb_output {
		std::set<b2Body*> bodies;
		std::set<unversioned_entity_id> entities;
		std::vector<b2Fixture*> fixtures;
	};

	physics_system();

	std::vector<raycast_output> ray_cast_all_intersections(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id()) const;

	static edge_edge_output edge_edge_intersection(vec2 p1_meters, vec2 p2_meters, vec2 edge_p1, vec2 edge_p2);

	raycast_output ray_cast(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id()) const;
	raycast_output ray_cast_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity = entity_id()) const;
	
	vec2 push_away_from_walls(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity = entity_id()) const;
	float get_closest_wall_intersection(vec2 position, float radius, int ray_amount, b2Filter filter, entity_id ignore_entity = entity_id()) const;

	query_aabb_output query_square(vec2 p1_meters, float side_meters, b2Filter filter, entity_id ignore_entity = entity_id()) const;
	query_aabb_output query_square_px(vec2 p1, float side, b2Filter filter, entity_id ignore_entity = entity_id()) const;

	query_aabb_output query_aabb(vec2 p1_meters, vec2 p2_meters, b2Filter filter, entity_id ignore_entity = entity_id()) const;
	query_aabb_output query_aabb_px(vec2 p1, vec2 p2, b2Filter filter, entity_id ignore_entity = entity_id()) const;

	query_output query_body(const_entity_handle, b2Filter filter, entity_id ignore_entity = entity_id()) const;

	query_output query_polygon(const std::vector<vec2>& vertices, b2Filter filter, entity_id ignore_entity = entity_id()) const;
	query_output query_shape(b2Shape*, b2Filter filter, entity_id ignore_entity = entity_id()) const;
	
	void step_and_set_new_transforms(logic_step&);
	void post_and_clear_accumulated_collision_messages(logic_step&);

	mutable int ray_casts_since_last_step = 0;

	// b2world causes a stack overflow due to a large stack allocator, therefore it must be dynamically allocated
	std::unique_ptr<b2World> b2world;

	physics_system& operator=(const physics_system&);
	physics_system& operator=(physics_system&&) = delete;
	physics_system(const physics_system&) = delete;
	physics_system(physics_system&&) = delete;
private:	
	/* callback structure used in QueryAABB function to get all shapes near-by */
	struct query_aabb_input : b2QueryCallback {
		entity_id ignore_entity;
		b2Filter filter;

		query_aabb_output out;

		bool ReportFixture(b2Fixture* fixture) override;
	};

	struct raycast_input : public b2RayCastCallback {
		entity_id subject;
		b2Filter subject_filter;

		bool save_all = false;
		raycast_output output;
		std::vector<raycast_output> outputs;

		bool ShouldRaycast(b2Fixture* fixture);
		float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction);
	};

	struct contact_listener : public b2ContactListener {
		void BeginContact(b2Contact* contact) override;
		void EndContact(b2Contact* contact) override;
		void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
		void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

		bool during_step = false;

		cosmos& cosm;
		physics_system& get_sys() const;

		contact_listener(const contact_listener&) = delete;
		contact_listener(contact_listener&&) = delete;
		
		contact_listener(cosmos&);
		~contact_listener();
		
		contact_listener& operator=(const contact_listener&) = delete;
		contact_listener& operator=(contact_listener&&) = delete;
	};

	void rechoose_owner_friction_body(entity_handle);
	void recurential_friction_handler(logic_step&, b2Body* const entity, b2Body* const friction_owner);
};
