#include "color_registry.h"

#include "common/constants.h"
#include "common/macros.h"

#include "common/data/color/color.h"
#include "common/data/string/string.h"

// Color Registry

#define COLOR_REGISTRY_ENTRY(color_name, value_red, value_green, value_blue) CONCAT6(COLOR_REGISTRY_TEMP_, value_red, _, value_green, _, value_blue),
enum {
    #if PROGRAM_FEATURE_NO_COLOR_REGISTRY == 0
    #include "color_registry_inline.h"
    #else
    COLOR_REGISTRY_ENTRY("undefined", 0, 0, 0)
    #endif

    COLOR_REGISTRY_ENTRIES // the number of entries in the color registry
};

typedef struct {
    cstr name;
    struct { u8 red; u8 green; u8 blue; } color;
} color_registry_entry;

#undef COLOR_REGISTRY_ENTRY
#define COLOR_REGISTRY_ENTRY(color_name, value_red, value_green, value_blue) (color_registry_entry) { .name = color_name, .color = { .red = value_red, .green = value_green, .blue = value_blue } },

static color_registry_entry registry[] = {
    #if PROGRAM_FEATURE_NO_COLOR_REGISTRY == 0
    #include "color_registry_inline.h"
    #else
    COLOR_REGISTRY_ENTRY("undefined", 0, 0, 0)
    #endif
};

#undef COLOR_REGISTRY_ENTRY

// Registry Functions

cstr color_registry_get_color_name(color color_value) {
    #if PROGRAM_FEATURE_NO_COLOR_REGISTRY != 0
    return NULL;
    #else

    cstr best_match = NULL;
    u32 min = U32_MAX;

    for (u32 i = 0; i < COLOR_REGISTRY_ENTRIES; i++) {
        color registry_color = (color) {
            .red   = registry[i].color.red,
            .green = registry[i].color.green,
            .blue  = registry[i].color.blue,
            .alpha = 255
        };

        u32 distance = color_rgb_distance(registry_color, color_value);
        if (distance == 0) {
            return registry[i].name;
        } else if (distance < min) {
            min = distance;
            best_match = registry[i].name;
        }
    }

    return best_match;
    #endif
}

color color_registry_get_color(str color_name) {
    #if PROGRAM_FEATURE_NO_COLOR_REGISTRY == 0
    for (u32 i = 0; i < COLOR_REGISTRY_ENTRIES; i++) {
        if (str_is_equals(color_name, (str) registry[i].name)) {
            return (color) {
                .red   = registry[i].color.red,
                .green = registry[i].color.green,
                .blue  = registry[i].color.blue,
                .alpha = 255
            };
        }
    }
    #endif

    return (color) { .red = 0, .green = 0, .blue = 0, .alpha = 0 };
}
