/*
 * Copyright (c) 1995 Argonaut Technologies Limited. All rights reserved.
 *
 * $Id: convert.c 1.2 1998/07/14 13:13:30 johng Exp $
 * $Locker: $
 *
 */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "brender.h"

enum {
    T_CBFN_CONVERT_PIXELMAP,
    T_CBFN_FLIP_HIGH_WIDE_PALETTES,
    T_CBFN_FLIP_X_PIXELMAP,
    T_CBFN_FLIP_Y_PIXELMAP,
};

typedef br_colour t_read_pixel_fn_type(char *pixels, br_float fract);
typedef void      t_write_pixel_fn_type(char *pixels, br_float fract, br_colour new_pixel);

typedef struct t_pixelmap_type_info {

    t_read_pixel_fn_type  *read_pixel_fn;
    t_write_pixel_fn_type *write_pixel_fn;

    t_read_pixel_fn_type *read_flip_pixel_fn;

    br_uint_8 needs_palette;

    br_int_32 type; /* pixelmap->type */

    char *identifier;
    char *description;

} t_pixelmap_type_info;

/*
 * command structure passed to pixelmap cbfn
 */
typedef struct t_pixelmap_cbfn_info {

    br_uint_32 type; /* cbfn type */

    br_int_32 new_type;

    br_int_32 width;
    br_int_32 height;

    br_int_32 origin_x;
    br_int_32 origin_y;

} t_pixelmap_cbfn_info;

static br_boolean LEAVE_TRANSPARENT = BR_FALSE;

static br_pixelmap *current_src;
static br_pixelmap *current_dst;

t_pixelmap_type_info PixelmapTypes[];

const char *T_EncodePixelmapType(br_int_32 type);

/*
 * per pixel read/write operations
 */
br_colour T_Read_Alpha_8(char *pixels, br_float fract)
{
    br_uint_8 index;

    index = *((br_uint_8 *)pixels);

    return BR_COLOUR_ARGB(index, 0, 0, 0);
}

br_colour T_Read_Flip_Index_8(char *pixels, br_float fract)
{
    br_uint_8 index;

    index = *((br_uint_8 *)pixels);

    return BR_COLOUR_ARGB(0, 0, 0, index);
}

void T_Write_Alpha_8(char *pixels, br_float fract, br_colour new_pixel)
{
    /* colour format R G B R G B R G B */

    *((br_uint_8 *)pixels) = BR_ALPHA(new_pixel);
}

br_colour T_Read_IndexA_88(char *pixels, br_float fract)
{
    /* palette colour format ??? */

    br_uint_8    index, alpha;
    char        *s_pixels;
    br_colour    colour;
    br_pixelmap *pm;
    br_float     s_fract, s_step;

    index = ((br_uint_8 *)pixels)[1];
    alpha = ((br_uint_8 *)pixels)[0];

    if(current_src->map == NULL)
        BR_ERROR2("'%s' (%s) is an indexed pixelmap but has no palette information", current_src->identifier,
                  T_EncodePixelmapType(current_src->type));

    pm          = current_src;
    current_src = current_src->map;
    s_step      = BrPixelmapPixelSize(current_src) / 8;

    if(current_src->height > current_src->width) {
        s_pixels = ((char *)current_src->pixels) + ((index + current_src->base_y) * current_src->row_bytes);
        s_fract  = (current_src->base_x) * s_step;
    } else {
        s_pixels = ((char *)current_src->pixels) + (current_src->base_y * current_src->row_bytes);
        s_fract  = (current_src->base_x + index) * s_step;
    }

    colour = PixelmapTypes[current_src->type].read_pixel_fn(s_pixels + ((int)s_fract), ((int)s_fract));

    current_src = pm;

    colour = BR_COLOUR_ARGB(alpha, BR_RED(colour), BR_GRN(colour), BR_BLU(colour));

    return colour;
}

br_colour T_Read_Flip_IndexA_88(char *pixels, br_float fract)
{
    br_uint_8 index, alpha;

    index = ((br_uint_8 *)pixels)[1];
    alpha = ((br_uint_8 *)pixels)[0];

    return index;
}

