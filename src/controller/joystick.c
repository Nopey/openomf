#include "controller/joystick.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include <stdlib.h>

#define UP -32768
#define DOWN 32767
#define LEFT -32768
#define RIGHT 32767
#define CENTER 0

void joystick_free(controller *ctrl) {
    joystick *k = ctrl->data;
    if(k->haptic) {
        SDL_CloseHaptic(k->haptic);
    }
    SDL_CloseGamepad(k->joy);
    omf_free(k->keys);
    omf_free(k);
}

static inline void joystick_cmd(controller *ctrl, int action, ctrl_event **ev) {
    controller_cmd(ctrl, action, ev);
}

int joystick_count(void) {
    int valid_joysticks = 0;
    SDL_Joystick *joy;
    for(int i = 0; i < SDL_NumJoysticks(); i++) {
        joy = SDL_OpenJoystick(i);
        if(joy) {
            valid_joysticks++;
        }
        if(SDL_JoystickConnected(joy)) {
            SDL_CloseJoystick(joy);
        }
    }
    return valid_joysticks;
}

int joystick_nth_id(int n) {
    SDL_Joystick *joy;
    int c = 0;
    for(int i = 0; i < SDL_NumJoysticks(); i++) {
        joy = SDL_OpenJoystick(i);
        if(joy) {
            c++;
            if(SDL_JoystickConnected(joy)) {
                SDL_CloseJoystick(joy);
            }
            if(c == n) {
                return i;
            }
        }
    }
    return -1;
}

int joystick_offset(int id, const char *name) {
    int offset = 0;
    for(int i = 0; i < id; i++) {
        if(i != id && !strcmp(name, SDL_JoystickNameForIndex(i))) {
            offset++;
        }
    }
    return offset;
}

int joystick_name_to_id(const char *name, int offset) {
    for(int i = 0; i < SDL_NumJoysticks(); i++) {
        if(!strcmp(name, SDL_JoystickNameForIndex(i))) {
            if(offset) {
                offset--;
            } else {
                return i;
            }
        }
    }
    return -1;
}

static int internal_joystick_poll(joystick *k, controller *ctrl, ctrl_event **ev, bool allow_esc) {
    if(!SDL_GamepadConnected(k->joy)) {
        controller_close(ctrl, ev);
        return 0;
    }

    Sint16 x_axis = SDL_GetGamepadAxis(k->joy, k->keys->x_axis);
    Sint16 y_axis = SDL_GetGamepadAxis(k->joy, k->keys->y_axis);
    int dpadup = SDL_GetGamepadButton(k->joy, k->keys->dpad[0]);
    int dpaddown = SDL_GetGamepadButton(k->joy, k->keys->dpad[1]);
    int dpadleft = SDL_GetGamepadButton(k->joy, k->keys->dpad[2]);
    int dpadright = SDL_GetGamepadButton(k->joy, k->keys->dpad[3]);

    int action = ACT_NONE;

    // joystick input
    // TODO the devide by 2 should be a 'dead zone' variable that can be set in the option menu but this devide works
    // well 99% of the time. Analog Stick (Axis 1) Movement
    if(x_axis <= LEFT / 2 && y_axis <= UP / 2) {
        action = ACT_UP | ACT_LEFT;
    } else if(x_axis <= LEFT / 2 && y_axis >= DOWN / 2) {
        action = ACT_DOWN | ACT_LEFT;
    } else if(x_axis >= RIGHT / 2 && y_axis <= UP / 2) {
        action = ACT_UP | ACT_RIGHT;
    } else if(x_axis >= RIGHT / 2 && y_axis >= DOWN / 2) {
        action = ACT_DOWN | ACT_RIGHT;
    } else if(x_axis >= RIGHT / 2) {
        action = ACT_RIGHT;
    } else if(x_axis <= LEFT / 2) {
        action = ACT_LEFT;
    } else if(y_axis <= UP / 2) {
        action = ACT_UP;
    } else if(y_axis >= DOWN / 2) {
        action = ACT_DOWN;
        // check the d-pad instead
    } else if(dpadup && dpadleft) {
        action = ACT_UP | ACT_LEFT;
    } else if(dpaddown && dpadleft) {
        action = ACT_DOWN | ACT_LEFT;
    } else if(dpadup && dpadright) {
        action = ACT_UP | ACT_RIGHT;
    } else if(dpaddown && dpadright) {
        action = ACT_DOWN | ACT_RIGHT;
    } else if(dpadright) {
        action = ACT_RIGHT;
    } else if(dpadleft) {
        action = ACT_LEFT;
    } else if(dpadup) {
        action = ACT_UP;
    } else if(dpaddown) {
        action = ACT_DOWN;
    }

    // button input
    if(SDL_GetGamepadButton(k->joy, k->keys->punch)) {
        action |= ACT_PUNCH;
    } else if(SDL_GetGamepadButton(k->joy, k->keys->kick)) {
        action |= ACT_KICK;
    }

    if(action) {
        joystick_cmd(ctrl, action, ev);
    }

    if(allow_esc && SDL_GetGamepadButton(k->joy, k->keys->escape)) {
        joystick_cmd(ctrl, ACT_ESC, ev);
    }

    if(ctrl->current == 0) {
        joystick_cmd(ctrl, ACT_STOP, ev);
    }

    return 0;
}

