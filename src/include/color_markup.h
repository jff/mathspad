#ifndef MP_COLOR_MARKUP_H
#define MP_COLOR_MARKUP_H

#define color_set_red(C,V)   (((C)&0x00ffff)|(((V)&0xff)<<16))
#define color_set_green(C,V) (((C)&0xff00ff)|(((V)&0xff)<<8))
#define color_set_blue(C,V)  (((C)&0xffff00)|((V)&0xff))


#ifdef COLOR256

/* color cube: 8x8x4 (RxGxB) */

#define color_set_cube(C,V)  (((((((V)&0xE0)>>5)*0xff)/0x7)<<16)| \
                              ((((((V)&0x1B)>>2)*0xff)/0x7)<<8)| \
                              ( ( ((V)&0x3)     *0xff)/0x3)    )
#else

/* color cube: 6x6x6 (RxGxB) */
#define color_set_cube(C,V)  (((((V)/36)*0x33)<<16)| \
                              (((((V)/6)%6)*0x33)<<8)| \
                              ((((V)%6)*0x33)))
#endif

extern void color_markup_init(void);

#endif
