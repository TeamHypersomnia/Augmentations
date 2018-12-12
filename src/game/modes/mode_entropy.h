#pragma once
#include <map>

#include "game/cosmos/cosmic_entropy.h"
#include "game/modes/mode_commands/team_choice.h"
#include "game/modes/mode_commands/item_purchase.h"
#include "augs/entity_system/storage_for_message_queues.h"
#include "game/modes/mode_player_id.h"
#include "game/modes/mode_commands/mode_entropy_structs.h"

#if MODE_ENTROPY_HAS_QUEUES
using mode_player_commands = augs::storage_for_message_queues<
	mode_commands::item_purchase
>;
#endif

class cosmos;

struct mode_player_entropy {
	// GEN INTROSPECTOR struct mode_player_entropy
	std::optional<mode_commands::team_choice> team_choice;
	std::optional<mode_commands::item_purchase> item_purchase;
	// END GEN INTROSPECTOR

	void clear();
	bool is_set() const;

	mode_player_entropy& operator+=(const mode_player_entropy&);
};

struct total_mode_player_entropy {
	// GEN INTROSPECTOR struct total_mode_player_entropy
	mode_player_entropy mode;
	cosmic_player_entropy cosmic;
	// END GEN INTROSPECTOR
};

using total_client_entropy = total_mode_player_entropy;

struct mode_entropy_general {
	// GEN INTROSPECTOR struct mode_entropy_general
	std::optional<add_player_input> added_player;
	std::optional<mode_player_id> removed_player;
	all_general_mode_commands_variant special_command;
	// END GEN INTROSPECTOR

	mode_entropy_general& operator+=(const mode_entropy_general& b);

	void clear();
	bool empty() const;
};

struct mode_entropy {
	// GEN INTROSPECTOR struct mode_entropy
	cosmic_entropy cosmic;
	std::map<mode_player_id, mode_player_entropy> players;
	mode_entropy_general general;
	// END GEN INTROSPECTOR

	void clear_dead_entities(const cosmos& cosm);
	void clear();
	bool empty() const;
	mode_entropy& operator+=(const mode_entropy& b);

	void accumulate(
		mode_player_id,
		entity_id,
		const total_mode_player_entropy&
	);

	total_mode_player_entropy get_for(
		mode_player_id,
		entity_id
	) const;
};
