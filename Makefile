# CENTER_POSITION - Next figure will fall by center of the window.
# 					Without it it will fall the same position
# 					where was cursor last time.
#
# ONE_STEP -		Run only once and exit.
#
# DEBUG -			Debbuging code.

TG = tetris
ERR_LOG = error.log

#OPTFLAGS = -DCENTER_POSITION -g -DDEBUG #-DONE_STEP
CFLAGS = -Wall -Wextra $(OPTFLAGS)

LIBS = -lncurses -pthread

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c,%.o,$(SRC))

all:$(TG)

$(TG):$(OBJ)
	$(CC) -o $(TG) $(OBJ) $(LIBS)

$@:$<
	$(CC) -o $@ -c $(CFLAGS) $<

clean:
	@if [ -a $(OBJ) ] ;		\
	then 					\
	    rm $(OBJ); 			\
	fi; 					\
	if [ -a $(TG) ] ; 		\
	then 					\
	    rm $(TG); 			\
	fi; 					\
	if [ -a $(ERR_LOG) ] ; 	\
	then 					\
	    rm $(ERR_LOG); 		\
	fi; 

debug: OPTFLAGS += -DCENTER_POSITION -g -DDEBUG #-DONE_STEP
debug: clean all
	touch $(ERR_LOG)
	valgrind --leak-check=full --show-reachable=yes --track-origins=yes ./$(TG) 2> $(ERR_LOG)
	cat $(ERR_LOG)
	
debug_calls: clean all
	valgrind --tool=callgrind ./$(TG)
