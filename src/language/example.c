#include <stdio.h>

void gcd(int *r, int a, int b)
{
    while (1) {
	if (a>b) a=a-(b/a-1)*b;
	else if (b>a) b=b-(a/b-1)*a;
	else break;
	printf("a\t%i\tb\t%i\n", a,b);
    }
    *r=a;
}

void fac(int *r, int a)
{
    *r=1;
    printf("a\t%i\tr\t%i\n", a, *r);
    while (a>0) {
	*r=*r*a;
	a=a-1;
	printf("a\t%i\tr\t%i\n", a, *r);
    }
}


int main(int argc, char **argv)
{
    int y=6;
    fac(&y,y);
    return 0;
}
