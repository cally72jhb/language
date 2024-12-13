#include "color.h"

#include "common/constants.h"

#include "common/math/primitives/f32_math.h"
#include "common/math/primitives/f64_math.h"

// Color Functions

color color_from_packed(packed_color packed) {
    return (color) {
        (packed >> 16) & 0xFF,
        (packed >>  8) & 0xFF,
        (packed >>  0) & 0xFF,
        (packed >> 24) & 0xFF
    };
}

packed_color color_to_packed(color color_value) {
    return (packed_color) ((color_value.red << 16) & (color_value.green << 8) & (color_value.blue << 0) & (color_value.alpha << 24));
}

u32 color_rgb_distance(color color1, color color2) {
    i32 delta_red   = color1.red   - color2.red;
    i32 delta_green = color1.green - color2.green;
    i32 delta_blue  = color1.blue  - color2.blue;

    return (u32) ((delta_red * delta_red) + (delta_green * delta_green) + (delta_blue * delta_blue));
}

u32 color_distance(color color1, color color2) {
    i32 delta_red   = color1.red   - color2.red;
    i32 delta_green = color1.green - color2.green;
    i32 delta_blue  = color1.blue  - color2.blue;
    i32 delta_alpha = color1.alpha - color2.alpha;

    return (u32) ((delta_red * delta_red) + (delta_green * delta_green) + (delta_blue * delta_blue) + (delta_alpha * delta_alpha));
}

bool color_rgb_equals(color color1, color color2) {
    return color1.red == color2.red && color1.green == color2.green && color1.blue == color2.blue;
}

bool color_equals(color color1, color color2) {
    return color1.red == color2.red && color1.green == color2.green && color1.blue == color2.blue && color1.alpha == color2.alpha;
}

color_hsv color_to_hsv(color color_value) {
    f32 red = ((f32) color_value.red) / 255.0F;
    f32 green = ((f32) color_value.green) / 255.0F;
    f32 blue = ((f32) color_value.blue) / 255.0F;

    f32 color_max = f32_max3(red, green, blue);
    f32 color_min = f32_min3(red, green, blue);

    f32 diff = color_max - color_min;

    f32 hue = 0.0F;
    u8 sat = 0;

    // Hue

    if (color_max == color_min) {
        hue = 0.0F;
    } else if (color_max == red) {
        hue = f32_mod(60.0F * ((green - blue) / diff) + 360.0F, 360.0F);
    } else if (color_max == green) {
        hue = f32_mod(60.0F * ((blue - red) / diff) + 120.0F, 360.0F);
    } else {
        hue = f32_mod(60.0F * ((red - green) / diff) + 240.0F, 360.0F);
    }

    if (color_max == 0.0F) {
        sat = 0;
    } else {
        sat = (u8) ((diff / color_max) * 100.0F);
    }

    u8 val = (u8) (color_max * 100.0F);

    return (color_hsv) { hue, sat, val };
}

f32 color_get_hue(color color_value) {
    f32 red = ((f32) color_value.red) / 255.0F;
    f32 green = ((f32) color_value.green) / 255.0F;
    f32 blue = ((f32) color_value.blue) / 255.0F;

    f32 color_max = f32_max3(red, green, blue);
    f32 color_min = f32_min3(red, green, blue);

    f32 diff = color_max - color_min;

    f32 hue = 0.0F;

    // Hue

    if (color_max == color_min) {
        hue = 0.0F;
    } else if (color_max == red) {
        hue = f32_mod(60.0F * ((green - blue) / diff) + 360.0F, 360.0F);
    } else if (color_max == green) {
        hue = f32_mod(60.0F * ((blue - red) / diff) + 120.0F, 360.0F);
    } else {
        hue = f32_mod(60.0F * ((red - green) / diff) + 240.0F, 360.0F);
    }

    return hue;
}

u8 color_get_sat(color color_value) {
    f32 red = ((f32) color_value.red) / 255.0F;
    f32 green = ((f32) color_value.green) / 255.0F;
    f32 blue = ((f32) color_value.blue) / 255.0F;

    f32 color_max = f32_max3(red, green, blue);
    f32 color_min = f32_min3(red, green, blue);

    f32 diff = color_max - color_min;

    u8 sat = 0;

    // Saturation

    if (color_max == 0) {
        sat = 0;
    } else {
        sat = (u8) ((diff / color_max) * 100.0F);
    }

    return sat;
}

u8 color_get_val(color color_value) {
    f32 color_max = ((f32) f32_max3(color_value.red, color_value.green, color_value.blue)) / 255.0F;
    return (u8) (color_max * 100.0F);
}

// HSV Functions

bool color_hsv_equals(color_hsv color1, color_hsv color2) {
    return color1.hue == color2.hue && color1.sat == color2.sat && color1.val == color2.val;
}

