#include <SDL2/SDL.h>
#include <assert.h>
#include <stdbool.h>

bool shift_engaged = false;

int tty_processinput()
{
    SDL_Event event;
    SDL_KeyboardEvent *key;
    unsigned char ascii_code = 0;

    /* Poll for events. SDL_PollEvent() returns 0 when there are no  */
    /* more events on the event queue, our while loop will exit when */
    /* that occurs.                                                  */
    while( SDL_PollEvent( &event ) ) {
        /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
        key = &event.key;
        switch( event.type ) {
        case SDL_KEYDOWN:
            switch (key->keysym.scancode) {
            case SDL_SCANCODE_LSHIFT:
            case SDL_SCANCODE_RSHIFT:
                printf("[SHIFT_ON]\n");
                shift_engaged = true;
                return 0;
                break;
            case SDL_SCANCODE_ESCAPE:
                printf("[ESC_ON]\n");
                return 0;
                break;
            default:
                break;
            }
            switch (shift_engaged) {
            case false:
                /* a-z */
                if (key->keysym.scancode >= 4 && key->keysym.scancode <= 29) {
                    ascii_code = 97 + (key->keysym.scancode - 4);
                    goto do_character;
                }
                /* 0-9 */
                if (key->keysym.scancode >= 30 && key->keysym.scancode <= 39) {
                    ascii_code = 48 + (key->keysym.scancode - 30);
                    goto do_character;
                }
            case true:
                /* A-Z */
                if (key->keysym.scancode >= 4 && key->keysym.scancode <= 29) {
                    ascii_code = 65 + (key->keysym.scancode - 4);
                    goto do_character;
                }
            }
            printf("key->keysym.scancode = %u\n", key->keysym.scancode);
do_character:
            printf("-> [%c]\n", ascii_code);
            /* append code to buffer if not overflowed */
            return 1;
            break;
        case SDL_KEYUP:
            switch (key->keysym.scancode) {
            case SDL_SCANCODE_LSHIFT:
            case SDL_SCANCODE_RSHIFT:
                printf("[SHIFT_OFF]\n");
                shift_engaged = false;
                return 0;
                break;
            case SDL_SCANCODE_ESCAPE:
                printf("[ESC_OFF]\n");
                exit(0);
                break;
            default:
                break;
            }

            break;
        default:
            break;
        }
    }

   //printf("/* NO KEYBOARD IO */\n");
    return 0;
}

