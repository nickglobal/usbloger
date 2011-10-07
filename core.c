#include <stdio.h>

int main()
{
	int i,jj;
	printf("usb log core \n");
	printf("start count \n");
	jj = 0;
	for ( i=0; i<10; i++)
	{
		printf(" count rqqqqwerewrwe %d \n", i);
		if ( (jj++) == 5) {
		   printf("JJ\n");
		   jj = 123;
		} 	
	}
	printf("count end\n");
	return 0;
}
