
#include "language.h"

static int call_noarg(int (*fcal)(), void **argl __attribute__((unused)) )
{
  return (*fcal)();
}

static int call_intarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((int*)argl[0]));
}

static int call_intintarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((int*)argl[0]), *((int*)argl[1]));
}

static int call_strarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((Char**)argl[0]));
}

static int call_strintintarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((Char**)argl[0]), *((int*)argl[1]), *((int*)argl[2]));
}

static int call_strintarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((Char**)argl[0]), *((int*)argl[1]));
}

static int call_intstrarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((int*)argl[0]), *((Char**)argl[1]));
}

static int call_intrefstrstrarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((int**)argl[0]),*((Char**)argl[1]), *((Char**)argl[2]));
}

static int call_strstrarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((Char**)argl[0]), *((Char**)argl[1]));
}

static int call_strstrintarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((Char**)argl[0]), *((Char**)argl[1]), *((int*)argl[2]));
}

static int call_intintintintintarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((int*)argl[0]), *((int*)argl[1]), *((int*)argl[2]),
		 *((int*)argl[3]), *((int*)argl[4]));
}

static int call_strstrstrstrfuncarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((Char**)argl[0]), *((Char**)argl[1]), *((Char**)argl[2]),
		 *((Char**)argl[3]), *((void**)argl[4]));
}

static int call_strfuncfuncarg(int (*fcal)(), void **argl)
{
  return (*fcal)(*((Char**)argl[0]), *((void**)argl[1]), *((void**)argl[2]));
}

static int call_resint(int (*fcal)(), void **argl)
{
  (*(*((int**)argl[0]))) = (*fcal)();
  return 0;
}

static int call_intintstrref(int (*fcal)(),void **argl)
{
  return (*fcal)(*((int*)argl[0]),*((int*)argl[1]),*((Char***)argl[2]));
}

typedef struct {
  Type tlist[8];
  Type rettype;
  int listlen;
  int (*callfunc)(int (*func)(), void **argl);
  Prototype *pt;
} PROTOLIST;

static PROTOLIST protolist[] = 
{
/*  0 */   { {0}, 0, 0, call_noarg, 0 },
/*  1 */   { {IntType}, 0, 1, call_intarg, 0 },
/*  2 */   { {IntType,IntType}, 0, 2, call_intintarg, 0},
/*  3 */   { {StringType}, 0, 1, call_strarg, 0},
/*  4 */   { {StringType, StringType}, 0, 2, call_strstrarg, 0 },
/*  5 */   { {IntType, IntType,IntType,IntType,IntType}, 0, 5,
    call_intintintintintarg,0},
/*  6 */   { {StringType, StringType, StringType, StringType, LazyEvalType }, 0, 5,
    call_strstrstrstrfuncarg, 0},
/*  7 */   { {StringType, LazyEvalType, LazyEvalType }, 0, 3,
    call_strfuncfuncarg, 0},
/*  8 */   { {ToRefType(IntType), StringType, StringType}, 0, 3,
    call_intrefstrstrarg, 0},
/*  9 */   { {StringType, StringType,IntType}, 0, 3, call_strstrintarg, 0 },
/* 10 */   { {StringType, IntType,IntType}, 0, 3, call_strintintarg, 0},
/* 11 */   { {StringType,IntType}, 0, 2, call_strintarg, 0},
/* 12 */   { {IntType,StringType}, 0, 2, call_intstrarg, 0},
/* 13 */   { {0}, IntType, 0,call_resint, 0},
/* 14 */   { {IntType, IntType, ToRefType(StringType)}, 0, 3, call_intintstrref,0},
/* 15 */   { {0}, 0, 0, 0}
};

#define PRONOARG 0
#define PROINTARG 1
#define PROINTINTARG 2
#define PROSTRINGARG 3
#define PROSTRINGSTRINGARG 4
#define PROINTINTINTINTINTARG 5
#define PROSTRSTRSTRSTRFUNCARG 6
#define PROSTRINGFUNCFUNCARG 7
#define PROINTREFSTRSTRARG 8
#define PROSTRSTRINTARG 9
#define PROSTRINTINTARG 10
#define PROSTRINTARG 11
#define PROINTSTRINGARG 12
#define PRONOARGINTRES 13
#define PROINTINTSTRINGREFARG 14

typedef struct {
  void (*func)();
  char *namefunc;
  char *description;
  int protopos;
} FUNCTIONTYPE;

