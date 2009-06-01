#ifndef MP_EDITOR_H
#define MP_EDITOR_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/* editor.h */


extern void display_left(Index);
extern void display_right(Index);
extern void up(Index);
extern void up_selection(void*);
extern void down(Index);
extern void forward_char(Index);
extern void backward_char(Index);
extern void forward_line(Index);
extern void backward_line(Index);
extern void begin_of_line(void);
extern void end_of_line(void);
extern void scroll_up(Index);
extern void scroll_down(Index);
extern void recenter(void);
extern void move_to_center(void);
extern void begin_of_buffer(void);
extern void end_of_buffer(void);
extern void forward_word(Index);
extern void backward_word(Index);
extern void transpose_chars(Index);
extern void transpose_words(Index);
extern void upcase_word(void);
extern void downcase_word(void);

/* extern void next_node_or_text(KEYCODE, Index); */
extern void next_node_or_insert(int, Index);
extern void next_node_insert(int, Index);
extern void openparen_insert(int, Index);
extern void closeparen_insert(int, Index);
extern void make_list_insert(int, Index);
extern void insert_char(int,Index);
extern void insert_string(Char *);
extern void insert_expr(Index);
extern void insert_text(Index);
extern void insert_disp(Index);
extern void insert_var(Index);
extern void insert_id(Index);
extern void insert_op(Index);
extern void insert_newline(Index);
extern void insert_hard_newline(Index);
extern void insert_rtab(Index);
extern void insert_symbol(int, Index);

extern void backward_remove_char(Index);
extern void forward_remove_char(Index);
extern void remove_double_chars(void);
extern void remove_region(void);
extern void kill_region(void);
extern void kill_word(Index);
extern void backward_kill_word(Index);
extern void kill_line(Index);
extern void backward_kill_line(Index);
extern void append_next_kill(void);
extern void yank(void);

extern void set_index_nr(Index);
extern void increase_spacing(Index);
extern void decrease_spacing(Index);
extern void reset_spacing(void);
extern void raise_node(void);
extern void lower_region(void);
extern void copy_region(void);
extern void swap_region(void);
extern void set_parens(void);
extern void unset_parens(void);
extern void clear_parens(void);
extern void switch_parens(void);
extern void insert_notation(Index);
extern void rename_id(void);
extern void apply(void);
extern void distribute(void);
extern void factorise(void);
extern void commute(void);
extern void goto_latex_line(Index);
extern void latex_text_only(Bool tonly);
extern void latex_all_parens(Bool allparens);
extern void latex_selection(int selnum);

extern void join_selections(Index,Index);
extern void swap_selections(Index,Index);
extern void copy_selections(Index,Index);
extern void unset_select(Index);
extern int  ps_notation(Offset *vnr);
extern int  ss_notation(Offset *vnr);
extern int  selected_notation(int selnr, Offset *vnr);
extern int  ps_id_font(void);
extern void new_id_font(Offset nfnr);
extern void new_version(Offset nnnr);
extern void stack_position(void);
extern void use_stack(void);
extern void clear_stack(void);
extern void clear_stack_and_use(void);
extern Bool check_find(void);
extern Bool check_find_replace(void);
extern Bool find_tree(void);
extern void replace_tree(void);
extern void replace_all_tree(void);
extern void remove_find_stack(void);
extern void find_prev_on_stack(void);
extern void find_next_on_stack(void);
extern void find_new_on_stack(void);
extern int  get_findrep(Char *, int*,int);
extern int  put_findrep(void);
extern Bool find_string(Char *);
extern Bool find_backward_string(Char*);
extern Bool findprev_string(Char *);
extern Bool findnext_string(Char *);
extern Bool findwrap_backward_string(Char *);
extern Bool findwrap_string(Char *);
extern Bool find_replace(Char *);
extern Bool findnext_replace(Char *);
extern void replace_string(Char *, Char *);
extern void replace_all(Char *, Char *);
extern Bool find_stencil(Index);
extern Bool find_backward_stencil(Index);
extern Bool findprev_stencil(Index);
extern Bool findnext_stencil(Index);
extern Bool findwrap_backward_stencil(Index);
extern Bool findwrap_stencil(Index);
extern Bool find_replace_stencil(Index);
extern Bool findnext_replace_stencil(Index);
extern void replace_notation(Index,Index);
extern void replace_all_notation(Index, Index);

extern void construct_selection(void **);
extern void destruct_selection(void *);
extern void copy_selection(void **, void *);
extern void filter_template_selection(void *, Index *);
extern void insert_template_selection(void *, Index);
extern void insert_string_selection(void *, Char *);
extern void commute_selection(void *);
extern void next_node_selection(void *);
extern void insert_parse_result(void *);
extern void insert_selection(void *, Char);
extern void filter_selection(void *, Char *);
extern int  get_selection_path(void *full_sel, void *sub_sel,
			       int *list, int max);
extern void change_selection(void *, int*, int);
extern void redraw_selection(void *);
extern void *get_selection(int nr);

extern void draw(void *);
extern void update_selections(void);
extern void redraw_window(void*);
extern void word_wrap_window(void*);
extern void word_wrap_selection(int);
extern void clear_tab_positions(void);
extern void resize_window(void*,unsigned int,unsigned int);
extern void other_window(void*);
extern void editwindow_line(void*,int);
extern void editwindow_topto(void*, Char*);
extern int line_number(void *);
extern int number_of_lines(void*);
extern void cursor_use_xor(Bool);

extern void *open_miniwindow(void*,unsigned int, unsigned int);
extern void *open_scratchwindow(void*,unsigned int,unsigned int);
extern void close_scratchwindow(void);
extern void *open_findwindow(void*,unsigned int,unsigned int);
extern void close_findwindow(void);
extern void *open_replacewindow(void*,unsigned int,unsigned int);
extern void close_replacewindow(void);
extern void *open_editwindow(void*,unsigned int,unsigned int);
extern void close_editwindow(void*);
extern void old_load_editwindow(void*,FILE*);
extern void old_include_editwindow(void*, FILE*);
extern void append_editwindow(void*, Char *, unsigned int);
extern void append_structure(void*);
extern void load_editwindow(void*);
extern void include_editwindow(void*);
extern void include_selection(void);
extern void save_editwindow(void*);
extern Bool window_changed(void*);
extern Bool window_empty(void*);
extern void latex_editwindow(void*);
extern void clear_window(void*);
extern int  get_node(Char*,int*,int);
extern int  get_ascii_node(Char*,int*,int);
extern void cleanup_nodestack(void);
extern void *make_node(Char type, Char *txt, int len, int nnr, int spacing);
extern void apply_leibnitz(void);
extern void join_parse_stack(void);
extern void *add_parse_stack(Char *txt, int len);

extern void mouse_down(void*,int,int, Index button);
extern void mouse_move(int, int);
extern void mouse_up(int,int);
extern void dbl_click(void);

extern Bool move_selection;

extern Char *get_subnode_string(int selnr, int *posi, int len);
extern void *get_subnode(int selnr, int *posi, int len);
extern int  node_notation(void *node, int *vnr);
extern void latex_node(void *node);
extern void *first_node(void *selection);
extern void *last_node(void *selection);


#endif
