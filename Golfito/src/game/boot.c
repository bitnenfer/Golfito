#include "boot.h"
#include "../core/gfx.h"
#include "../core/input.h"
#include "../core/memory.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SPRITES 8000
typedef struct {
    vec2_t position;
    vec2_t scaleRotation;
    float32_t rotSpeed;
    uint32_t frame;
    uint32_t color;
} Sprite;
typedef struct {
    float32_t x, y;
    float32_t w, h;
} Frame;
static Frame frames[3] = { { 0, 0, 61, 99}, { 61, 0, 120, 120 }, { 181, 0, 114, 159 } };
static TextureID sampleTexture = INVALID_TEXTURE_ID;
static TextureID otherTexture = INVALID_TEXTURE_ID;
static Sprite sprites[MAX_SPRITES] = { 0.0f };
static uint32_t count = 1;
static uint32_t currentFrame = 0;
static vec2_t textureSize = { 0.0f, 0.0f };

float32_t crappy_random () {
    return ((float32_t)rand()) / ((float32_t)RAND_MAX);
}

void game_sys_initialize(void) {
    
}

void game_sys_shutdown(void) {
    
}

void game_start (void) {
#if defined(_WIN32)
    sampleTexture = gfx_load_texture("../assets/sheet.png");
    assert(sampleTexture != INVALID_TEXTURE_ID);
    otherTexture = gfx_load_texture("../assets/image.png");
    assert(sampleTexture != INVALID_TEXTURE_ID);
#else
	sampleTexture = gfx_load_texture("sheet.png");
	assert(sampleTexture != INVALID_TEXTURE_ID);
    otherTexture = gfx_load_texture("image.png");
    assert(otherTexture != INVALID_TEXTURE_ID);
#endif
    textureSize = gfx_get_texture_size(sampleTexture);
    srand((uint32_t)time(NULL));
    vec2_t size = gfx_get_view_size();
    sprites[0].position.x = size.x * crappy_random();
    sprites[0].position.y = size.y * crappy_random();
    sprites[0].scaleRotation.x = 0.8f + crappy_random() * 0.5f;
    sprites[0].scaleRotation.y = crappy_random();
    sprites[0].rotSpeed = crappy_random() * 0.01f;
    sprites[0].frame = currentFrame;
    sprites[0].color = COLOR_WHITE;
    currentFrame = 0;
}
void game_end (void) {}
void game_loop (float32_t dt) {
    gfx_set_clear_color(0.0f, 0.0f, 0.0f, 1.0f);
    
    gfx_set_pipeline(PIPELINE_TEXTURE);
//    gfx_draw_texture(otherTexture, 0, 0);
    
    for (uint32_t index = 0; index < count; ++index) {
        Sprite* pSprite = &sprites[index];
        Frame frame = frames[pSprite->frame];
        gfx_push_matrix();
        gfx_translate(pSprite->position.x, pSprite->position.y);
        gfx_rotate(pSprite->scaleRotation.y);
        gfx_scale(pSprite->scaleRotation.x, pSprite->scaleRotation.x);
        gfx_draw_texture_frame_with_color(sampleTexture, -frame.w / 2, -frame.h / 2, frame.x, frame.y, frame.w, frame.h, pSprite->color);
        gfx_pop_matrix();
        pSprite->scaleRotation.y += pSprite->rotSpeed;
    }
    
//    gfx_draw_texture_with_color(otherTexture, 200, 200, GET_COLOR_RGB_U32(0xff, 0, 0));
    gfx_flush();
    
    if (input_pointer_hit(0)) {
        vec2_t pos = input_pointer_position(0);
        if (count < MAX_SPRITES) {
            Sprite sprite;
            sprite.position = pos;
            sprite.rotSpeed = -0.01f + crappy_random() * 0.02f;
            sprite.scaleRotation.x = 0.5f + crappy_random() * 0.8f;
            sprite.scaleRotation.y = crappy_random();
            sprite.color = GET_COLOR_RGB_F32(crappy_random(), crappy_random(), crappy_random());
            sprite.frame = currentFrame;
            sprites[count] = sprite;
            count += 1;
        }
    } else {
        if (input_pointer_up(0)) {
            currentFrame = (currentFrame + 1) % 3;
        }
    }
}
