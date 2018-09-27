#ifndef _GFX_H_
#define _GFX_H_

#include "types.h"
#include "math.h"

typedef void* TextureID;
#define INVALID_TEXTURE_ID NULL
#define GET_COLOR_RGBA_U32(red, green, blue, alpha) ((((red) & 0xFF) << 24) | (((green) & 0xFF) << 16) | (((blue) & 0xFF) << 8)) | (((alpha) & 0xFF))
#define GET_COLOR_RGB_U32(red, green, blue) GET_COLOR_RGBA_U32(red, green, blue, 0xFF)
#define GET_COLOR_RGBA_F32(red, green, blue, alpha) GET_COLOR_RGBA_U32((uint8_t)((red) * 255.0f), (uint8_t)((green) * 255.0f), (uint8_t)((blue) * 255.0f), (uint8_t)((alpha) * 255.0f))
#define GET_COLOR_RGB_F32(red, green, blue) GET_COLOR_RGBA_F32(red, green, blue, 1.0f)

#define COLOR_WHITE     GET_COLOR_RGBA_U32(0xFF, 0xFF, 0xFF, 0xFF)
#define COLOR_RED       GET_COLOR_RGBA_U32(0xFF, 0x00, 0x00, 0xFF)
#define COLOR_GREEN     GET_COLOR_RGBA_U32(0x00, 0xFF, 0x00, 0xFF)
#define COLOR_BLUE      GET_COLOR_RGBA_U32(0x00, 0x00, 0xFF, 0xFF)
#define COLOR_YELLOW    GET_COLOR_RGBA_U32(0xFF, 0xFF, 0x00, 0xFF)
#define COLOR_PINK      GET_COLOR_RGBA_U32(0xFF, 0x00, 0xFF, 0xFF)
#define COLOR_BLACK     GET_COLOR_RGBA_U32(0x00, 0x00, 0x00, 0xFF)

#define PIPELINE_TEXTURE 0
#define PIPELINE_LINE 1

void gfx_initialize (void);
void gfx_shutdown(void);
void gfx_begin (void);
void gfx_end (void);
void gfx_flush (void);
void gfx_resize (float32_t width, float32_t height);
void gfx_set_clear_color (float32_t r, float32_t g, float32_t b, float32_t a);
TextureID gfx_create_texture (uint32_t width, uint32_t height, const void* pPixels);
TextureID gfx_load_texture (const char* pTexturePath);
void gfx_draw_texture (TextureID texture, float32_t x, float32_t y);
void gfx_draw_texture_with_color (TextureID texture, float32_t x, float32_t y, uint32_t color);
void gfx_draw_texture_frame (TextureID texture, float32_t x, float32_t y, float32_t fx, float32_t fy, float32_t fw, float32_t fh);
void gfx_draw_texture_frame_with_color (TextureID texture, float32_t x, float32_t y, float32_t fx, float32_t fy, float32_t fw, float32_t fh, uint32_t color);
vec2_t gfx_get_texture_size (TextureID texture);
vec2_t gfx_get_view_size (void);
void gfx_push_matrix (void);
void gfx_pop_matrix (void);
void gfx_translate (float32_t x, float32_t y);
void gfx_scale (float32_t x, float32_t y);
void gfx_rotate (float32_t r);
void gfx_load_identity (void);
void gfx_vertex2 (float32_t x, float32_t y, uint32_t color);
void gfx_line (float32_t x0, float32_t y0, float32_t x1, float32_t y1, uint32_t color);
void gfx_line2 (float32_t x0, float32_t y0, float32_t x1, float32_t y1, uint32_t color0, uint32_t color1);

bool32_t gfx_set_pipeline (uint32_t pipeline);
float32_t gfx_get_pixel_ratio (void);

#endif
