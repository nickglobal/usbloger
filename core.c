#include <stdio.h>

int main()
{
	int i,j;
	printf("usb log core \n");
	printf("start count");
	j = 0;
	for ( i=0; i<10; i++)
	{
		printf(" count equald %d", i);
		if ( (j++) == 5) printf(" jj is eq 5\n");
	}
	printf("count end\n");
	return 0;
}