int joystick_poll(controller *ctrl, ctrl_event **ev) {
    joystick *k = ctrl->data;

    ctrl->last = ctrl->current;
    ctrl->current = 0;

    return internal_joystick_poll(k, ctrl, ev, false);
}

int joystick_rumble(controller *ctrl, float magnitude, int duration) {
    joystick *k = ctrl->data;
    if(k->rumble == 1) {
        SDL_PlayHapticRumble(k->haptic, magnitude, duration);
    }
    return 0;
}

static inline void internal_joystick_default_keys(joystick_keys *keys) {
    keys->x_axis = SDL_GAMEPAD_AXIS_LEFTX;
    keys->y_axis = SDL_GAMEPAD_AXIS_LEFTY;
    keys->dpad[0] = SDL_GAMEPAD_BUTTON_DPAD_UP;
    keys->dpad[1] = SDL_GAMEPAD_BUTTON_DPAD_DOWN;
    keys->dpad[2] = SDL_GAMEPAD_BUTTON_DPAD_LEFT;
    keys->dpad[3] = SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
    keys->punch = SDL_GAMEPAD_BUTTON_SOUTH;
    keys->kick = SDL_GAMEPAD_BUTTON_EAST;
    keys->escape = SDL_GAMEPAD_BUTTON_START;
}

int joystick_create(controller *ctrl, int joystick_id) {
    joystick *k = omf_calloc(1, sizeof(joystick));
    k->keys = omf_calloc(1, sizeof(joystick_keys));
    internal_joystick_default_keys(k->keys);
    k->rumble = 0;
    ctrl->data = k;
    ctrl->type = CTRL_TYPE_GAMEPAD;
    ctrl->poll_fun = &joystick_poll;
    ctrl->free_fun = &joystick_free;
    ctrl->supports_delay = true;

    k->joy = SDL_OpenGamepad(joystick_id);
    if(k->joy) {
        k->haptic = SDL_OpenHapticFromJoystick(SDL_GetGamepadJoystick(k->joy));
        if(k->haptic) {
            if(SDL_HapticRumbleSupported(k->haptic)) {
                if(SDL_InitHapticRumble(k->haptic) == 0) {
                    k->rumble = 1;
                    ctrl->rumble_fun = joystick_rumble;
                } else {
                    log_error("Failed to initialize rumble: %s", SDL_GetError());
                }
            } else {
                log_debug("Rumble not supported");
            }
        } else {
            log_debug("Haptic not supported");
        }
        log_debug("Game controller initialized");
        return 1;
    }

    log_error("Failed to open game controller: %s", SDL_GetError());
    return 0;
}

static vector every_gamepad;

void joystick_init(void) {
    int num_joysticks = SDL_NumJoysticks();
    vector_create_with_size(&every_gamepad, sizeof(SDL_Gamepad *),
                            num_joysticks);

    for(int idx = 0; idx < num_joysticks; idx++) {
        joystick_deviceadded(idx);
    }
}

void joystick_close(void) {
    iterator it;
    vector_iter_begin(&every_gamepad, &it);
    SDL_Gamepad **gamepad;
    foreach(it, gamepad) {
        SDL_CloseGamepad(*gamepad);
    }
    vector_free(&every_gamepad);
}

void joystick_menu_poll_all(controller *menu_ctrl, ctrl_event **ev) {
    if(vector_size(&every_gamepad) == 0) {
        return;
    }

    joystick k;
    memset(&k, 0, sizeof(k));
    joystick_keys keys;
    internal_joystick_default_keys(&keys);
    k.keys = &keys;

    iterator it;
    vector_iter_begin(&every_gamepad, &it);
    SDL_Gamepad **gamepad;
    foreach(it, gamepad) {
        k.joy = *gamepad;
        internal_joystick_poll(&k, menu_ctrl, ev, true);
    }
}

void joystick_deviceadded(int sdl_joystick_index) {
    SDL_Gamepad *gamepad = SDL_OpenGamepad(sdl_joystick_index);
    if(!gamepad) {
        return;
    }
    vector_append(&every_gamepad, &gamepad);
}

void joystick_deviceremoved(int sdl_joystick_instance_id) {
    unsigned gamepad_count = vector_size(&every_gamepad);
    for(unsigned idx = 0; idx < gamepad_count; idx++) {
        SDL_Gamepad *gamepad = *(SDL_Gamepad **)vector_get(&every_gamepad, idx);
        SDL_Joystick *joy = SDL_GetGamepadJoystick(gamepad);
        if(SDL_GetJoystickID(joy) == sdl_joystick_instance_id) {
            SDL_CloseGamepad(gamepad);
            vector_swapdelete_at(&every_gamepad, idx);
            return;
        }
    }
}
