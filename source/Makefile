CC = gcc
OBJ = main.o encoder.o indexAlignment.o configure.o motorCtrl.o gcodeParser.o varsFile.o dirFiles.o
LIBS = -lpigpio -lrt -lpthread -lncurses -lm
INCS =
CFLAGS = -g -Wall
BIN = qclcnc/qclcnc
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS) $(INCS)
