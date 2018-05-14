#include <SDL2/SDL.h>
#include <assert.h>
#include <stdbool.h>

bool shift_engaged = false;

#define KB_BUFSIZE  80

unsigned char keybuf[KB_BUFSIZE];
uint8_t kbip = 0;

/* TODO: reimplement as a bunch of tables for easier modification */

uint8_t tty_getbuflen()
{
    return kbip;
}

uint8_t tty_popkeybuf()
{
    uint8_t popped = 0;
    popped = keybuf[0];
    if (kbip) {
        kbip --;
        memmove(&keybuf, &keybuf+1, kbip);
        keybuf[kbip] = '\0';
        }
    return popped;
}

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
            case SDL_SCANCODE_SPACE:
                ascii_code = ' ';
                goto do_character;
                break;
            
            case SDL_SCANCODE_RETURN:
                ascii_code = '\r';
                goto do_character;
                break;
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
                if (key->keysym.scancode >= 30 && key->keysym.scancode < 39) {
                    ascii_code = 49 + (key->keysym.scancode - 30);
                    goto do_character;
                }

            switch (key->keysym.scancode) {
                case SDL_SCANCODE_EQUALS:
                    ascii_code = '=';
                    goto do_character;
                    break;
                case SDL_SCANCODE_0:
                    ascii_code = '0';
                    goto do_character;
                    break;
                case SDL_SCANCODE_PERIOD:
                    ascii_code = '.';
                    goto do_character;
                    break;
                case SDL_SCANCODE_SEMICOLON:
                    ascii_code = ';';
                    goto do_character;
                    break;
                default:
                    ascii_code = '\0';
                    goto do_character;
                    break;
                }

            case true:
                /* A-Z */
                if (key->keysym.scancode >= 4 && key->keysym.scancode <= 29) {
                    ascii_code = 65 + (key->keysym.scancode - 4);
                    goto do_character;
                }

            switch (key->keysym.scancode) {
                case SDL_SCANCODE_EQUALS:
                    ascii_code = '+';
                    goto do_character;
                    break;
                case SDL_SCANCODE_8:
                    ascii_code = '*';
                    goto do_character;
                    break;
                case SDL_SCANCODE_PERIOD:
                    ascii_code = '>';
                    goto do_character;
                    break;
                case SDL_SCANCODE_SEMICOLON:
                    ascii_code = ':';
                    goto do_character;
                    break;
                default:
                    ascii_code = '\0';
                    goto do_character;
                    break;
                }

            }
            printf("key->keysym.scancode = %u\n", key->keysym.scancode);
do_character:
            //printf("-> [%c]\n", ascii_code);
            /* append code to buffer if not overflowed */
            if (kbip <= KB_BUFSIZE) {
                keybuf[kbip] = ascii_code;
                kbip++;
                } else {
                printf("+++ keyboard buffer overflow\n");
                assert(NULL);
                }
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

