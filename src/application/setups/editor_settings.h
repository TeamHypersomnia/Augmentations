#pragma once
#include "augs/filesystem/path.h"

struct editor_autosave_settings {
	// GEN INTROSPECTOR struct editor_autosave_settings
	double once_every_min = 1.0;
	bool enabled = true;
	// END GEN INTROSPECTOR

	bool operator!=(const editor_autosave_settings b) const {
		return once_every_min != b.once_every_min || enabled != b.enabled;
	}
};

struct editor_settings {
	// GEN INTROSPECTOR struct editor_settings
	editor_autosave_settings autosave;
	vec2 camera_panning_speed = { 1.f, 1.f };
	// END GEN INTROSPECTOR
};