#include <errno.h>

extern int sys_nerr;
extern char *sys_errlist[];
extern int errno;

char *strerror(int num)
{
    if (num>=0 && num<=sys_nerr) return sys_errlist[num];
    return 0;
}
