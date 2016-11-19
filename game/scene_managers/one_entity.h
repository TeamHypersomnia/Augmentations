#pragma once
#include <vector>
#include "game/transcendental/entity_id.h"

namespace augs {
	struct machine_entropy;
}

struct cosmic_entropy;
#include "game/transcendental/step_declaration.h"
class cosmos;
class viewing_session;
struct input_context;

namespace scene_managers {
	class one_entity {
		std::vector<entity_id> characters;

		unsigned current_character = 0;
		entity_id world_camera;

	public:
		void populate_world_with_entities(logic_step&);
		cosmic_entropy make_cosmic_entropy(augs::machine_entropy, const input_context&, const cosmos&);
		entity_id get_controlled_entity() const;

		void configure_view(viewing_session&) const;
	};
}