br_colour T_Read_Index_8(char *pixels, br_float fract)
{
    /* palette colour format ??? */

    br_uint_8    index;
    char        *s_pixels;
    br_colour    colour;
    br_pixelmap *pm;
    br_float     s_fract, s_step;

    index = *((br_uint_8 *)pixels);

    if(current_src->map == NULL)
        BR_ERROR2("'%s' (%s) is an indexed pixelmap but has no palette information", current_src->identifier,
                  T_EncodePixelmapType(current_src->type));

    if(LEAVE_TRANSPARENT) {
        // If Colour 0 then Transparent
        if(index == 0)
            return BR_COLOUR_ARGB(255, 0, 0, 0);
    }

    pm          = current_src;
    current_src = current_src->map;
    s_step      = BrPixelmapPixelSize(current_src) / 8;

    if(current_src->height > current_src->width) {
        s_pixels = ((char *)current_src->pixels) + ((index + current_src->base_y) * current_src->row_bytes);
        s_fract  = (current_src->base_x) * s_step;
    } else {
        s_pixels = ((char *)current_src->pixels) + (current_src->base_y * current_src->row_bytes);
        s_fract  = (current_src->base_x + index) * s_step;
    }

    colour = PixelmapTypes[current_src->type].read_pixel_fn(s_pixels + ((int)s_fract), ((int)s_fract));

    current_src = pm;

    return colour;
}

void T_Write_Index_8(char *pixels, br_float fract, br_colour new_pixel)
{
    /*
     * write an index
     */

    *((br_uint_8 *)pixels) = (br_uint_8)new_pixel;
}

br_colour T_Read_Index_4(char *pixels, br_float fract)
{
    /* palette colour format ??? */

    br_uint_8    index;
    char        *s_pixels;
    br_colour    colour;
    br_pixelmap *pm;
    br_float     s_step, s_fract;

    if(fract == 0)
        /*
         * even number pixel
         */
        index = (*((br_uint_8 *)pixels)) & 0x0f;
    else
        /*
         * odd number pixel
         */
        index = (*((br_uint_8 *)pixels) >> 4) & 0x0f;

    if(current_src->map == NULL)
        BR_ERROR2("'%s' (%s) is an indexed pixelmap but has no palette information", current_src->identifier,
                  T_EncodePixelmapType(current_src->type));

    pm          = current_src;
    current_src = current_src->map;
    s_step      = ((br_float)BrPixelmapPixelSize(current_src)) / 8.0;

    if(current_src->height > current_src->width) {
        s_pixels = ((char *)current_src->pixels) + ((index + current_src->base_y) * current_src->row_bytes);
        s_fract  = (current_src->base_x) * s_step;
    } else {
        s_pixels = ((char *)current_src->pixels) + (current_src->base_y * current_src->row_bytes);
        s_fract  = (current_src->base_x + index) * s_step;
    }

    colour = PixelmapTypes[current_src->type].read_pixel_fn(s_pixels + ((int)s_fract), s_fract - ((int)s_fract));

    current_src = pm;

    return colour;
}

br_colour T_Read_Flip_Index_4(char *pixels, br_float fract)
{
    br_uint_8 index;

    if(fract == 0)
        /*
         * even number pixel
         */
        index = (*((br_uint_8 *)pixels)) & 0x0f;
    else
        /*
         * odd number pixel
         */
        index = (*((br_uint_8 *)pixels) >> 4) & 0x0f;

    return index;
}

void T_Write_Index_4(char *pixels, br_float fract, br_colour new_pixel)
{
    /*
     * write an index
     */
    if(fract == 0)
        /*
         * even number pixel
         */
        *((br_uint_8 *)pixels) = (*((br_uint_8 *)pixels) & 0xf0) | (new_pixel & 0x0f);
    else
        /*
         * odd number pixel
         */
        *((br_uint_8 *)pixels) = (*((br_uint_8 *)pixels) & 0x0f) | ((new_pixel << 4) & 0xf0);
}

