#include <stdio.h>

int main()
{
	// comment pushed via ssh
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
	if ( jj == 123 ) 
	{
		printf("remote branch added fix 1  \n");	
		printf("remote branch added fix 2  \n");
	
		printf("remote branch added fix 3  \n");	
		printf("remote branch added fix 4  \n");
		printf("commit once again \n");
		printf("count end\n");
	}
#ifdef USB_FEATURE
	printf("USB logger [ON]\n");
#endif
	return 0;
}
