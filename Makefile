##############################################################################
## UDPTOYS Makefile ##########################################################
##############################################################################

WARNINGS = -Wall -W -Wno-unused-parameter 

# CC = gcc $(WARNINGS) -g
CC = gcc $(WARNINGS) -g -O3
# CC = gcc $(WARNINGS) -g -fprofile-arcs -ftest-coverage
# CC = gcc $(WARNINGS) -g
# CC = gcc $(WARNINGS) -g -DNDEBUG
# CC = g++ $(WARNINGS) -g -O3

ifdef COMPILER
	CC := $(COMPILER)
endif

## Definitions ###############################################################

EXE = \
	udpsend \
	udpreceive \
	udpreflect \
	udpexpect 

## Target ####################################################################
default: $(EXE) 

## Core Library ##############################################################

$(EXE):%: $(ROOTDIR)%.c 
	$(CC) -o $@ $< 

## Cleanup ###################################################################
clean:
	rm -f core
	rm -f $(OBJ) $(EXE)
	rm -f *.da *.bb *.bbg *.gcov

