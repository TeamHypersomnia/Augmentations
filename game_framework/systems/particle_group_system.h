#pragma once
#include "misc/timer.h"

#include "entity_system/processing_system.h"

#include "../components/particle_group_component.h"

using namespace augs;

class particle_group_system : public processing_system_templated<components::particle_group> {
	timer timer;
public:
	using processing_system_templated::processing_system_templated;

	void step_streams_and_particles_and_destroy_dead();
};