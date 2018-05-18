#include <assert.h>
#include <stdlib.h>
#include "ansicanvas.h"
#include "gfx_sdl.h"
#include "bmf.h"

ANSICanvas *canvas = NULL;
BitmapFont *myfont = NULL;

#define width 80
#define height 24

uint16_t tty_x = 0;
uint16_t tty_y = 0;
extern uint16_t current_x;
extern uint16_t current_y;
uint16_t last_x;
uint16_t last_y;

extern bool allow_clear;


int ansitty_init()
{
    ANSIRaster *r = NULL;
    char *font_filename = NULL;
    printf("ansitty_init()\n");

    font_filename = "bmf/8x8.bmf";
    myfont = bmf_load(font_filename);
    if (!myfont) {
        perror("bmf_load");
        exit(1);
    }

    allow_clear = true;

    canvas = new_canvas();

    /* very specific settings needed to make the canvas behave as a TTY */

    canvas->allow_hard_clear = true;
    canvas->repaint_entire_canvas = false;
    canvas->scroll_on_output = false;
    canvas->scroll_limit = height;

    for (int i = 0; i < 24; i++) {
        r = canvas_add_raster(canvas);
        raster_extend_length_to(r, 80);
    }

    gfx_sdl_main((width*8), (height*16), "z80p");
    //gfx_sdl_expose();
    canvas->is_dirty = true;
    return 0;

}

int ansitty_scroll(ANSICanvas *canvas)
{
    ANSIRaster *r = NULL, *d = NULL, *n = NULL;
    r = canvas_add_raster(canvas);
    assert(r);
    raster_extend_length_to(r, 80);
    /* get old head */
    d = canvas_get_raster(canvas, 0);
    assert(d);
    /* get new head */
    n = canvas_get_raster(canvas, 1);
    assert(n);
    /* point start of list to new head */
    assert(canvas->first_raster);
    canvas->first_raster = n;
    /* TODO: free old head, there is a memory leak here */
    canvas->lines --;
    //tty_y --;
    /* force refresh of entire canvas */
    canvas_reindex(canvas);
    gfx_sdl_clear();
    gfx_sdl_canvas_render(canvas, myfont);
    //gfx_sdl_expose();
    canvas->is_dirty = true; 
//    assert(NULL);
    return 0;
}

int ansitty_putc(unsigned char c)
{
    char outbuffer[2];
    last_x = current_x;
    last_y = current_y;

    /* check if output would cause a scroll before proceeding */

        if (current_x > (width  - 1)) {
            current_x = 0;
            current_y ++;
        }

//    if (c == '\n') {
        //assert(current_y < (canvas->scroll_limit-1));
        /* just scroll, and let libansicanvas handle the wrapping */

        if (current_y == height) {
            ansitty_scroll(canvas);
            current_y -= 1;
        }

        tty_x = current_x;
        tty_y = current_y;

//    }

    //send_byte_to_canvas(canvas, c);
    last_x = current_x;
    last_y = current_y; 
    outbuffer[0] = c;
    if (!ansi_to_canvas(canvas, &outbuffer, 1, 0)) {
        printf("+++ error!\n");
        assert(NULL);
    }
    tty_x = current_x;
    tty_y = current_y;

    if (canvas->repaint_entire_canvas) {
        printf("FULL CANVAS REFRESH\n");
        gfx_sdl_canvas_render(canvas, myfont);
        canvas->repaint_entire_canvas = false;
    } else {
        /* regular output */
        assert(tty_y <= canvas->scroll_limit);
        if (c != '\n' && c!= '\r') {
            tty_x = current_x;
            tty_y = current_y;
            gfx_sdl_canvas_render_xy(canvas, myfont, last_x, last_y);
            //gfx_sdl_expose();
            canvas->is_dirty = true; 
            }
    }

    return 0;
}

int _ansitty_putc(unsigned char c)
{
    ANSIRaster *r = NULL;
    ANSIRaster *d = NULL;
    ANSIRaster *n = NULL;

    if (c == '\r') {
        tty_x = 0;
        return 1;
    }

    if (c == '\n') {
        tty_x = 0;
        tty_y ++;
        //return 1;
        c = 0;
    }

    r = canvas_get_raster(canvas, tty_y);
    if (tty_y == height && r == NULL) {
        /* HARDWARE SCROLL */
        r = canvas_add_raster(canvas);
        assert(r);
        raster_extend_length_to(r, 80);
        /* get old head */
        d = canvas_get_raster(canvas, 0);
        assert(d);
        /* get new head */
        n = canvas_get_raster(canvas, 1);
        assert(n);
        /* point start of list to new head */
        assert(canvas->first_raster);
        canvas->first_raster = n;
        /* TODO: free old head, there is a memory leak here */
        canvas->lines --;
        tty_y --;
        /* force refresh of entire canvas */
        canvas_reindex(canvas);
        gfx_sdl_clear();
        gfx_sdl_canvas_render(canvas, myfont);
        //gfx_sdl_expose();
        canvas->is_dirty=true; 
    }

    if (tty_y >= height) {
        tty_y = height -1;
        r = canvas_get_raster(canvas, tty_y);
    }

    if (r == NULL) {
        printf("error: couldn't get raster %u\n", tty_y);
        assert(r);
    }

    assert(r->chardata);
    assert(r->bytes == 80);

    if (tty_x >= 80) {
        printf("error: ttyx >= 80 == %u\n", tty_x);
        assert(tty_x < 80);
    }

    if (c == 0) {
        return 1;
    }
    r->chardata[tty_x] = c;
    //send_byte_to_canvas(canvas, c);
    /* FIXME: this is incredibly slow. add a method to gfx_sdl just to update a particular byte or region */
    gfx_sdl_canvas_render_xy(canvas, myfont, tty_x, tty_y);
    //gfx_sdl_canvas_render(canvas, myfont);
    //gfx_sdl_expose();
    canvas->is_dirty = true; 

    tty_x ++;
    return 1;
}

void ansitty_expose()
{
    gfx_sdl_expose();

}

bool ansitty_canvas_getdirty()
{
    return canvas->is_dirty;

}

void ansitty_canvas_setdirty(bool state)
{
    canvas->is_dirty = state;

}
