#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include "worker.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <poll.h>
#include <time.h> 
#include <signal.h>
#include "info.h"
# define READ 0
# define WRITE 1

int fdpipe; //Global pipe used for polling with signals

//**** Because this program will be called from another, program wont exist but will print to ouput any errors found***

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void catchkill(void){//Signal Handler for child, when signal arrives, write to global pipe
	int size=1;	
	if( write (fdpipe,&size,sizeof(int) )==-1){perror("write1");printf("Write Problem!\n");}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main (int argc, char *argv[])
{
	struct pollfd fdt[2];	 
	int fda[2];
	if (pipe(fda) == -1) { perror("pipe "); return(-1); }
	fdt[1].fd=fda[READ]; //Set up poll for the pipe the signal writes too
	fdt[1].revents=0;
	fdt[1].events=POLLIN;
	
	fdpipe=fda[WRITE]; //give a price to the global int, setting up the signal handler

	static struct sigaction act;
	act.sa_handler = (void*)catchkill; //Signals in this group use this function
	sigemptyset (&(act.sa_mask));	
	sigaddset (&(act.sa_mask), SIGRTMIN+1);	//add this specific signal
	sigaction ( SIGRTMIN+1, &act , NULL ) ; 

	
	kill(getppid(),SIGRTMIN+3); //inform parent that process started	
	

	double t1, t2, cpu_time,ticspersec; 
	struct tms tb1, tb2;
		
	u_info info_pack;
  	struct stat st;
  	int size,fd[2],fdd[2],nread,fdo,count,img_count,flag;
	char* filename,* given_url,* stored,* temp=NULL,*copy=NULL,copy2[4],*fixed=NULL,* host,*host2,*type;
	char buff[1],buff2[9];
	if(argc!=3){printf("Not enough or too many inline arguments\n");printf("Parameter PROBLEM\n");}
	fd[READ]=atoi(argv[1]); //retrieve from parent, pipe to read and pipe to write too
	fdd[WRITE]=atoi(argv[2]);
	fdt[0].fd=fd[READ]; //add the read part to polling structor, previously with signal pipe
	fdt[0].revents=0;
	fdt[0].events=POLLIN;
	char givefile[256],getfile[256];
	int rv,size2,fsize;
	FILE *fp;
while(1){ //The big loop, stops when signal has been caught, all data computed and sent
	info_pack.total_url=0;info_pack.total_img=0;info_pack.total_size=0;strcpy(info_pack.type,"None");info_pack.Ctime=0;info_pack.Rtime=0;
	//information is set to 0, in case given url can't be retrieved or some problem occurs
	
	if((rv = poll(fdt,2, -1)) == -1){ //Polling on 2 pipes: read from parent, read from pipe of signal
		if((rv = poll(fdt,2, -1)) == -1){ perror("Poll"); return(-1);} //If signal happens durring poll, poll will exit, returing -1
										// second poll will ensure that all fdt get the correct revent value
	}
	 		
	if (fdt[0].revents & POLLIN){ //If the pipe with data from parent has information in it
		read (fd[ READ ], &size ,sizeof(int)); //read size of upcoming url
		given_url=(char*)malloc((size+1)*sizeof(char));
		given_url[size]='\0';
		read (fd[ READ ], given_url , size); //read the url
		
		ticspersec = (double) sysconf(_SC_CLK_TCK); //start timers
		t1 = (double) times(&tb1);
		
		size2=sizeof(int)+strlen("_o")+2;
		snprintf(givefile, size2,"%d_o",(int)getpid()); //create two temporary, unique files to give to wget
		snprintf(getfile, size2,"%d_e",(int)getpid()); //givefile will store the file wget returns, getfile will store the output of wget
		
		size=strlen(given_url);
		size+=2*size2;
		size+=strlen("wget  -O  1>/dev/null 2> --timeout=10")+2;
		stored=(char*)malloc((size+1)*sizeof(char));	
		stored[size]='\0';
		snprintf(stored, size, "wget \"%s\" -O %s 1>/dev/null 2>%s --timeout=10", given_url,givefile,getfile);
		//wget takes as arguments, the url, a file to store the downloaded file, a file to store the output
		if( (fp=popen(stored,"r") ) ==NULL){ //call popen to run the above command,if null, something went wrong
			info_pack.total_url=0;info_pack.total_img=0;info_pack.total_size=0;strcpy(info_pack.type,"None");info_pack.Ctime=0;info_pack.Rtime=0;
			size=0; //return a package with all 0, showing that current url could not be crawled
			if( write (fdd[WRITE ],&size,sizeof(int) )==-1){perror("write1");printf("Write Problem!\n");}
			if( write (fdd[WRITE ],&info_pack,sizeof(u_info) )==-1){perror("writeT");printf("Write Problem!\n");}
			free(given_url);
			continue; //continue for next url retrieved from parent
		}
		free(stored);
 		pclose(fp);
 		if ( stat(givefile, &st) != 0 ){ //Stat the downloaded file
 			info_pack.total_url=0;info_pack.total_img=0;info_pack.total_size=0;strcpy(info_pack.type,"None");info_pack.Ctime=0;info_pack.Rtime=0;
			size=0; //If downloaded file has any problems, send package with all 0
			if( write (fdd[WRITE ],&size,sizeof(int) )==-1){perror("write1");printf("Write Problem!\n");}
			if( write (fdd[WRITE ],&info_pack,sizeof(u_info) )==-1){perror("writeT");printf("Write Problem!\n");}
			free(given_url);
			continue;
		}
 		fsize=st.st_size; //Get size of file, if zero, wget must have returned 404,403 or something else
 		if( fsize>0){ //file exists, download succesfull
			filename=givefile;
			host=get_host(given_url); //for parsing purposes, get hostname(www.google.com/bob becomes www.google.com)
			host2=get_host2(given_url); //similar to above, but gets last directory( www.google.com/bob/cat will return www.google.com/bob/)
			temp=NULL,copy=NULL,fixed=NULL; 
			count=0;img_count=0;
			if ( (fdo = open (filename, O_RDONLY )) == -1 ){
				remove(filename); //if some problem occurs with opening file, return empty package
				free(given_url);
				free(host);free(host2);
				info_pack.total_url=0;info_pack.total_img=0;info_pack.total_size=0;strcpy(info_pack.type,"None");info_pack.Ctime=0;info_pack.Rtime=0;
				size=0;
				if( write (fdd[WRITE ],&size,sizeof(int) )==-1){perror("write1");printf("Write Problem!\n");}
				if( write (fdd[WRITE ],&info_pack,sizeof(u_info) )==-1){perror("writeT");printf("Write Problem!\n");}
				continue;
			}
			while ( (nread = read (fdo ,buff ,1 )) > 0   ){ //start parsing
 	 			flag=0;
  				if( buff[0] == '<'){ //find this character
  					nread = read (fdo ,buff ,1 );if(nread<0){perror("Read");printf("READ Problem!\n");}
  					if( buff[0] != 'a' && buff[0] != 'i'){continue;} //a for url, i for image
  					if(buff[0]=='a'){//for urls	 				
  						nread = read (fdo ,buff2 ,7 );if(nread<0){perror("Read");printf("READ Problem!\n");}
  						buff2[nread]='\0';
  						if( !strcmp(buff2," href=\"") ){//make sure remaning characters are correct
  							temp=get_string('"',fdo);//FUNCTION THAT GETS THE URL FROM THE FILE
  							size=strlen(temp)+1;
  							strncpy(copy2,temp,4);
  							copy2[4]='\0';
 							if ( (temp[0]=='/') || ( strcmp(copy2,"http") ) ){
 								size=strlen(temp);
 								copy=NULL;
 								copy=temp;
 								if(temp[0]=='/'){//if found, add url to end of host
 									if(host[strlen(host)-1]!='/'){
 										flag=1;
 										size+=strlen(host);
 										fixed=(char*)malloc((size+1)*sizeof(char));
 										strcpy(fixed,host);
 										strcat(fixed, copy);
	 									fixed[size]='\0';
	 									free(temp);
 										temp=fixed;
 									}else{ copy=temp+1;}
 								}else{
 									if(host2[strlen(host2)-1]!='/'){
 										flag=1;
 										size+=strlen(host2)+1;
 										fixed=(char*)malloc((size+1)*sizeof(char));
 										strcpy(fixed,host2);
 										strcat(fixed,"/");
 										strcat(fixed, copy);
 										fixed[size]='\0';
	 									free(temp);
 										temp=fixed;
 									}
 								}
 								if(flag==0){			
 									size+=strlen(host);
 									fixed=(char*)malloc((size+1)*sizeof(char));
 									strcpy(fixed,host);
 									strcat(fixed, copy);
 									fixed[size]='\0';
	 								free(temp);
 									temp=fixed;
 								}
 							}
   							size=strlen(temp)+1;
   							temp[size-1]='\0';
							fix_end(temp);//removes / at the end
							//send url retrieved and parsed to parent
							if( write (fdd[WRITE ],&size,sizeof(int) )==-1){perror("write1");printf("Write Problem!\n");}
							if(write (fdd[WRITE ], temp, size)==-1){perror("write2");printf("Write Problem!\n");}
 	  						count++;
 	  						free(temp);
   						}
   					}else if(buff[0]=='i'){//if image found, increase counter
   						nread = read (fdo ,buff2 ,3 );if(nread<0){perror("Read");printf("READ Problem!\n");}
  						buff2[2]='\0';
 						if( !strcmp(buff2,"mg") ){img_count++;}
 					}
   				} 	
  			}
			free(host);
			free(host2);
  			info_pack.total_url=count;	
  			info_pack.total_img=img_count;
			
    			if (fstat(fdo, &st) == 0) {
    			}else{ perror("STAT"); printf("\n\nFILENAME=[%s]\n\n",filename);}
    			info_pack.total_size=(int)st.st_size;
 
			type=get_type(getfile);//searches through the output of wget for specific key words that show the type of file
			strcpy(info_pack.type,type);
			free(type);
			t2 = (double) times(&tb2);//stop the timer
			cpu_time = (double) ((tb2.tms_utime + tb2.tms_stime) -(tb1.tms_utime + tb1.tms_stime));	
			info_pack.Rtime=(t2 - t1)/ticspersec;
			info_pack.Ctime=cpu_time/ticspersec;
			close(fdo);
			//remove(filename);
			remove(givefile); remove(getfile);	
		}else{ remove(givefile); remove(getfile);} //if file does not exist, remove kai return empty package
		
		size=0; //Write package to parent
		if( write (fdd[WRITE ],&size,sizeof(int) )==-1){printf("WRITE PROBLEM\n");}
		if( write (fdd[WRITE ],&info_pack,sizeof(u_info) )==-1){perror("writeT");printf("WRITE PROBLEM\n");}
		free(given_url);
	}
	if (fdt[1].revents & POLLIN)
	{
		read (fdt[1].fd, &size ,sizeof(int));//read an int from pipe to clear pipe	
		kill(getppid(),SIGRTMIN+2);//Send termination signal to parent
		close(fd[READ]);
		close(fdd[WRITE]);
		//printf("END OF PROCESS [%d]\n",getpid());
			
		break;	
	}
}
	
	
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* get_host(char* url){//get hostname from url
	char* host=NULL,*temp;
	int size=strlen(url);
	host=(char*)malloc((size+1)*sizeof(char));
	host[size]='\0';
	strcpy(host,url);
	temp=host+7;
	char* pch=strchr(temp,'/');
	if(!(pch==NULL))
		*pch='\0';
	return host;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* get_host2(char* url){//get last directory from url
	char* host=NULL;
	int size=strlen(url);
	host=(char*)malloc((size+1)*sizeof(char));
	host[size]='\0';
	strcpy(host,url);
	char* pch,*a;
	pch=host;
	pch=host+7;
	size=strlen(pch);
	a=pch+size;
	while( !( (pch=strchr(pch,'/')) == NULL) ){ 
		pch=pch+1;
		a=pch;	
	}
	*a='\0';
	return host;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* fix_end(char* url){//remove ending / if exists
	int size=strlen(url);
	if( url[size-1]=='/')
		url[size-1]='\0';
	return url;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
char* get_type(char* filename){//get type of file from output of wget
	int fd,nread,n=0;
	char* url;
	char buff[1],buff2[10];
	if ( (fd = open (filename, O_RDONLY )) == -1 ){perror("open"); exit(-1);}
	while ( (nread = read (fd ,buff ,1 )) > 0   ){
 	 if(nread<0){perror("Read");printf("READ Problem!\n");}
  	if( buff[0] == 'L'  ){//search for the word: Length in file
  		nread = read (fd ,buff2 ,5 );
  		buff2[5]='\0';
  		if(!strcmp(buff2,"ength")){
  			n=0;
  			while ( (nread = read (fd ,buff ,1 )) > 0   ){
  				if(buff[0]=='['){//Save string inbetween [ and ], this is the type
  						//Because length uknown, read entire string once(char by char), 
  						//then lseek and read all together 
  					while ( (nread = read (fd ,buff ,1 )) > 0   ){
 	 					if(nread<0){perror("Read");printf("READ Problem!\n");}
  						if( buff[0] == ']'  ){n++; break;}  
  						n++;
 					}
 					if(lseek(fd,(off_t)-n,SEEK_CUR)<0){ perror("lseek");printf("LSEEK Problem!\n");}
 					if( (url = malloc(sizeof(char)*n) ) == NULL){perror("Malloc");printf("MALLOC Problem!\n");} 
 					if( (nread =  read(fd,url,n-1) ) < 0){perror("Read_2");printf("READ Problem!\n");}
 					url[nread]='\0';
 					return url;
  				}
  			}	
  		}
  		
 	}
 	}
 	if( (url = malloc(sizeof(char)*strlen("Unknown")) ) == NULL){perror("Malloc");printf("MALLOC Problem!\n");} 
 	strcpy(url,"Unknown");
 	return url;


}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

char*  get_string(char del,int fd){ //Function that takes a specific char and a file descriptor(in a specific position) 
				//and returns a string from the fd until the first occurance of the del
	int n=0,nread;
 	char buff[1],*url;
 	//Save string inbetween start and del, this is the type
  	//Because length uknown, read entire string once(char by char), 
  	//then lseek and read all together 		
 	while ( (nread = read (fd ,buff ,1 )) > 0   ){
 	 	if(nread<0){perror("Read");printf("READ Problem!\n");}
  		if( buff[0] == del  ){n++; break;}  
  		n++;
 	}
 	if(lseek(fd,(off_t)-n,SEEK_CUR)<0){ perror("lseek");printf("LSEEK Problem!\n");}
 	if( (url = malloc(sizeof(char)*n) ) == NULL){perror("Malloc");printf("MALLOC Problem!\n");} 
 	if( (nread =  read(fd,url,n-1) ) < 0){perror("Read_2");printf("READ Problem!\n");}
 	url[nread]='\0';
 	if ( read(fd,buff,1) < 0 ){perror("Read_3");printf("READ Problem!\n");}
	return url;                     
}












