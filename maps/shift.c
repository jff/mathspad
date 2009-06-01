#include <stdio.h>

int main(int argc, char **argv)
{
    char buffer[80];
    int i,j;
    while (fgets(buffer, 79,stdin)) {
	if (buffer[2]<'2') buffer[2]='8'-'0'+buffer[2];
	else buffer[2]='A'-'2'+buffer[2];
	if (buffer[4]!=' ') {
	    if (buffer[4]<'2') buffer[4]='8'-'0'+buffer[4];
	    else buffer[4]='A'-'2'+buffer[4];
	}
	fputs(buffer, stdout);
    }
}
