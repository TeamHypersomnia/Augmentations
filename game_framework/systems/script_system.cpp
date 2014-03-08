#include "stdafx.h"
#include <luabind/class_info.hpp>

#include "entity_system/world.h"
#include "entity_system/entity.h"
#include "entity_system/entity_ptr.h"

#include "script_system.h"

#include "../components/scriptable_component.h"
#include "../bindings/bindings.h"

#include "../messages/collision_message.h"
#include "../messages/damage_message.h"
#include "../messages/intent_message.h"
#include "../messages/shot_message.h"

#include "misc/randval.h"
#include "misc/vector_wrapper.h"
#include "misc/ptr_wrapper.h"
#include "../resources/scriptable_info.h"

void set_world_reloading_script(resources::script* new_scr) {
	world_reloading_script = new_scr;
}

int bitor(lua_State* L) {
	int arg_count = lua_gettop(L);
	int result = 0;

	for (int i = 1; i <= arg_count; i++) {
		luabind::object obj(luabind::from_stack(L, i));

		if(luabind::type(obj) == LUA_TNUMBER) 
			result |= luabind::object_cast<int>(obj);
	}

	lua_pushinteger(L, result);
	return 1;
}

int bitflag(lua_State* L) {
	int result = 1 << luabind::object_cast<int>(luabind::object(luabind::from_stack(L, 1)));
	lua_pushinteger(L, result);
	return 1;
}


namespace bindings {
	extern luabind::scope
		_sfml_audio(),

		_minmax(),
		_vec2(),
		_value_animator(),
		_b2Filter(),
		_rgba(),
		_rect_ltrb(),
		_rect_xywh(),
		_glwindow(),
		_script(),
		_texture(),
		_animation(),
		_world(),
		_entity_ptr(),
		_sprite(),
		_polygon(),
		_polygon_fader(),

		_timer(),

		_particle(),
		_emission(),
		_particle_effect(),

		_message(),
		_intent_message(),
		_animate_message(),
		_particle_burst_message(),
		_collision_message(),
		_damage_message(),
		_destroy_message(),
		_shot_message(),

		_render_component(),
		_transform_component(),
		_visibility_component(),
		_pathfinding_component(),
		_animate_component(),
		_camera_component(),
		_chase_component(),
		_children_component(),
		_crosshair_component(),
		_damage_component(),
		_gun_component(),
		_input_component(),
		_lookat_component(),
		_movement_component(),
		_particle_emitter_component(),
		_physics_component(),
		_steering_component(),
		_scriptable_component(),
		_behaviour_tree_component(),

		_entity(),
		_body_helper(),

		_opengl_binding(),
		_all_systems()
		;
}

int the_callback(lua_State *L) {
	std::string error_message(lua_tostring(L, -1));
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "my_traceback");
	lua_pushvalue(L, 1);
	lua_pushinteger(L, 2);
	lua_call(L, 2, 1);
	printf("%s\n", lua_tostring(L, -1));
	
	std::cout << error_message << std::endl;
	return 1; 
}

void debugger_break() {
	int breakp = 12;
	breakp = 0;
}  
 
script_system::script_system() {
}