static FUNCTIONTYPE keysfuncs[] = 
{
    { switch_textdots,
      "switch_textdots",
      "Toggle the dots around text place holders.",
      PRONOARG },
    { context_sensitive_help,
      "context_help",
      "Get context sensitive help, that is, help on the window "
	"that contains the mouse cursor",
	PRONOARG },
    { menu_filter_string,
      "insert_string",
      "Insert the string argument at the cursor position.",
      PROSTRINGARG },
    { menu_permanent_message,
      "set_message",
      "Display a permanent message in the console window.",
      PROSTRINGARG },
    { insert_expr,
      "insert_expression",
      "Insert an expression place holder. (N times)",
      PROINTARG },
    { insert_disp,
      "insert_display",
      "Insert a displayed expression place holder. (N times)",
      PROINTARG },
    { insert_var,
      "insert_variable",
      "Insert a variable place holder. (N times)",
      PROINTARG },
    { insert_id,
      "insert_identifier",
      "Insert an identifier place holder. (N times)",
      PROINTARG },
    { insert_op,
      "insert_operator",
      "Insert an operator place holder (N times)",
      PROINTARG },
    { insert_text,
      "insert_text",
      "Insert an text place holder. (N times)",
      PROINTARG },
    { raise_node,
      "ungroup_region",
      "Ungroup the selected region, removing one structural level.",
      PRONOARG },
    { lower_region,
      "group_region",
      "Group the selected region, adding a structural level.",
      PRONOARG },
    { rename_id,
      "rename_identifier",
      "Rename an identifier, respecting bounded variables.",
      PRONOARG },
    { commute,
      "commute_expression",
      "Reverse the order of selected expressions and operators.",
      PRONOARG },
    { factorise,
      "factorise_function",
      "Factorise a common function from the selected expression.",
      PRONOARG },
    { distribute,
      "distribute_function",
      "Distribute a function over the selected expression.",
      PRONOARG },
    { apply,
      "apply_function",
      "Apply a function to the selected expression.",
      PRONOARG },
    { clear_parens,
      "remove_parens",
      "Remove the parenthesis (does not include ungrouping)",
      PRONOARG },
    { switch_parens,
      "switch_parens",
      "Add or remove the parenthesis if it is save",
      PRONOARG },
    { set_parens,
      "add_parens",
      "Add parenthesis around an expression (includes grouping if necessary).",
      PRONOARG },
    { copy_region,
      "copy_region",
      "Copy the selected region to the current position.",
      PRONOARG },
    { swap_region,
      "swap_region",
      "Swap the two selected regions.",
      PRONOARG },
    { remove_region,
      "remove_region",
      "Remove the selected region",
      PRONOARG },
    { latex_select,
      "LaTeX_region",
      "Convert the selected region to LaTeX and make it available to the"
      " window system.",
      PROINTARG },
    { up,
      "expression_up",
      "Move up in the expression tree",
      PROINTARG },
    { down,
      "expression_down",
      "Move down in the expression tree",
      PROINTARG },
    { recenter,
      "recenter",
      "Recenter the content of the window around the current position.",
      PRONOARG },
    { move_to_center,
      "move_to_center",
      "Move the current position to the center of the window",
      PRONOARG },
    { scroll_up,
      "scroll_up",
      "Scroll the content one page up. (N times)",
      PROINTARG },
    { scroll_down,
      "scroll_down",
      "Scroll the content one page down. (N times)",
      PROINTARG },
    { transpose_chars,
      "transpose_chars",
      "Interchange the character left of the cursor with the character right of it and move the cursor one position forward. (N times)",
      PROINTARG },
    { transpose_words,
      "transpose_words",
      "Interchange the word left of the cursor with the word right of it and move the cursor one word forward. (N times)",
      PRONOARG },
    { goto_latex_line,
      "goto_latex_line",
      "Go to a specific LaTeX line. (Prompts for the line number in the console)",
      PROINTARG },
    { begin_of_line,
      "beginning_of_line",
      "Go to the beginning of the current line.",
      PRONOARG },
    { end_of_line,
      "end_of_line",
      "Go to the end of the current line.",
      PRONOARG },
    { backward_line,
      "previous_line",
      "Go to the end of the previous line. (N times)",
      PROINTARG },
    { forward_line,
      "next_line",
      "Go to the beginning of the next line. (N times)",
      PROINTARG },
    { backward_char,
      "backward_char",
      "Go one character backward. (N times)",
      PROINTARG },
    { forward_char,
      "forward_char",
      "Go one character forward. (N times)",
      PROINTARG },
    { begin_of_buffer,
      "beginning_of_buffer",
      "Go to the beginning of the document",
      PRONOARG },
    { end_of_buffer,
      "end_of_buffer",
      "Go to the end of the document",
      PRONOARG },
    { forward_word,
      "forward_word",
      "Go one word forward. (N times)",
      PROINTARG },
    { backward_word,
      "backward_word",
      "Go one word backward. (N times)",
      PROINTARG },
    { forward_remove_char,
      "delete_char",
      "Remove the character after the cursor, where structures are treated as one character. (N times)",
      PROINTARG },
    { backward_remove_char,
      "backward_delete_char",
      "Remove the character before the cursor, where structures are treated as one character. (N times)",
      PROINTARG },
    { remove_double_chars,
      "just_one_space",
      "Remove multiple occurances of characters if the character to the left and right of the cursor are equal.",
      PRONOARG },
    { display_left,
      "display_left",
      "Move a displayed expression to the left. (N times)",
      PROINTARG },
    { display_right,
      "display_right",
      "Move a displayed expression to the right. (N times)",
      PROINTARG },
    { kill_line,
      "kill_line",
      "Remove everything after the cursor, up to the end of the line and add it to the kill buffer. Structures are treated as line separators. If the next character is a line separator, it will be removed and added to the kill buffer. (N times)",
      PROINTARG },
    { make_project,
      "make_project",
      "Open the file selector to save the current project.",
      PRONOARG },
    { backward_kill_line,
      "backward_kill_line",
      "Remove everything before the cursor, up to the beginning of the line and add it to the kill buffer. Structures are treated as line separators. If the previous character is a line separator, it will be removed and added to the kill buffer. (N times)",
      PROINTARG },
    { kill_word,
      "kill_word",
      "Remove the next word and add it to the kill buffer. (N times)",
      PROINTARG },
    { backward_kill_word,
      "backward_kill_word",
      "Remove the previous word and add it to the kill buffer. (N times)",
      PROINTARG },
    { kill_region,
      "kill_region",
      "Remove the selected region and add it to the kill buffer.",
      PRONOARG },
    { yank,
      "yank",
      "Insert the content of the kill buffer at the cursor position.",
      PRONOARG },
    { append_next_kill,
      "append_next_kill",
      "Append additional content to the current kill buffer.",
      PRONOARG },
    { insert_newline,
      "soft_newline",
      "Move to the next empty place holder or first text position after the current structure.",
      PRONOARG },
    { insert_rtab,
      "insert_rtab",
      "Insert a tab character. (N times)",
      PROINTARG },
    { insert_hard_newline,
      "newline",
      "Insert a newline. (N times)",
      PROINTARG },
    /*    { start_argument,
      "universal_argument",
      "Start constructing the counter argument for other functions"
      " (for those who use it).",
      PRONOARG },
    { adjust_argument,
      "argument_adjust",
      "Adjust the counter argument by interpreting the pressed key.",
      PROINTARG }, */
    { increase_spacing,
      "increase_spacing",
      "Increase the spacing around the selected operator. (N times)",
      PROINTARG },
    { decrease_spacing,
      "decrease_spacing",
      "Decrease the spacing around the selected operator. (N times)",
      PROINTARG },
    { set_index_nr,
      "set_index_nr",
      "Set the index number of a place holder to N (in the find and replace window).",
      PROINTARG },
    { reset_spacing,
      "reset_spacing",
      "Reset the spacing around the selected operator to the default value.",
      PRONOARG },
    { menu_notation_shortcut,
      "insert_template",
      "Insert the template with the given name.",
      PROSTRINGARG },
    { menu_number_shortcut_string,
      "insert_version_str",
      "Insert the version with the give unique number (as string argument)",
      PROSTRINGARG },
    { menu_number_shortcut,
      "insert_version",
      "Insert the version with the give unique number",
      PROINTARG },
    { next_template_version,
      "next_version",
      "Switch to the next version of the selected template",
      PRONOARG },
    { new_id_font,
      "set_identifier_font",
      "Set the font of the current identifier to a specific value.",
      PROINTARG },
    { ask_selection,
      "insert_selection",
      "Insert the selection from the window system.",
      PRONOARG },
    { menu_insert_string,
      "selection_callback",
      "insert_selection sends a request to the window system. Use "
      "this function to handle the answer from the window system.",
      PRONOARG },
    { menu_keysymbol,
      "symbol_click",
      "Insert the symbol selected from the symbol window.",
      PRONOARG },
    { menu_notation,
      "template_click",
      "Insert the template selected from the template window.",
      PRONOARG },
    { menu_selected_notation,
      "apply_selected_template",
      "Insert the template selected by the source selection.",
      PRONOARG },
    { ask_save_time,
      "set_save_time",
      "Set the time between two automatic saves.",
      PRONOARG },
    { start_replace,
      "query_replace",
      "Start a query replace function. It will ask for two strings",
      PRONOARG },
    { insert_char,
      "self_insert",
      "Insert the pressed key (N times)",
      PROINTINTARG },
    { next_node_or_insert,
      "next_node_or_insert",
      "",
      PRONOARG },
    { next_node_insert,
      "next_node_insert",
      "Inside text, the first argument is inserted (N times).  Inside "
      "an expression, the selection will move to the next empty place holder "
      "or text position and insert the first argument if possible.",
      PROINTINTARG },
    { make_list_insert,
      "insert_list_element",
      "Inside text, the first argument is inserted (N times). Inside"
      " an expression, the first argument is inserted after the expression"
      " and a new expression is inserted.",
      PROINTINTARG },
    { openparen_insert,
      "open_parenthesis",
      "Inside text the first argument is inserted (N times). Inside"
      " an expression, a pair of parenthesis is added to the selected"
      " expression.",
      PROINTINTARG },
    { closeparen_insert,
      "close_parenthesis",
      "Inside text the first argument is inserted (N times). Inside"
      " an expression, the selection is expanded until is contains"
      " the closing parenthesis. If no closing parenthesis is found"
      " the closing parenthesis is inserted into the text.",
      PROINTINTARG },
    { insert_symbol,
      "insert_symbol",
      "Insert the symbol with the given number (N times).",
      PROINTINTARG },
    { start_find,
      "isearch_forward",
      "Start the incremental forward search. A keyboard map named \"search\""
      " is required.",
      PRONOARG },
    { start_backward_find,
      "isearch_backward",
      "Start the incremental backward search.",
      PRONOARG },
    /* { start_print,
      "start_print",
      "Start the keyboard debug print mode. A keyboard map named \"print\""
      " is required.",
      PRONOARG }, */
    { use_map_string,
      "use_map",
      "Switch to a different keyboard map.",
      PROSTRINGARG },
    { use_temporary_map_string,
      "use_map_temporary",
      "Switch temporary to a different keyboard map.",
      PROSTRINGFUNCFUNCARG },
    { reset_map,
      "reset_map",
      "Return to the default map.",
      PRONOARG },
    /* { print_keywrap,
      "print_key",
      "Print the pressed key in the console window. (For debugging)",
      PRONOARG }, */
    { do_find,
      "search_next",
      "Search for the next occurence. (incremental search)",
      PRONOARG },
    { do_find_backward,
      "search_previous",
      "Search for the previous occurence. (incremental search)",
      PRONOARG },
    { add_char,
      "search_self_insert",
      "Insert the first argument in the search string and search for the next"
      " or previous occurence. (incremental search)",
      PROINTINTARG },
    { add_return,
      "search_newline",
      "In an incremental search session, the search is ended and the"
      " selection is left as it is.  In a query replace session,"
      " either the search value is closed and the replace value is started,"
      " or the search and replace session is started.",
      PRONOARG },
    { add_tab,
      "search_tab",
      "Insert a tab character in the search string.",
      PRONOARG },
    { add_sym,
      "search_symbol",
      "Insert the symbol from the symbol window in the search string.",
      PRONOARG },
    { add_search_notation,
      "search_template",
      "Search for the template from the stencil window.",
      PRONOARG },
    { remove_search_char,
      "search_remove_char",
      "Remove a character from the search string and/or go back to the"
      " previously found occurence",
      PRONOARG },
    { end_search_oldpos,
      "search_cancel",
      "Stop the incremental search and restore the selection to the original"
      " position.",
      PRONOARG },
    { end_search,
      "search_stop",
      "Stop the incremental search and keep the selection as is it.",
      PRONOARG },
    { positive_yn,
      "answer_yes",
      "Replace one match, as answer to the query replace question.",
      PRONOARG },
    { negative_yn,
      "answer_no",
      "Skip to the next match, as answer to the query replace question.",
      PRONOARG },
    { end_search,
      "answer_stop",
      "Stop the query replace.",
      PRONOARG },
    { end_search_oldpos,
      "answer_cancel",
      "Stop the query replace and restore the selection.",
      PRONOARG },
    { replace_all_yn,
      "answer_all",
      "Replace all matches.",
      PRONOARG },
    { edit_open,
      "new_editwindow",
      "Open a new edit window.",
      PRONOARG },
    { notation_open,
      "new_stencilwindow",
      "Open a new stencil window.",
      PRONOARG },
    { symbol_open,
      "new_symbolwindow",
      "Open a new symbol window.",
      PRONOARG },
    { find_open,
      "open_findwindow",
      "Open the find and replace window.",
      PRONOARG },
    { buffer_open,
      "open_bufferwindow",
      "Open the buffer window.",
      PRONOARG },
    { default_open,
      "open_propertieswindow",
      "Open the properties window.",
      PRONOARG },
    { find_close,
      "close_findwindow",
      "Close the find and replace window.",
      PRONOARG },
    { buffer_close,
      "close_bufferwindow",
      "Close the buffer window.",
      PRONOARG },
    { default_close,
      "close_propertieswindow",
      "Close the properties window.",
      PRONOARG },
    { swap_selections,
      "swap_selections",
      "Swap two selections (without swapping the content).",
      PROINTINTARG },
    { unset_select,
      "unset_selection",
      "Remove a selection (without removing the content).",
      PROINTARG },
    { join_selections,
      "join_selections",
      "Join two selections.",
      PROINTINTARG },
    { copy_selections,
      "copy_selections",
      "Make a selection equal to another selection.",
      PROINTINTARG },
    { print_selection_path,
      "selection_path",
      "Construct a relative selection path of the one selection to another.",
      PROINTINTSTRINGREFARG },
    { word_wrap_selection,
      "word_wrap_region",
      "Wrap around words in a region.",
      PROINTARG },
    { swap_selections,
      "swap_selections",
      "Swap two selections (without swapping the content)",
      PROINTINTARG },
    { open_simple_program,
      "start_program",
      "Start a program with the output redirected to an edit window.",
      PROSTRINGSTRINGARG },
    { system_wrap,
      "execute",
      "Execute a system command.",
      PROSTRINGARG },
    { open_message_window,
      "open_message_window",
      "Open an edit window for a catagory of messages.",
      PROSTRINGARG },
    { string_to_window,
      "add_message",
      "Add a message to a given message window.",
      PROSTRINGSTRINGARG },
    { open_helpfile,
      "open_helpfile",
      "Open a given help document at a certain section (using the URL # syntax)",
      PROSTRINGARG },
    { edit_signal_to_proces,
      "send_signal",
      "Send a signal to a running program.",
      PROINTSTRINGARG },
    { edit_string_to_proces,
      "send_string",
      "Send a (formatted) string to a running program.",
      PROSTRINGSTRINGARG },
    { save_project_wrap,
      "save_project",
      "Save the current state in a project file.",
      PROSTRINGARG },
    { menu_close,
      "exit_mathspad",
      "Exit mathspad with notifications for modified documents.",
      PRONOARG },
    { menu_quick_exit,
      "quick_exit_mathspad",
      "Exit mathspad without notifications for modified documents.",
      PRONOARG },
    { menu_open,
      "open_console",
      "Open the console window and start a mathspad session.",
      PROINTINTINTINTINTARG },
    { open_fileselector,
      "open_fileselector",
      "Open the file selector to select a file and call a function.",
      /* comment, startdir, startmask, startfile, callbackfunction */
      PROSTRSTRSTRSTRFUNCARG },
    { open_dirselector,
      "open_dirselector",
      "Open the directory selector to select a directory and call a function.",
      /* comment, startdir, startmask, startfile, callbackfunction */
      PROSTRSTRSTRSTRFUNCARG },
    { (void (*)()) make_id_popup,
      "open_identifier_popup",
      "Open the popup to change the identifier.",
      PRONOARGINTRES},
    { (void (*)()) make_version_popup,
      "open_version_popup",
      "Open the popup to change between versions of a template.",
      PRONOARGINTRES},
    { (void (*)()) check_find,
      "correct_find_term",
      "Check if the place holders are correct for a find action.",
      PRONOARGINTRES},
    { (void (*)()) check_find_replace,
      "correct_replace_term",
      "Check if the place holders are correct for a replace action.",
      PRONOARGINTRES},
    { (void (*)()) find_tree,
      "find_structure",
      "Find an occurence of the structure from the find window.",
      PRONOARGINTRES },
    { replace_tree,
      "replace_structure",
      "Replace the found occurence of the structure from the find window "
      "with the structure from the replace window.",
      PRONOARG },
    { replace_all_tree,
      "replace_all_structure",
      "Replace all occurences of the structure from the find window "
      "with the structure from the replace window.",
      PRONOARG },
    { find_prev_on_stack,
      "find_previous_item",
      "Select the previous find&replace item.",
      PRONOARG },
    { find_next_on_stack,
      "find_next_item",
      "Select the next find&replace item.",
      PRONOARG },
    { find_new_on_stack,
      "find_new_item",
      "Create a new find&replace item.",
      PRONOARG },
    { remove_find_stack,
      "find_clear_items",
      "Find an occurence of the structure from the find and replace window.",
      PRONOARG },
    { find_load_items,
      "find_load_items",
      "Load all the find&replace items from a given file.",
      PROSTRINGARG },
    { find_save_items,
      "find_save_items",
      "Save all the items to the given file.",
      PROSTRINGARG },
    /*
       { open_simple_program,
      "start_program",
      "Start a program with the output redirected to an edit window.",
      PROSTRINGSTRINGARG },
    { save_current_document,
      "save_document",
      "Save the current document.",
      PROSTRINGSTRINGARG },
    { load_current_document,
      "load_document",
      "Load a document in the current edit window.",
      PROSTRINGSTRINGARG },
    { open_document,
      "open_document",
      "Open a document in a new edit window.",
      PROSTRINGARG },
    { output_document,
      "save_as_document",
      "Save the document in the given output format.",
      PROSTRINGSTRINGARG },
    { document_saved,
      "document_saved",
      "Is the given document saved?",
      PROSTRINGARGINTRES },
    { document_name,
      "document_name",
      "The name of the given document",
      PROSTRINGARGSTRRES },
    { document_path,
      "document_path",
      "The path of the given document",
      PROSTRINGARGSTRRES },
    { select_start,
      "selection_start",
      "Start the selection at the given position.",
      PROINTINTINTARG },
    { select_expand,
      "selection_adjust",
      "Adjust the selection to include the given position.",
      PROINTINTINTARG }, */
    { change_attribute,
      "change_font",
      "Change the attribute of a font.",
      PROINTREFSTRSTRARG },
    { process_mpk_file,
      "read_mpk_file",
      "Read an mpk file to update menus, call functions or adjust the keyboard.",
      PROSTRINGARG },
    { symbol_new_page,
      "symbol_new_page",
      "Define a new symbol page, with the given name and size.",
      PROSTRINTINTARG },
    { symbol_selected_symbol,
      "symbol_add_selected",
      "Add the selected symbol to the symbol page with the given name.",
      PROSTRINGARG },
    { symbol_range_page,
      "symbol_range",
      "Add a range of characters to the symbol page with the given name.",
      PROSTRINTINTARG },
    { symbol_empty_page,
      "symbol_skip",
      "Leave a number of postions open the symbol page with the given name.",
      PROSTRINTARG },
    { symbol_goto_page,
      "symbol_page",
      "Goto the symbol page with the given name.",
      PROSTRINGARG },
    { symbol_select_up,
      "symbol_up",
      "Move the symbol selection N rows up.",
      PROINTARG },
    { symbol_select_down,
      "symbol_down",
      "Move the symbol selection N rows down.",
      PROINTARG },
    { symbol_select_left,
      "symbol_left",
      "Move the symbol selection N columns left.",
      PROINTARG },
    { symbol_select_right,
      "symbol_right",
      "Move the symbol selection N columns right.",
      PROINTARG },
    { window_def_keyboard,
      "standard_keyboard",
      "Set the standard keyboard list for a given window type.",
      PROSTRINGSTRINGARG },
    { popup_enable_item,
      "popup_enable_item",
      "Enable in a given popup menu a certain item.",
      PROSTRINGSTRINGARG },
    { popup_disable_item,
      "popup_disable_item",
      "Disable in a given popup menu a certain item.",
      PROSTRINGSTRINGARG },
    { 0, 0, 0, 0 }
    
};
