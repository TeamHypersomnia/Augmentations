#pragma once
#include "augs/misc/enum/enum_boolset.h"

enum class item_category {
	// GEN INTROSPECTOR enum class item_category
	GENERAL,

	RAIL_ATTACHMENT,
	MUZZLE_ATTACHMENT,
	BACK_WEARABLE,
	OVER_BACK_WEARABLE,
	BELT_WEARABLE,
	PERSONAL_DEPOSIT_WEARABLE,
	TORSO_ARMOR,
	MAGAZINE,
	SHOT_CHARGE,
	ROCKET,
	SHOULDER_WEARABLE,

	COUNT
	// END GEN INTROSPECTOR
};

using item_category_flagset = augs::enum_boolset<item_category>;

bool is_clothing(const item_category_flagset& category);