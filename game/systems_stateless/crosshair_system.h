#pragma once
class cosmos;
class logic_step;

class crosshair_system {
public:
	void generate_crosshair_intents(logic_step&);
	
	void apply_crosshair_intents_to_base_offsets(logic_step&);
	void apply_base_offsets_to_crosshair_transforms(logic_step&);
};