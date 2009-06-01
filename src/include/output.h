#ifndef MP_OUTPUT_H
#define MP_OUTPUT_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
**  output.h
*/

#define VISIBLE   0
#define INVISIBLE 1
#define SHADES    2
#define SMART     3

#define OFF       0
#define ON        1

#define CURSOR    0
#define MARK      1
#define CURSORHOLLOW 2
#define MARKHOLLOW   3

extern int more_keys(void);
extern int saved_chars;

extern void set_message_window(void *w);
extern void out_message(Char *txt);
extern void out_permanent_message(Char *txt);
extern void out_message_curs(Char *txt);
extern void clear_message(Bool allways);
extern void draw_message(void);
extern void *test_window(void);
extern void set_output_window(void *w);
extern void unset_output_window(void);
extern void adjust_lineheight(void);
extern void out_clear(void);
extern void switch_thick(void);
extern void switch_thin(void);
extern void switch_reverse(void);
extern void switch_visible(void);
extern void set_text_mode(int mode);
extern void set_underline(Bool on);
extern void set_default_thinspace(int n);
extern int  get_default_thinspace(void);
extern void set_italic(Bool on);
extern int  next_id_font(int nr);
extern void set_index(Bool on);
extern void set_drawstyle(int style);
extern void set_margin(int newmargin);
extern void set_smart_height(int height);
extern void set_search_func(void (*func)(void*), int x, int y);
extern void detect_margin(void);
extern int  where_x(void);
extern int  where_y(void);
extern void set_x_y(int new_x, int new_y);
extern void set_x(int new_x);
extern void set_y(int new_y);
/* extern int  char_width(Char data); */
extern void clear_to_end_of_page(void);
extern void move_content_up(int size, int height);
extern void move_content_down(int size, int height);
extern void flush(void);
extern void out_text_delim(int on);
extern void out_index(int data);
extern void out_cursor(int kind);
extern void open_node(void* data);
extern void close_node(void);
extern void out_char(Char data);
extern Bool display_tab(Char data);
extern void out_bold(Char *str);
extern void out_string(Char *str);
extern void out_char_string(char *str);
extern void out_char_bold(char *str);
extern void thinspace(int spacing);
extern int  line_height(void);
extern void output_init(void);
extern void put_mark(int x, int y);
extern void tab_unlock(void *ts);
extern void* tab_lock(void);
extern void set_tab_stack(void *ts, int nropen);
extern Bool tab_equal(void *ts);
extern void open_tabbing(void);
extern void close_tabbing(void);
extern void set_display_delta(int d);
extern int  get_display_delta(void);
extern void open_display(void);
extern void close_display(void);
extern void settabsize(int newsize);
extern int  getsimpletabsize(void);
extern void setsimpletabsize(int newsize);
extern void opspace(int size);
extern int output_important(void);

#endif