br_colour T_Read_RGB_888(char *pixels, br_float fract)
{
    /* colour format R G B R G B R G B */

    br_colour colour;
    br_uint_8 r, g, b;

    r = ((br_uint_8 *)pixels)[2];
    g = ((br_uint_8 *)pixels)[1];
    b = ((br_uint_8 *)pixels)[0];

    colour = BR_COLOUR_ARGB(0, r, g, b);

    return colour;
}

void T_Write_RGB_888(char *pixels, br_float fract, br_colour new_pixel)
{
    /* colour format R G B R G B R G B */

    ((br_uint_8 *)pixels)[2] = BR_RED(new_pixel);
    ((br_uint_8 *)pixels)[1] = BR_GRN(new_pixel);
    ((br_uint_8 *)pixels)[0] = BR_BLU(new_pixel);
}

void T_Write_RGBX_888(char *pixels, br_float fract, br_colour new_pixel)
{
    /* colour format X R G B X R G B X R G B */

    *((br_colour *)pixels) = new_pixel;
}
br_colour T_Read_RGBX_888(char *pixels, br_float fract)
{
    /* colour format X R G B X R G B X R G B */

    return *((br_colour *)pixels);
}

br_colour T_Read_RGB_555(char *pixels, br_float fract)
{
    /* colour format R G B R G B R G B  - 2 bytes per pixel */

    br_uint_16 temp_colour;
    br_uint_16 r, g, b;

    temp_colour = *((br_uint_16 *)pixels);

    r = (temp_colour >> 7) & 0xf8;
    g = (temp_colour >> 2) & 0xf8;
    b = (temp_colour << 3) & 0xff;

    return BR_COLOUR_ARGB(0, r, g, b);
}

void T_Write_RGB_555(char *pixels, br_float fract, br_colour new_pixel)
{
    /* colour format R G B R G B R G B - 2 bytes per pixel */
    br_uint_16 temp_colour;

    temp_colour = ((BR_RED(new_pixel) >> 3) << 10) | ((BR_GRN(new_pixel) >> 3) << 5) | ((BR_BLU(new_pixel) >> 3));

    *((br_uint_16 *)pixels) = temp_colour;
}

br_colour T_Read_RGB_565(char *pixels, br_float fract)
{
    /* colour format R G B R G B R G B  - 2 bytes per pixel */

    br_colour  colour;
    br_uint_16 temp_colour;
    br_uint_8  r, g, b;

    temp_colour = *((br_uint_16 *)pixels);

    r = (temp_colour >> 8) & 0xf8;
    g = (temp_colour >> 3) & 0xf8;
    b = (temp_colour << 3) & 0xf8;

    colour = BR_COLOUR_ARGB(0, r, g, b);

    return colour;
}

void T_Write_RGB_565(char *pixels, br_float fract, br_colour new_pixel)
{
    /* colour format R G B R G B R G B - 2 bytes per pixel */
    br_uint_16 temp_colour;

    temp_colour = ((BR_RED(new_pixel) >> 3) << 11) | ((BR_GRN(new_pixel) >> 2) << 5) | ((BR_BLU(new_pixel) >> 3));

    *((br_uint_16 *)pixels) = temp_colour;
}

br_colour T_Read_BGR_555(char *pixels, br_float fract)
{
    /* colour format B G R B G R B G R  - 2 bytes per pixel */

    br_colour  colour;
    br_uint_16 temp_colour;
    br_uint_8  r, g, b;

    temp_colour = *((br_uint_16 *)pixels);

    b = (temp_colour >> 7) & 0xf8;
    g = (temp_colour >> 2) & 0xf8;
    r = (temp_colour << 3) & 0xf8;

    colour = BR_COLOUR_ARGB(0, r, g, b);

    return colour;
}

void T_Write_BGR_555(char *pixels, br_float fract, br_colour new_pixel)
{
    /* colour format B G R - 2 bytes per pixel */
    br_uint_16 temp_colour;

    temp_colour = ((BR_BLU(new_pixel) >> 3) << 10) | ((BR_GRN(new_pixel) >> 3) << 5) | ((BR_RED(new_pixel) >> 3));

    *((br_uint_16 *)pixels) = temp_colour;
}

