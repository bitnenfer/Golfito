#ifndef _INPUT_H_
#define _INPUT_H_

#include "types.h"
#include "math.h"

bool32_t input_initialize(void);
bool32_t input_pointer_down(uint32_t pointerID);
bool32_t input_pointer_hit(uint32_t pointerID);
bool32_t input_pointer_move(uint32_t pointerID);
bool32_t input_pointer_up(uint32_t pointerID);
vec2_t input_pointer_position(uint32_t pointerID);

#endif
