#include <stdio.h>

int main()
{
	int i,j;
	printf("usb log core \n");
	printf("start count \n");
	j = 0;
	for ( i=0; i<10; i++)
	{
		printf(" count %d \n", i);
		if ( (j++) == 5) printf("JJ\n");
	}
	printf("count end\n");
	return 0;
}
