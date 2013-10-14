OBJS 	= main.o core.o hash.o list.o queue.o prime.o  worker.o info.o
OBJS1 	= main.o core.o hash.o list.o queue.o prime.o info.o
OBJS2 	= worker.o info.o
SOURCE	= main.c core.c hash.c list.c queue.c prime.c  worker.c info.c
HEADER  = core.h queue.h list.h hash.h prime.h p worker.h info.h
OUT1  	= mycrawler
OUT2 	= worker
CC	= gcc
#FLAGS   = -c -g  -Wall -Wextra -ggdb -pg -DDEBUG
LIBS	= -lm
# -c flag generates object code for separate files

ifdef DEBUG
	FLAGS   = -c -g  -Wall -Wextra -ggdb -pg -DDEBUG
else
	FLAGS= -c
endif

all: $(OBJS)
	$(CC) -g $(OBJS1) -o $(OUT1) $(LIBS)
	$(CC) -g $(OBJS2) -o $(OUT2)  
	

# create/compile the individual files >>separately<< 
worker.o: worker.c
	$(CC) $(FLAGS) worker.c	
	
core.o : core.c
	$(CC) $(FLAGS) core.c
	
main.o: main.c
	$(CC) $(FLAGS) main.c

list.o: list.c
	$(CC) $(FLAGS) list.c

hash.o: hash.c
	$(CC) $(FLAGS) hash.c

prime.o: prime.c
	$(CC) $(FLAGS) prime.c 

queue.o: queue.c
	$(CC) $(FLAGS) queue.c
	
info.o: info.c
	$(CC) $(FLAGS) info.c



debug:
	make all DEBUG=1	

# clean house
clean:
	rm -f $(OBJS) $(OUT1) $(OUT2) 


