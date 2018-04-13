#pragma once
#include "application/setups/editor/gui/standard_window_mixin.h"

struct editor_command_input;

struct editor_history_gui : standard_window_mixin<editor_history_gui> {
	using base = standard_window_mixin<editor_history_gui>;
	using base::base;

	void perform(editor_command_input);
};
