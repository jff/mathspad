#ifndef MP_MODULE_PVS_H
#define MP_MODULE_PVS_H

extern void pvs_add_keyword(Char *keyword, Char *term, int induct);

extern void pvs_check_hint(int selection);

extern int pvs_parse(unsigned char *buff, unsigned int *len);

extern void pvs_start(Char *title);

#endif