color color_hsv_to_rgb(color_hsv color_value) {
    f32 sat = ((f32) color_value.sat) / 100.0F;
    f32 val = ((f32) color_value.val) / 100.0F;

    f32 sat_val = sat * val;
    f32 temp = f32_mod(color_value.hue / 60.0F, 2.0F) - 1.0F;
    f32 value = sat_val * (1.0F - f32_abs(temp));
    f32 modifier = val - sat_val;

    f32 red = 0.0F;
    f32 green = 0.0F;
    f32 blue = 0.0F;

    if (color_value.hue >= 0.0F && color_value.hue < 60.0F) {
        red = sat_val;
        green = value;
        blue = 0.0F;
    } else if (color_value.hue >= 60.0F && color_value.hue < 120.0F) {
        red = value;
        green = sat_val;
        blue = 0.0F;
    } else if (color_value.hue >= 120.0F && color_value.hue < 180.0F) {
        red = 0.0F;
        green = sat_val;
        blue = value;
    } else if (color_value.hue >= 180.0F && color_value.hue < 240.0F) {
        red = 0.0F;
        green = value;
        blue = sat_val;
    } else if (color_value.hue >= 240.0F && color_value.hue < 300.0F) {
        red = value;
        green = 0.0F;
        blue = sat_val;
    } else {
        red = sat_val;
        green = 0.0F;
        blue = value;
    }

    return (color) {
        (u8) f32_round((red + modifier) * 255.0F),
        (u8) f32_round((green + modifier) * 255.0F),
        (u8) f32_round((blue + modifier) * 255.0F)
    };
}

u8 color_hsv_get_red(color_hsv color_value) {
    f32 sat = ((f32) color_value.sat) / 100.0F;
    f32 val = ((f32) color_value.val) / 100.0F;

    f32 sat_val = sat * val;
    f32 temp = f32_mod(color_value.hue / 60.0F, 2.0F) - 1.0F;
    f32 value = sat_val * (1.0F - f32_abs(temp));
    f32 modifier = val - sat_val;

    f32 red = 0.0F;

    if (color_value.hue >= 60.0F && color_value.hue < 120.0F || color_value.hue >= 240.0F && color_value.hue < 300.0F) {
        red = value;
    } else if (color_value.hue >= 120.0F && color_value.hue < 180.0F || color_value.hue >= 180.0F && color_value.hue < 240.0F) {
        red = 0.0F;
    } else {
        red = sat_val;
    }

    return (u8) f32_round((red + modifier) * 255.0F);
}

u8 color_hsv_get_green(color_hsv color_value) {
    f32 sat = ((f32) color_value.sat) / 100.0F;
    f32 val = ((f32) color_value.val) / 100.0F;

    f32 sat_val = sat * val;
    f32 temp = f32_mod(color_value.hue / 60.0F, 2.0F) - 1.0F;
    f32 value = sat_val * (1.0F - f32_abs(temp));
    f32 modifier = val - sat_val;

    f32 green = 0.0F;

    if (color_value.hue >= 0.0F && color_value.hue < 60.0F || color_value.hue >= 180.0F && color_value.hue < 240.0F) {
        green = value;
    } else if (color_value.hue >= 60.0F && color_value.hue < 120.0F || color_value.hue >= 120.0F && color_value.hue < 180.0F) {
        green = sat_val;
    } else {
        green = 0.0F;
    }

    return (u8) f32_round((green + modifier) * 255.0F);
}

u8 color_hsv_get_blue(color_hsv color_value) {
    f32 sat = ((f32) color_value.sat) / 100.0F;
    f32 val = ((f32) color_value.val) / 100.0F;

    f32 sat_val = sat * val;
    f32 temp = f32_mod(color_value.hue / 60.0F, 2.0F) - 1.0F;
    f32 value = sat_val * (1.0F - f32_abs(temp));
    f32 modifier = val - sat_val;

    f32 blue = 0.0F;

    if (color_value.hue >= 0.0F && color_value.hue < 60.0F || color_value.hue >= 60.0F && color_value.hue < 120.0F) {
        blue = 0.0F;
    } else if (color_value.hue >= 180.0F && color_value.hue < 240.0F || color_value.hue >= 240.0F && color_value.hue < 300.0F) {
        blue = sat_val;
    } else {
        blue = value;
    }

    return (u8) f32_round((blue + modifier) * 255.0F);
}

void color_hsv_validate(color_hsv* color_value) {
    if (color_value->hue < 0.0F) {
        color_value->hue = 0.0F;
    } else if (color_value->hue > 360.0F) {
        color_value->hue = 360.0F;
    }

    if (color_value->sat > 100) {
        color_value->sat = 100;
    }

    if (color_value->val > 100) {
        color_value->val = 100;
    }
}

bool color_hsv_is_valid(color_hsv color_value) {
    return color_value.hue >= 0.0F && color_value.hue <= 360.0F && color_value.sat <= 100 && color_value.val <= 100;
}
