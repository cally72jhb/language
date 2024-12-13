#ifndef STANDARD_LIBRARY_COLOR
#define STANDARD_LIBRARY_COLOR

// Includes

#include "common/constants.h"

// Typedefs

typedef struct {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} color;

typedef struct {
    f32 red;
    f32 green;
    f32 blue;
    f32 alpha;
} color_f;

typedef struct {
    f32 hue;
    u8 sat;
    u8 val;
} color_hsv;

typedef u32 packed_color;

// Color RGBA Functions

color color_from_packed(packed_color packed);
packed_color color_to_packed(color color_value);

u32 color_rgb_distance(color color1, color color2);
u32 color_distance(color color1, color color2);
bool color_rgb_equals(color color1, color color2);
bool color_equals(color color1, color color2);

color_hsv color_to_hsv(color color_value);

f32 color_get_hue(color color_value);
u8 color_get_sat(color color_value);
u8 color_get_val(color color_value);

// HSV Functions

bool color_hsv_equals(color_hsv color1, color_hsv color2);
color color_hsv_to_rgb(color_hsv color_value);

u8 color_hsv_get_red(color_hsv color_value);
u8 color_hsv_get_green(color_hsv color_value);
u8 color_hsv_get_blue(color_hsv color_value);

void color_hsv_validate(color_hsv* color_value);
bool color_hsv_is_valid(color_hsv color_value);

#endif
