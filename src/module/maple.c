#ifdef MAPLE
static void popup_maple(void *data, int n);
    {"Maple",8,1,NULL,popup_misc},
    {"Start Maple", 1,0,NULL,popup_maple},
    {"Evaluate", 2,0,NULL,popup_maple},
    {"Simplify", 3,0,NULL,popup_maple},
    {"Normalize", 4,0,NULL,popup_maple},
    {"Raw Command", 5,0,NULL,popup_maple},
    {"Plot", 8,0,NULL,popup_maple},
    {NULL, 0, 0, NULL, NULL},

    case 8: open_program("maplescript %i &", "Maple Shell", NULL); break;

static void popup_maple(void *data __attribute__((unused)), int n)
{
    switch (n) {
    case 1: open_program("maplescript %i &", "Maple Shell",NULL); break;
    case 2: edit_string_to_proces("eval(%1);\n", "Maple Shell"); break;
    case 3: edit_string_to_proces("simplify(%1);\n", "Maple Shell"); break;
    case 4: edit_string_to_proces("normal(%1);\n", "Maple Shell"); break;
    case 5: edit_string_to_proces("%1;\n", "Maple Shell"); break;
    case 8: edit_string_to_proces("plotsetup(gif);\nplot(%1,%2);\n!mapleshow plot.gif ;\n", "Maple Shell"); break;
    default: break;
    }
}

#endif
