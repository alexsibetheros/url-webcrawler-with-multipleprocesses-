#include <stdio.h>
#include "info.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void print_poll(u_info* pack){//debugging function that prints info package
	printf("\nPrinting URL INFO STRUCT!\n");
	printf("-->Total Url [%d]\n",pack->total_url);
	printf("-->Total Img [%d]\n",pack->total_img);
	printf("-->Total Size [%d]\n",pack->total_size);
	printf("-->Cpu Time [%lf]\n",pack->Ctime);	
	printf("-->Real Time [%lf]\n",pack->Rtime);
	printf("-->URL Type [%s]\n\n",pack->type);
						
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
