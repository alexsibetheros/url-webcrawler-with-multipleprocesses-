#include "queue.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define FALSE 0
//#define NULL 0
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
queue *AddItem (p_queue q_pointer, char* get_url) {//Add item to end of queue
    queue *lp = q_pointer;
    if (q_pointer != NULL) { 
	while (q_pointer -> link != NULL)//Go to end
	    q_pointer = (p_queue)q_pointer->link;
	q_pointer -> link = (struct queue  *) malloc (sizeof (queue));
	q_pointer = (p_queue)q_pointer->link;
	q_pointer -> link = NULL;
	int size=strlen(get_url);
	q_pointer -> url=(char*)malloc((size+1)*sizeof(char));
	strcpy(q_pointer -> url,get_url);
	return lp;
    }
    else { //Create Queue
	q_pointer = ( queue *) malloc (sizeof (queue));
	q_pointer -> link = NULL;
	int size=strlen(get_url);
	q_pointer -> url=(char*)malloc((size+1)*sizeof(char));
	strcpy(q_pointer -> url,get_url);
	return q_pointer;
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
p_queue RemoveItem (p_queue q_pointer) { //Remove Top item
    p_queue tempp;
    tempp = (p_queue)q_pointer -> link;
    free(q_pointer->url);
    free (q_pointer);
    return tempp;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PrintQueue (p_queue q_pointer) { //Debugging tool to print Queue
    	if (q_pointer == NULL)
		printf ("Queue is empty!\n");
    	else{
    		printf("Printing Queue:\n");
		while (q_pointer != NULL) {
	    		printf ("{%s}\n", q_pointer -> url);
	    		q_pointer =(p_queue)q_pointer -> link;
		}
	}
    printf ("\n\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* pop_top(p_queue * q_pointer){ //Returns the item on top of the queue
	if( *q_pointer==NULL){ //No more items
		return NULL;
	}
	int size=strlen((*q_pointer)->url);
	char* temp=(char*)malloc((size+1)*sizeof(char));
	strcpy(temp,(*q_pointer)->url);
	*q_pointer=RemoveItem((*q_pointer)); //Item is removed
	return temp; //Top of queue
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClearQueue (p_queue q_pointer) { //Function that clears list
    while (q_pointer != NULL) {
	q_pointer = RemoveItem (q_pointer);
    }
}


