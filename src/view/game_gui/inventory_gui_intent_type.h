#pragma once
#include "augs/misc/enum/enum_map.h"
#include "augs/misc/basic_input_intent.h"

#include "augs/window_framework/event.h"

enum class inventory_gui_intent_type {
	// GEN INTROSPECTOR enum class inventory_gui_intent_type
	INVALID,

	LAST_USED_WEAPON,

	HOLSTER,

	HOTBAR_0,
	HOTBAR_1,
	HOTBAR_2,
	HOTBAR_3,
	HOTBAR_4,
	HOTBAR_5,
	HOTBAR_6,
	HOTBAR_7,
	HOTBAR_8,
	HOTBAR_9,

	SPECIAL_ACTION_BUTTON_1,
	SPECIAL_ACTION_BUTTON_2,
	SPECIAL_ACTION_BUTTON_3,
	SPECIAL_ACTION_BUTTON_4,
	SPECIAL_ACTION_BUTTON_5,
	SPECIAL_ACTION_BUTTON_6,
	SPECIAL_ACTION_BUTTON_7,
	SPECIAL_ACTION_BUTTON_8,
	SPECIAL_ACTION_BUTTON_9,
	SPECIAL_ACTION_BUTTON_10,
	SPECIAL_ACTION_BUTTON_11,
	SPECIAL_ACTION_BUTTON_12,

	COUNT
	// END GEN INTROSPECTOR
};

using inventory_gui_intent = basic_input_intent<inventory_gui_intent_type>;

using inventory_gui_intent_map = augs::enum_map<
	augs::event::keys::key,
	inventory_gui_intent_type
>;
