CC = gcc
OBJ = main.o
LIBS = -lpigpio -lrt -lpthread
INCS = 
CFLAGS = -g -Wall
BIN = prog
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS) $(INCS)
