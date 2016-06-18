#pragma once
#include <vector>
#include <unordered_map>

#include "game/entity_id.h"
#include "misc/timer.h"

#include "game/resources/behaviour_tree.h"
#include "game/assets/behaviour_tree_id.h"

namespace components {
	struct behaviour_tree {
		struct instance {
			resources::behaviour_tree::state_of_tree_instance state;
			assets::behaviour_tree_id tree_id;
		};

		std::vector<instance> concurrent_trees;
	};
}

