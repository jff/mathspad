/* detect the attributes for a given font. The name of the font is
** available. Other attributes are set to zero/empty/NULL
*/
static void detect_attributes(FONTREC *frec)
{
}

/* load the font in the window system, using the information available
** in FREC (attributes/range/name/encoding). Return FONT_SUCCESS if
** the function succeeded. Return FONT_ERROR if it failed.
*/
static int load_system_font(FONTREC *frec)
{
    return FONT_ERROR;
}

/* return RESULT if information is available on position POS in
** font FREC. Otherwise, return NULL.
*/
static CharInfo *info_system_char(FONTREC *frec, int pos, CharInfo *result)
{
    return NULL;
}

/* Initialize any internal data structures or handles from the information
** passed by DATA
*/
static void font_system_data(void *data)
{
}
