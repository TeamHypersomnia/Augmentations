#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/components/transform_component.h"

template<bool is_const, class entity_handle_type>
class basic_spatial_properties_getters {
public:
	components::transform logic_transform() const;
};

template<bool, class>
class spatial_properties_getters;

template<class entity_handle_type>
class spatial_properties_getters<false, entity_handle_type> : public basic_spatial_properties_getters<false, entity_handle_type> {
public:
	void set_logic_transform(const components::transform) const;
};

template<class entity_handle_type>
class spatial_properties_getters<true, entity_handle_type> : public basic_spatial_properties_getters<true, entity_handle_type> {
};