br_colour T_Read_RGBA_4444(char *pixels, br_float fract)
{
    /* colour format R G B A R G B A R G B A - 2 bytes per pixel */

    br_colour  colour;
    br_uint_16 temp_colour;
    br_uint_8  r, g, b, a;

    temp_colour = *((br_uint_16 *)pixels);

    r = (temp_colour >> 8) & 0xf0;
    g = (temp_colour >> 4) & 0xf0;
    b = (temp_colour)&0xf0;
    a = (temp_colour << 4) & 0xf0;

    colour = BR_COLOUR_ARGB(a, r, g, b);

    return colour;
}

br_colour T_Read_ARGB_4444(char *pixels, br_float fract)
{
    /* colour format R G B A R G B A R G B A - 2 bytes per pixel */

    br_colour  colour;
    br_uint_16 temp_colour;
    br_uint_8  r, g, b, a;

    temp_colour = *((br_uint_16 *)pixels);

    r = (temp_colour >> 4) & 0xf0;
    g = (temp_colour)&0xf0;
    b = (temp_colour << 4) & 0xf0;
    a = (temp_colour << 8) & 0xf0;

    colour = BR_COLOUR_ARGB(a, r, g, b);

    return colour;
}

void T_Write_RGBA_4444(char *pixels, br_float fract, br_colour new_pixel)
{
    /* colour format R G B A R G B A R G B A- 2 bytes per pixel */
    br_uint_16 temp_colour;

    temp_colour = ((BR_RED(new_pixel) >> 4) << 12) | ((BR_GRN(new_pixel) >> 4) << 8) | ((BR_BLU(new_pixel) >> 4) << 4) |
                  ((BR_ALPHA(new_pixel) >> 4));

    *((br_uint_16 *)pixels) = temp_colour;
}

void T_Write_ARGB_4444(char *pixels, br_float fract, br_colour new_pixel)
{
    /* colour format R G B A R G B A R G B A- 2 bytes per pixel */
    br_uint_16 temp_colour;

    temp_colour = ((BR_ALPHA(new_pixel) >> 4) << 12) | ((BR_RED(new_pixel) >> 4) << 8) |
                  ((BR_GRN(new_pixel) >> 4) << 4) | ((BR_BLU(new_pixel) >> 4));

    *((br_uint_16 *)pixels) = temp_colour;
}

// clang-format off

/*
 * Lookup table that tells everything needed about supported pixelmaps
 */
