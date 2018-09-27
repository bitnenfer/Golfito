#if defined(TARGET_MACOS)
#include "input.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>

#define MAX_TOUCHES 1
#define INPUT_DOWN 0
#define INPUT_HIT 2
#define INPUT_MOVE 4
#define INPUT_UP 8

typedef struct {
    vec2_t position;
    uint16_t state;
} TouchInputMacOS;

typedef struct {
    TouchInputMacOS touch[MAX_TOUCHES];
} InputMacOS;

static InputMacOS gInputState = { 0 };

bool32_t input_initialize () {
    memset((void*)&gInputState, 0, sizeof(gInputState));
    return UT_TRUE;
}

bool32_t input_pointer_down (uint32_t pointerID) {
    if (pointerID >= MAX_TOUCHES) return UT_FALSE;
    return UT_IS_TRUE(gInputState.touch[0].state, INPUT_DOWN);
}

bool32_t input_pointer_hit (uint32_t pointerID) {
    if (pointerID >= MAX_TOUCHES) return UT_FALSE;
    if (UT_IS_TRUE(gInputState.touch[0].state, INPUT_HIT)) {
        UT_SET_FALSE(gInputState.touch[0].state, INPUT_HIT);
        return UT_TRUE;
    }
    return UT_FALSE;
}

bool32_t input_pointer_move (uint32_t pointerID) {
    if (pointerID >= MAX_TOUCHES) return UT_FALSE;
    return UT_IS_TRUE(gInputState.touch[0].state, INPUT_MOVE);
}

bool32_t input_pointer_up(uint32_t pointerID) {
    if (pointerID >= MAX_TOUCHES) return UT_FALSE;
    TouchInputMacOS* pState = &gInputState.touch[0];
    if (UT_IS_TRUE(pState->state, INPUT_UP)) {
        UT_SET_FALSE(pState->state, INPUT_UP);
        return UT_TRUE;
    }
    return UT_FALSE;
}

vec2_t input_pointer_position (uint32_t pointerID) {
    if (pointerID >= MAX_TOUCHES) { vec2_t pos = { 0.0f, 0.0f }; return pos; }
    return gInputState.touch[0].position;
}

void _input_update_down (uint32_t pointerID, float32_t x, float32_t y) {
    TouchInputMacOS* pState = &gInputState.touch[0];
    uint32_t state = pState->state;
    pState->position.x = x;
    pState->position.y = y;
    if (UT_IS_FALSE(state, INPUT_DOWN)) {
        UT_SET_TRUE(state, INPUT_HIT);
    }
    UT_SET_TRUE(state, INPUT_DOWN);
    UT_SET_FALSE(state, INPUT_UP);
    pState->state = state;
}

void _input_update_up (uint32_t pointerID, float32_t x, float32_t y) {
    TouchInputMacOS* pState = &gInputState.touch[0];
    uint32_t state = pState->state;
    pState->position.x = x;
    pState->position.y = y;
    UT_SET_TRUE(state, INPUT_UP);
    UT_SET_FALSE(state, INPUT_DOWN);
    UT_SET_FALSE(state, INPUT_HIT);
    pState->state = state;
}

void _input_update_move (uint32_t pointerID, float32_t x, float32_t y) {
    TouchInputMacOS* pState = &gInputState.touch[0];
    uint32_t state = pState->state;
    bool32_t isMoving = (pState->position.x != x || pState->position.y != y);
    pState->position.x = x;
    pState->position.y = y;
    if (isMoving) UT_SET_TRUE(state, INPUT_MOVE);
    else UT_SET_FALSE(state, INPUT_MOVE);
    pState->state = state;
}
#endif
