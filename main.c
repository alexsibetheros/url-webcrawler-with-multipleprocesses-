#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <err.h>
#include <ctype.h>
#include "core.h"
#include "prime.h"
#include "hash.h"
#include "queue.h"
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <poll.h>
#include "info.h"
# define READ 0
# define WRITE 1
# define BUFSIZE 500

char*  get_next_url(char del,int fd);

void catchinterrupt (void ){
	//function does nothing, but main will use sigsuspend to handel number of signals
	//printf(" Caught Signal\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void loadBar(int x, int n){//Progress Bar function
    	float ratio = x/(float)n;
    	printf("Total Url Sent: %3d%%",(int)(ratio*100) );
	
    	printf("\r");
	fflush(stdout);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void loadBar2(int x, int n){//Progress Bar function
    	float ratio = x/(float)n;
    	printf("Total Url Recieved: %3d%%",(int)(ratio*100) );
	
    	printf("\r");
	fflush(stdout);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////



int main (int argc, char *argv[])
{
	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!------------------START OF MAIN----------------!!!!!!!!!!!!!!!!!!!!!!!\n");
  	char *value=NULL,*input_file=NULL,*input_url=NULL,*output_file=NULL,*message=NULL,*top_temp=NULL,*line=NULL,**url_table,*largest_url=NULL,*tempurl=NULL,ch;
  	char stored[BUFSIZE],stored2[BUFSIZE];
  	int proc_num,url_num,index,c,opt_flag=0,must_flag=0,i,rv,done=0,total=0,size,cancel,tick=0,nread,url_flag=0,largest=0,sizet=0,totalU=0,fdo,x=0;
  	double totalR=0,totalC=0,average_r=0,average_c=0,average;
	u_info pack;
	FILE *s_fd,*t_fd;
  	opterr = 0;
  	pid_t pid,pids;
  	
  	while ((c = getopt (argc, argv, "f:u:p:n:s:")) != -1)//Read inline arguments, all possible cases are dealt with
		switch (c){
	  		case 'f':
				value = optarg;
				if (value==NULL){fprintf(stdout,"Please give a file with -f flag.\n");return 1;}
				if (opt_flag!=0){fprintf(stdout,"Please give only -u or -f\n");return 1;}
				opt_flag=1; must_flag++;
				if ((input_file = malloc(strlen(value) + 1)) == NULL) {perror("malloc");  return 1;}
				strcpy(input_file,value);
				fprintf(stdout, "Input file is: %s \n",input_file);
				break;
 	 		case 'u':
				value = optarg;
				if (value==NULL){fprintf(stdout,"Please give a url with -u flag.\n");return 1;}
				if (opt_flag!=0){fprintf(stdout,"Please give only -u or -f\n");return 1;}
				opt_flag=2; must_flag++;
				if ((input_url = malloc(strlen(value) + 1)) == NULL) {perror("malloc");  return 1;}
				strcpy(input_url,value);
				fprintf(stdout, "Input url is: %s \n",input_url);
				break;
	  		case 'p':
				value = optarg;
				if (value==NULL){fprintf(stdout,"Please give a number of processes with the -p flag.\n");return 1;}
				proc_num=atoi(value);
				fprintf(stdout, "Number of processes is: %d \n",proc_num);
				must_flag++;
				break;
 			case 'n':
				value = optarg;
				if (value==NULL){fprintf(stdout,"Please give a number of urls with the -n flag.\n");return 1;}
				url_num=atoi(value);
				fprintf(stdout, "Number of url's is: %d \n",url_num);
				must_flag++;
				break;
		 	case 's':
				value = optarg;
				if (value==NULL){fprintf(stdout,"Please give a file with the -s flag.\n");return 1;}
				if ((output_file = malloc(strlen(value) + 1)) == NULL) {perror("malloc");  return 1;}
				strcpy(output_file,value);
				fprintf(stdout, "Output file is: %s \n",output_file);
				must_flag++;
				break;
	  		case '?':
				if (isprint (optopt)) {fprintf (stderr, "Unknown option `-%c'.\n", optopt);}
				else {fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);}
				return 1;
	 		default:
		 		 abort ();
	  	}
	for (index = optind; index < argc; index++)
  		printf ("Non-option argument %s\n", argv[index]);	
  	if( must_flag!=4){ printf("Not enough parameters given\n"); return 1;}
	  	
	
	size=get_next_prime(url_num/2); //Calculate the closed(larger) prime number of the number of url's requested divide by 2
	//This creates a medium size hash table, with a few overflows in each cell
	core_point my_core=initiate_core(size,url_num); //Core contains various variables and the hashtable and queue
	url_table=malloc((proc_num)*sizeof(char*)); //table that contains url given to process
	int fd[proc_num][2],fdd[proc_num][2],tempfd[2],table[proc_num];//Url table is used to check if processes are in use or not
	struct pollfd fdt[proc_num];
	pid_t pdtt[proc_num];
	printf("\n\n");	
	
	if(opt_flag==1){ //Depending on if "-f" or "-u" was used, add input into hashtable/queue
		i=0;
		if ( (fdo = open (input_file, O_RDONLY )) == -1 ){perror("open"); exit(-1);}
		do{
			if(tempurl!=NULL){ free(tempurl);}
			tempurl=get_next_url('\n',fdo);
			if(!(strcmp(tempurl,""))){break;}
			if(my_core->max==my_core->total){
			}else{ add_url(my_core,tempurl); }
			i++;	
		}while(strcmp(tempurl,""));  
		if(i==0){printf("Url File is empty, please rerun\n");exit(-1);}  
	}else{
		if(my_core->max==my_core->total){
			}else{ add_url(my_core,input_url); }
	}  
		
	s_fd = fopen(output_file, "w"); if (s_fd == NULL) err(1, "output file");
	fprintf(s_fd, "\n%s\n", "***URL INFORMATION***");
	fprintf(s_fd, "%5s | %9s | %5s | %8s | %8s | %10s |%s\n", "#_URL", "#_Size", "#_Img","CPU_T", "Real_T","Type", "URL");
	fprintf(s_fd, "%5s * %9s * %5s * %8s * %8s * %10s * %s\n", "*****", "*********", "*****","********", "********","**********", "***********");	
	
	t_fd = fopen("tempfile", "w"); if (t_fd == NULL) err(1, "tempfile"); //tempfile will hold all url returned, to add to output file in the end
	
		
	sigset_t  oldmask;
	static struct sigaction act;
	act.sa_handler = (void*)catchinterrupt;
	sigemptyset (&(act.sa_mask));	
	sigaddset (&(act.sa_mask), SIGRTMIN+3);	//special signal that alerts parent that child is ready to accept url's
	sigaction ( SIGRTMIN+3, &act , NULL ) ;
	sigprocmask (SIG_BLOCK, &(act.sa_mask), &oldmask);//block all signals of this type, in order to collect all together when ready(signals are queued)

	sigset_t  oldmask2;
	static struct sigaction act2;
	act2.sa_handler = (void*)catchinterrupt;
	sigemptyset (&(act2.sa_mask));	
	sigaddset (&(act2.sa_mask), SIGRTMIN+2);//signal that alerts parent that child has completed everything	
	sigaction ( SIGRTMIN+2, &act2 , NULL ) ;
	sigprocmask (SIG_BLOCK, &(act2.sa_mask), &oldmask2);//queue this signal for later 
	if (pipe (tempfd) == -1) { perror("pipe "); return(-1); }//Extra pipe that is used to ensure poll wont unfreeze for processes that have finished
								//Processes close the other side of pipe when finished, sto code subsitutes that pipe with this temp one
								//Allowing poll to work will all running processes
	
	for(i=0;i<proc_num;i++){//pipe+fork+exec
		fdt[i].revents=0; //initialize poll structure 
		fdt[i].events=POLLIN  ;
		table[i]=0;
		url_table[i]=NULL;
		if (pipe (fd[i]) == -1) { perror("pipe "); return(-1); }//pipe for parent to write too
		if (pipe (fdd[i]) == -1) { perror("pipe "); return(-1);}//pipe for parent to read from
		if ( (pid = fork ()) == -1 ){ perror("fork ");return(-1); } //create child
		if ( pid != 0 ){ 	
			pdtt[i]=pid; //keep the program id's to send signals too later			
			fdt[i].fd = fdd[i][ READ ]; //poll structure gets read of this pipe
			close (fd[i][READ ]);	
			close (fdd[i][WRITE ]);	
		}
		else { 
			close (fd[i][WRITE ]); //child closes other ends of pipes and exec's
			close(fdd[i][READ]);
			if( (nread=snprintf(stored, BUFSIZE,"%d",fd[i][READ]))<0){ perror("snprintf");printf("SNPRINTF Problem!\n");}
			if( (nread=snprintf(stored2, BUFSIZE,"%d",fdd[i][WRITE]))<0){ perror("snprintf");printf("SNPRINTF Problem!\n");}
			//execl("/usr/bin/valgrind", "valgrind -tool=memcheck --leak-check=full --track-origins=yes --show-reachable=yes" ,"./worker",stored,stored2,(char*)NULL );
			execlp("./worker","worker",stored,stored2,(char*)NULL );
			perror("execlp");return(-1);
		}
	}
	
	i=0;
	printf("Collection starting signals from children...\n");
	while (1){ //Parent freezes untill all children have sent a special signal that they are ready,
			//otherwise, parent may have completed work and sent termination signal before children had chance to catch it		
		sigsuspend (&oldmask);
		i++;
		if(i==proc_num){  break;}	
	}
	sigprocmask (SIG_UNBLOCK, &(act.sa_mask), NULL);//Signals of +3 are now unblocked(since no more will come)

	printf("Giving url and collecting information...\n");
	while(1){//Loop untill all url have been crawled and all processes have returned all information they computed
		if(done==0){
			
			for(i=0;i<proc_num;i++){
				if(table[i]==0){//if current processes is not busy
					
					if((top_temp=pop_top(&(my_core->queue)))!=NULL){//get url from queue
						x++;
						loadBar(x, url_num);//progress bar for number of urls sent to children
						size=strlen(top_temp)+1;//write the size and url to the given child
						if( write (fd[i][WRITE ],&size,sizeof(int) )==-1){perror("write1");return(-1);}
						if(write (fd[i][WRITE ], top_temp, size)==-1){perror("write2");return(-1);}
						table[i]=1;//child is busy
						url_table[i]=(char*)malloc((size)*sizeof(char));
						strcpy(url_table[i],top_temp);//hold url in table for later use, for this process
						free(top_temp);
						total++;
					}else{ url_flag=1;}//No more url in queue, for the moment
					if(total==url_num){ //If total number of url have been given
						printf("\nTotal number of url has been given...\n");
						done=1; //Sets in motion termination of children
						break;
					}
				}
			}
			if(done==1){ printf("Signaling children to finish up...\n"); //If done, signal children to finish
				for(i=0;i<proc_num;i++){
					kill(pdtt[i],SIGRTMIN+1);				
				}	
			}

		}
		if(done==1){
			for(i=0;i<proc_num;i++){
				if(table[i]==0){ //if done, any children that are not in use
						//will have there pipe switched out with an empty pipe
						//because poll normally would unfreeze(because process has closed its pipe)
					fdt[i].fd=tempfd[READ];	
				}
			}
		}
		if((rv = poll(fdt,proc_num, -1)) == -1){ perror("poll"); return(-1);}//Poll all pipes from processes with information to be read
		else  {
            		for(i=0;i<proc_num;i++){ //If poll unfreezes, see which processes sent info
  				
  				if (fdt[i].revents & POLLIN){
  					fdt[i].revents=0;
  					while(1){ //Read all url's from child
  						read (fdt[i].fd, &size , sizeof(int)); //get size of url
						if(size==0){ //if size returned is 0, child has finished parsing
							read (fdt[i].fd, &pack , sizeof(u_info));//package with statistics is sent
							table[i]=0; //child is no longer busy
							fprintf(s_fd, "%5d - %9d - %5d - %8.5lf - %8.5lf - %10s - %s\n", pack.total_url, pack.total_size,pack.total_img,  pack.Ctime, pack.Rtime, pack.type, url_table[i]);//write statistics to file
							if(pack.total_size>largest){//if largest url in pack is larger than current, replace 
								largest=pack.total_size;
								sizet=strlen(url_table[i]);
								if(largest_url!=NULL){ free(largest_url); }
								largest_url=(char*)malloc((sizet+1)*sizeof(char));
								strcpy(largest_url,url_table[i]);
							}
							totalU+=pack.total_url;
							totalR+=pack.Rtime;
							totalC+=pack.Ctime;
							free(url_table[i]);
							url_table[i]=NULL;
							if(done==1){//If no more url to send from parent, change the pipe this child has, 
									//so as poll to not unfreeze because child has closed his pipe
								fdt[i].fd=tempfd[READ];
							}			
							
							break;
						}
        					message=(char*)malloc((size+1)*sizeof(char));
						message[size]='\0';
						read (fdt[i].fd, message , size); //get the message itself
        					if(my_core->max!=my_core->total){add_url(my_core,message);} //add to hash/queue if space available
        					fprintf(t_fd, "%s\n",message);//add message to temp file
						free(message);
						url_flag=0; //More urls added to queue/hash
        				}
        				tick++;	//increase total url crawled(for error checking)			
        				if(done==1){loadBar2(tick,url_num);}
        			}
        		}
    
        	}
        	cancel=0;	
        	for(i=0;i<proc_num;i++){//Check to see if any proc busy
			if(table[i]==1){cancel=1;break;}
		}
		if(cancel==0){//if nobody busy, and no more url's to give and total not reached exit program
		       	if(url_flag==1){  printf("Not enough Url in starting file + url recursively found from original\n"); return(-1);}	
	     	}				
				
 
		if(done==1){ //total url sent
			cancel=0;
			for(i=0;i<proc_num;i++){
				if(table[i]==1){cancel=1;break;}
			}
			if(cancel==0){//if no proc busy, loop can be stopped
				
				break;
			}
		}
	}
	
	i=0;
	printf("\nWaiting for children to return termination signal...\n");
	while (1){ 	//freeze untill all children sent termination signal
		sigsuspend (&oldmask2);
		i++;
		if(i==proc_num){  break;}	
	}
	sigprocmask (SIG_UNBLOCK, &(act2.sa_mask), NULL);
	
	
	average=(double)((double)totalU/url_num);
	average_c=(double)(totalC/url_num);
	average_r=(double)(totalR/url_num);
	
	if(tick!=url_num){printf("TICK: [%d] and Num [%d]\n",tick,url_num); return(-1);}

	for(i=0;i<proc_num;i++){//close pipes	
		close(fd[i][WRITE]);
		close(fdd[i][ READ ]);	
	}	
	printf("Waiting for children to finish any remaining chores...(All information from pipe and signals have been already collected)\n");
	for(i=0;i<proc_num;i++){ //wait for children to exit
  		waitpid(pdtt[i],NULL,0);
 	}
	
	destroy_core( my_core);	
  	fclose(s_fd);
  	free(url_table);		
  	free(input_file);	
	free(input_url);
	free(line);
  		
  	printf("Sorting Information URL alphabetically...\n");	
  	if ( (pids= fork ()) == -1 ){perror("Fork");return(-1);}  
 	if ( pids != 0 ){waitpid(pids,NULL,0); }
 	else{
  		execlp("sort","sort","-o",output_file,"-k13",output_file,(char*)NULL );
  		perror("execlp");return(-1);
 	}	
  	printf("Sorting all retrieved url...\n");
  	if ( (pids= fork ()) == -1 ){perror("Fork");return(-1);}  
 	if ( pids != 0 ){waitpid(pids,NULL,0); }
 	else{
  		execlp("sort","sort","-u","-o","tempfile","tempfile",(char*)NULL );
  		perror("execlp");return(-1);
 	}	
  	
  	
  	
  	
  	fclose(t_fd);
  	printf("Adding to file all URL retrieved during crawl + extra information requested...\n");
  	s_fd = fopen(output_file, "a+"); if (s_fd == NULL) err(1, "output file");
 	t_fd = fopen("tempfile", "r"); if (t_fd == NULL) err(1, "tempfile");
  	fprintf(s_fd, "\n\nTotal Links: [%d]\nLargest Url: [%s] with Size: [%d]\nAverage Url per page: [%lf]\nAverage CPU time: [%lf]\nAverage Real time: [%lf]\n", tick,largest_url,largest,average,average_c,average_r );
  	fprintf(s_fd, "\n\n%s \n\n", "***COMPLETE LIST OF URL CRAWLED***");
    	do{	
		ch=fgetc(t_fd);
		if(ch==EOF){ break;}
		fputc(ch,s_fd);
	}
	while(ch!=EOF);
  	
  	fclose(s_fd);
  	fclose(t_fd);
  	free(output_file);	
  	remove("tempfile");	
	free(largest_url);
  	printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!-------------------END OF MAIN----------------!!!!!!!!!!!!!!!!!!!!!!!\n");		
  return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
char*  get_next_url(char del,int fd){//Function that searchs file for next url
	int n=0,nread;
 	char buff[1],*url;		
 	while ( (nread = read (fd ,buff ,1 )) > 0   ){
 	 	if(nread<0){perror("Read");printf("READ Problem!\n");}
  		if( buff[0] == del  ){n++; break;}  
  		n++;
 	}
 	if(nread==0){return "";}
 	if(lseek(fd,(off_t)-n,SEEK_CUR)<0){ perror("lseek");printf("LSEEK Problem!\n");}
 	if( (url = malloc(sizeof(char)*n) ) == NULL){perror("Malloc");printf("MALLOC Problem!\n");} 
 	if( (nread =  read(fd,url,n-1) ) < 0){perror("Read_2");printf("READ Problem!\n");}
 	url[nread]='\0';
 	if ( read(fd,buff,1) < 0 ){perror("Read_3");printf("READ Problem!\n");}
	return url;                     
}







