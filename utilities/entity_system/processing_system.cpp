#pragma once
#include "stdafx.h"

#include "processing_system.h"
#include "entity.h"
#include <algorithm>

namespace augs {
	namespace entity_system {
		void processing_system::add(entity* e) {
			targets.push_back(e);
		}
		
		void processing_system::remove(entity* e) {
			targets.erase(std::remove(targets.begin(), targets.end(), e), targets.end());
		}

		void processing_system::clear() {
			targets.clear();
		}
	}
}