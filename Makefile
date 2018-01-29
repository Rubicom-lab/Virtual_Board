CC = gcc
CFLAGS = -W -Wall
TARGET = main
OBJECTS = main.o vm.o MultiDataTransfer.o SingleDataTransfer.o bitsHandler.o DataProcessing.o parse.o Branch.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
clean :
	rm *.o main

