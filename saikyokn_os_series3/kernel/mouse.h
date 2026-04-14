#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>

typedef struct {
    int x, y;
    int buttons;
    int dx, dy;
} mouse_state_t;

void mouse_init(void);
void mouse_poll(void);
void mouse_get_state(mouse_state_t *state);

#endif