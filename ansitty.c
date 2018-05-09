#include <assert.h>
#include <stdlib.h>
#include "ansicanvas.h"
#include "gfx_sdl.h"
#include "bmf.h"

ANSICanvas *canvas = NULL;
BitmapFont *myfont = NULL;

#define width 80
#define height 24

int tty_x = 0;
int tty_y = 0;

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


    canvas = new_canvas(); 

    for (int i = 0; i < 24; i++) {
        r = canvas_add_raster(canvas);
        raster_extend_length_to(r, 80); 
        }

    gfx_sdl_main((width*8), (height*16), "z80p");
    gfx_sdl_expose();
    return 0;

}

int ansitty_putc(unsigned char c)
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
        return 1;
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
        gfx_sdl_canvas_render(canvas, myfont);
        gfx_sdl_expose();
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

    r->chardata[tty_x] = c;
    /* FIXME: this is incredibly slow. add a method to gfx_sdl just to update a particular byte or region */
    gfx_sdl_canvas_render_xy(canvas, myfont, tty_x, tty_y);
    gfx_sdl_expose();
    tty_x ++;
    return 1;
}

