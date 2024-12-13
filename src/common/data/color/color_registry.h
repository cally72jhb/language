#ifndef STANDARD_LIBRARY_COLOR_REGISTRY
#define STANDARD_LIBRARY_COLOR_REGISTRY

// Includes

#include "common/constants.h"
#include "common/data/color/color.h"

// Registry Functions

cstr color_registry_get_color_name(color color_value);
color color_registry_get_color(str color_name);

#endif
