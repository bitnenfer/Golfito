#ifndef _GAME_BOOT_H_
#define _GAME_BOOT_H_

#include "../core/types.h"

void game_sys_initialize(void);
void game_sys_shutdown(void);
void game_start(void);
void game_end(void);
void game_loop(float32_t dt);

#endif
