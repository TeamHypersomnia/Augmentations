#pragma once
#include <vector>
#include "augs/graphics/pixel.h"
#include "augs/misc/templated_readwrite.h"
#include <experimental/filesystem>

struct desaturation_metadata {
	std::experimental::filesystem::file_time_type last_write_time_of_source;
};

void regenerate_desaturations();