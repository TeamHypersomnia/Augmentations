#pragma once
#include "augs/templates/type_in_list_id.h"

struct controlled_character;
struct plain_invisible_body;
struct plain_sprited_body;
struct shootable_weapon;
struct shootable_charge;
struct sprite_decoration;
struct wandering_sprite_decoration;
struct static_light;
struct throwable_explosive;
struct plain_missile;
struct finishing_trace;
struct container_item;
struct explosive_missile;

using all_entity_types = type_list<
	controlled_character,
	plain_invisible_body,
	plain_sprited_body,
	shootable_weapon,
	shootable_charge,
	sprite_decoration,
	wandering_sprite_decoration,
	static_light,
	throwable_explosive,
	plain_missile,
	finishing_trace,
	container_item,
	explosive_missile
>;

using entity_type_id = type_in_list_id<all_entity_types>;