t_pixelmap_type_info PixelmapTypes[] = {

	NULL, NULL, NULL, 0, BR_PMT_INDEX_1, "BR_PMT_INDEX_1", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_INDEX_2, "BR_PMT_INDEX_2", "Not supported",
	T_Read_Index_4, T_Write_Index_4, T_Read_Flip_Index_4, 1, BR_PMT_INDEX_4, "BR_PMT_INDEX_4", "4 bit indexed",
	T_Read_Index_8, T_Write_Index_8, T_Read_Flip_Index_8, 1, BR_PMT_INDEX_8, "BR_PMT_INDEX_8", "8 bit indexed",

	T_Read_RGB_555, T_Write_RGB_555, T_Read_RGB_555, 0, BR_PMT_RGB_555, "BR_PMT_RGB_555", "RGB 16 bit 5 bits per colour component",
	T_Read_RGB_565, T_Write_RGB_565, T_Read_RGB_565, 0, BR_PMT_RGB_565, "BR_PMT_RGB_565", "RGB 16 bit 5,6,5 bit colour components",
	T_Read_RGB_888, T_Write_RGB_888, T_Read_RGB_888, 0, BR_PMT_RGB_888, "BR_PMT_RGB_888", "RGB 24 bit 8 bits per colour component",
	T_Read_RGBX_888, T_Write_RGBX_888, T_Read_RGBX_888, 0, BR_PMT_RGBX_888, "BR_PMT_RGBX_888", "RGB 32 bit 8 bits per colour component",
	T_Read_RGBX_888, T_Write_RGBX_888, T_Read_RGBX_888, 0, BR_PMT_RGBA_8888, "BR_PMT_RGBA_8888", "RGBA 32 bit 8 bits per colour component",

	NULL, NULL, NULL, 0, BR_PMT_YUYV_8888, "BR_PMT_YUYV_8888", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_YUV_888, "BR_PMT_YUV_888", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_DEPTH_16, "BR_PMT_DEPTH_16", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_DEPTH_32, "BR_PMT_DEPTH_32", "Not supported",
	T_Read_Alpha_8, T_Write_Alpha_8, T_Read_Alpha_8, 1, BR_PMT_ALPHA_8, "BR_PMT_ALPHA_8", "Opacity map",

	T_Read_IndexA_88, NULL, T_Read_Flip_IndexA_88, 0, BR_PMT_INDEXA_88, "BR_PMT_INDEXA_88", "Opacity + index",
	NULL, NULL, NULL, 0, BR_PMT_NORMAL_INDEX_8, "BR_PMT_NORMAL_INDEX_8","Not supported",
	NULL, NULL, NULL, 0, BR_PMT_NORMAL_XYZ, "BR_PMT_NORMAL_XYZ", "Not supported",
	T_Read_BGR_555, T_Write_BGR_555, T_Read_BGR_555, 0, BR_PMT_BGR_555, "BR_PMT_BGR_555", "Reversed 16 bit 5 bits per colour component",
    T_Read_ARGB_4444, T_Write_ARGB_4444, T_Read_ARGB_4444, 0, BR_PMT_ARGB_4444, "BR_PMT_ARGB_4444", "ARGB 16 bit 4 bits per colour component",

    NULL, NULL, NULL, 0, BR_PMT_RBG_bab     , "BR_PMT_RGB_bab", "Not supported",
    NULL, NULL, NULL, 0, BR_PMT_RBG_1aba    , "BR_PMT_RGB_1aba", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_RGB_332		, "BR_PMT_RGB_332", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_DEPTH_8		, "BR_PMT_DEPTH_8", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_ARGB_8888	, "BR_PMT_ARGB_8888", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_ALPHA_4		, "BR_PMT_ALPHA_4", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_INDEXA_44	, "BR_PMT_INDEXA_44", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_DEPTH_15	, "BR_PMT_DEPTH_15", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_DEPTH_31	, "BR_PMT_DEPTH_31", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_DEPTH_FP16	, "BR_PMT_DEPTH_FP16", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_DEPTH_FP15	, "BR_PMT_DEPTH_FP15", "Not supported",

	NULL, NULL, NULL, 0, BR_PMT_RGBA_5551	, "BR_PMT_RGBA_5551", "Not supported",
	NULL, NULL, NULL, 0, BR_PMT_ARGB_1555	, "BR_PMT_ARGB_1555", "Not supported",

    T_Read_RGBA_4444, T_Write_RGBA_4444, T_Read_RGBA_4444, 0, BR_PMT_RGBA_4444, "BR_PMT_RGBA_4444", "RGBA 16 bit 4 bits per colour component",

};

int PixelmapTypesSize = BR_ASIZE(PixelmapTypes);

// clang-format on

/*
 * generate string of pixelmap type
 */
const char *T_EncodePixelmapType(br_int_32 type)
{
    int   i;
    char *type_str;

    for(i = 0; i < PixelmapTypesSize; i++)
        if(type == PixelmapTypes[i].type) {
            type_str = PixelmapTypes[i].identifier;
            break;
        }

    if(i >= PixelmapTypesSize)
        BR_ERROR1("Unknown pixelmap type '%d'", type);

    return type_str;
}

/*
 * convert to a direct colour pixelmap
 */