void script_system::generate_lua_state(lua_state_wrapper& new_state) {
	using namespace resources;
	using namespace helpers;

	luabind::open(new_state);

	luaL_openlibs(new_state);

	//luabind::bind_class_info(lua_state);

	lua_register(new_state, "bitor", bitor);
	lua_register(new_state, "bitflag", bitflag);
	luabind::module(new_state)[
			luabind::class_<ptr_wrapper<float>>("float_ptr"),

			misc::vector_wrapper<float>::bind("float_vector"),
		
			bindings::_sfml_audio(),
			bindings::_minmax(),
			bindings::_vec2(),
			misc::vector_wrapper<vec2<>>::bind_vector("vec2_vector"),
			bindings::_value_animator(),
			bindings::_b2Filter(),
			bindings::_rgba(),
			bindings::_rect_ltrb(),
			bindings::_rect_xywh(),
			bindings::_glwindow(),
			bindings::_script(),
			bindings::_texture(),
			bindings::_animation(),
			bindings::_world(),
			bindings::_entity_ptr(),
			misc::vector_wrapper<entity_ptr>::bind_vector("entity_ptr_vector"),
			bindings::_sprite(),
			bindings::_polygon(),

			bindings::_timer(),

			bindings::_particle(),
			bindings::_emission(),
			bindings::_particle_effect(),
					  
			bindings::_message(),
			bindings::_intent_message(),
			bindings::_animate_message(),
			bindings::_particle_burst_message(),
			bindings::_collision_message(),
			bindings::_damage_message(),
			bindings::_destroy_message(),
			bindings::_shot_message(),

			bindings::_transform_component(),
			bindings::_render_component(),
			bindings::_visibility_component(),
			bindings::_pathfinding_component(),
			bindings::_animate_component(),
			bindings::_camera_component(),
			bindings::_chase_component(),
			bindings::_children_component(),
			bindings::_crosshair_component(),
			bindings::_damage_component(),
			bindings::_gun_component(),
			bindings::_input_component(),
			bindings::_lookat_component(),
			bindings::_movement_component(),
			bindings::_particle_emitter_component(),
			bindings::_physics_component(),
			bindings::_steering_component(),
			bindings::_scriptable_component(),
			bindings::_behaviour_tree_component(), 

			bindings::_entity(),
			bindings::_body_helper(),

			bindings::_opengl_binding(),

			luabind::def("set_world_reloading_script", &set_world_reloading_script),
			luabind::def("clamp", &augs::get_clamp<float>),
			luabind::def("debugger_break", &debugger_break),
			luabind::def("randval", (float(*)(float, float))&randval),
			luabind::def("randval_i", (int(*)(int, int))&randval),

			luabind::class_<resources::script::reloader>("_script_reloader")
			.def(luabind::constructor<>())
			.def("add_directory", &resources::script::reloader::add_directory),

			bindings::_polygon_fader(),
			bindings::_all_systems()
	];

	luabind::set_pcall_callback(the_callback);
}

script_system::~script_system() {
}

void script_system::process_entities(world& owner) {
	call_loop(owner, false);
}

void script_system::call_loop(world& owner, bool substepping_flag) {
	auto target_copy = targets;
	for (auto it : target_copy) {
		auto& scriptable = it->get<components::scriptable>();
		if (!scriptable.available_scripts) continue;

		auto loop_event = scriptable.available_scripts->get_raw().find(components::scriptable::LOOP);
		
		if (loop_event != scriptable.available_scripts->get_raw().end()) {
			try {
				luabind::call_function<void>((*loop_event).second, it, substepping_flag);
			}
			catch (std::exception compilation_error) {
				std::cout << compilation_error.what() << std::endl;
			}
		}
		//auto loop_event = scriptable
	}
}

void script_system::substep(world& owner) {
	call_loop(owner, true);
	pass_events(owner, true);
}

template<typename message_type>
void pass_events_to_script(world& owner, int msg_enum, bool substepping_flag) {
	auto& events = owner.get_message_queue<message_type>();

	std::for_each(events.begin(), events.end(), [msg_enum, substepping_flag](message_type msg){
		auto* scriptable = msg.subject->find<components::scriptable>();
		if (scriptable == nullptr || !scriptable->available_scripts) return;

		auto it = scriptable->available_scripts->get_raw().find(msg_enum);

		if (it != scriptable->available_scripts->get_raw().end()) {
			luabind::call_function<void>((*it).second, msg, substepping_flag);
		}
	});
}
using namespace messages;


void script_system::pass_events(world& owner, bool substepping_flag) {
	try {
		pass_events_to_script<collision_message>(owner, components::scriptable::COLLISION_MESSAGE, substepping_flag);
		pass_events_to_script<damage_message>(owner, components::scriptable::DAMAGE_MESSAGE, substepping_flag);
		pass_events_to_script<intent_message>(owner, components::scriptable::INTENT_MESSAGE, substepping_flag);
		pass_events_to_script<shot_message>(owner, components::scriptable::SHOT_MESSAGE, substepping_flag);
	}
	catch (std::exception compilation_error) {
		std::cout << compilation_error.what() << std::endl;
	}
}

void script_system::process_events(world& owner) {
	pass_events(owner, false);
}