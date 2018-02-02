CC = gcc
CFLAGS = -W -Wall
TARGET = Main
OBJECTS = Main.o VirtualMachine.o MultiDataTransfer.o SingleDataTransfer.o BitsHandler.o DataProcessing.o Parse.o Branch.o

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^
clean :
	rm -rf *.o Main