br_pixelmap *T_ConvertDirectPixelmap(br_pixelmap *item, t_pixelmap_cbfn_info *cbfn_command)
{

    br_pixelmap *pm;
    br_uint_32   x, y;
    char        *s_pixels, *d_pixels;
    br_float     s_fract, d_fract;
    br_float     s_x_step, d_x_step;
    br_float     d_x_start, s_x_start;
    br_float     d_y_step, s_y_step;
    br_float     d_y_start;

    t_read_pixel_fn_type  *read_pixel_fn;
    t_write_pixel_fn_type *write_pixel_fn;

    /*
     * allocate new pixelmap
     */
    if(cbfn_command->type == T_CBFN_FLIP_HIGH_WIDE_PALETTES)
        pm = BrPixelmapAllocate(cbfn_command->new_type, item->height, item->width, NULL, 0);
    else
        pm = BrPixelmapAllocate(cbfn_command->new_type, item->width, item->height, NULL, 0);

    pm->identifier = BrResStrDup(pm, item->identifier);

    /*
     * convert to direct colour pixelmap
     */
    d_y_start = pm->base_y * pm->row_bytes;

    s_pixels = ((char *)item->pixels) + (item->base_y * item->row_bytes);
    d_pixels = ((char *)pm->pixels) + (int)d_y_start;

    s_x_step = ((br_float)BrPixelmapPixelSize(item)) / 8.0;
    d_x_step = ((br_float)BrPixelmapPixelSize(pm)) / 8.0;

    s_y_step = item->row_bytes;
    d_y_step = pm->row_bytes;

    s_x_start = item->base_x * s_x_step;
    d_x_start = pm->base_x * d_x_step;

    read_pixel_fn  = PixelmapTypes[item->type].read_pixel_fn;
    write_pixel_fn = PixelmapTypes[pm->type].write_pixel_fn;

    if(cbfn_command->type == T_CBFN_FLIP_X_PIXELMAP) {
        d_x_start += ((pm->width - 1) * d_x_step);
        d_x_step = -d_x_step;

        read_pixel_fn = PixelmapTypes[item->type].read_flip_pixel_fn;
    }

    if(cbfn_command->type == T_CBFN_FLIP_Y_PIXELMAP) {

        d_pixels += ((pm->height - 1) * pm->row_bytes);
        d_y_step = -d_y_step;

        read_pixel_fn = PixelmapTypes[item->type].read_flip_pixel_fn;
    }

    current_src = item;
    current_dst = pm;

    if(cbfn_command->type == T_CBFN_FLIP_HIGH_WIDE_PALETTES) {
        for(y = 0, s_fract = s_x_start, d_fract = d_x_start; y < item->height; y++, s_pixels += (int)s_y_step,
        d_pixels = ((char *)pm->pixels) + (int)d_y_start, d_fract += d_x_step, s_fract = s_x_start) {
            for(x = 0; x < item->width; x++, s_fract += s_x_step, d_pixels += (int)d_y_step) {
                write_pixel_fn(d_pixels + ((int)d_fract), d_fract - ((int)d_fract),
                               read_pixel_fn(s_pixels + ((int)s_fract), s_fract - ((int)s_fract)));
            }
        }
    } else {
        for(y = 0, s_fract = s_x_start, d_fract = d_x_start; y < item->height;
            y++, s_pixels += (int)s_y_step, d_pixels += (int)d_y_step, s_fract = s_x_start, d_fract = d_x_start) {
            for(x = 0; x < item->width; x++, s_fract += s_x_step, d_fract += d_x_step) {
                write_pixel_fn(d_pixels + ((int)d_fract), d_fract - ((int)d_fract),
                               read_pixel_fn(s_pixels + ((int)s_fract), s_fract - ((int)s_fract)));
            }
        }
    }

    return pm;
}

br_pixelmap *crocconvert_to_rgb888(br_pixelmap *pm)
{
    t_pixelmap_cbfn_info info = (t_pixelmap_cbfn_info){
        .type     = T_CBFN_CONVERT_PIXELMAP,
        .new_type = BR_PMT_RGB_888,
        .width    = pm->width,
        .height   = pm->height,
        .origin_x = pm->origin_x,
        .origin_y = pm->origin_y,
    };

    return T_ConvertDirectPixelmap(pm, &info);
}