#pragma once
#include <optional>
#include "augs/misc/timing/fixed_delta_timer.h"
#include "augs/math/camera_cone.h"

#include "game/detail/render_layer_filter.h"
#include "game/cosmos/entity_handle.h"
#include "game/modes/test_mode.h"

#include "test_scenes/test_scene_settings.h"

#include "application/intercosm.h"
#include "application/setups/default_setup_settings.h"

#include "application/debug_settings.h"
#include "application/input/entropy_accumulator.h"
#include "application/setups/setup_common.h"
#include "view/mode_gui/arena/arena_player_meta.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "view/viewables/ad_hoc_atlas_subject.h"
#include "view/viewables/ad_hoc_in_atlas_map.h"
#include "application/setups/builder/project/builder_project_meta.h"
#include "application/setups/project_selector/project_selector_gui.h"

struct config_lua_table;
struct draw_setup_gui_input;

namespace sol {
	class state;
}

class project_selector_setup : public default_setup_settings {
	all_viewables_defs viewables;

	project_selector_gui gui;

	void load_gui_state();
	void save_gui_state();

	bool rebuild_miniatures = false;
	void scan_for_all_arenas();

	ad_hoc_entry_id miniature_index_counter = 0;

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;

	project_selector_setup();
	~project_selector_setup();

	auto get_audiovisual_speed() const {
		return 1.0;
	}

	const auto& get_viewed_cosmos() const {
		return cosmos::zero;
	}

	auto get_interpolation_ratio() const {
		return 1.0;
	}

	auto get_viewed_character_id() const {
		return entity_id();
	}

	auto get_controlled_character_id() const {
		return entity_id();
	}

	auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	const auto& get_viewable_defs() const {
		return viewables;
	}

	custom_imgui_result perform_custom_imgui(perform_custom_imgui_input);
	void customize_for_viewing(config_lua_table&) const;

	void apply(const config_lua_table&) {
		return;
	}

	auto escape() {
		return setup_escape_result::IGNORE;
	}

	double get_inv_tickrate() const {
		return 1.0 / 60;
	}

	template <class C>
	void advance(
		const setup_advance_input& in,
		const C& callbacks
	) {
		(void)in;
		(void)callbacks;
	}

	template <class T>
	void control(const T&) {

	}

	void accept_game_gui_events(const game_gui_entropy_type&) {}

	std::optional<camera_eye> find_current_camera_eye() const {
		return std::nullopt;
	}

	augs::path_type get_unofficial_content_dir() const {
		return {};
	}

	auto get_render_layer_filter() const {
		return render_layer_filter::disabled();
	}

	void draw_custom_gui(const draw_setup_gui_input&) {}

	void ensure_handler() {}
	bool requires_cursor() const { return false; }

	const entropy_accumulator& get_entropy_accumulator() const {
		return entropy_accumulator::zero;
	}

	template <class F>
	void on_mode_with_input(F&&) const {}

	auto get_game_gui_subject_id() const {
		return get_viewed_character_id();
	}

	std::nullopt_t get_new_player_metas() {
		return std::nullopt;
	}

	std::optional<ad_hoc_atlas_subjects> get_new_ad_hoc_images();

	const arena_player_metas* find_player_metas() const {
		return nullptr;
	}

	void after_all_drawcalls(game_frame_buffer&) {}
	void do_game_main_thread_synced_op(renderer_backend_result&) {}

	augs::path_type get_selected_project_path() const;
};