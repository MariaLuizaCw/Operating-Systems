CC = gcc
CFLAGS = -Wall -pthread
TARGET = barbershop

all: $(TARGET)

$(TARGET): barbershop.c
	$(CC) $(CFLAGS) -o $(TARGET) barbershop.c

clean:
	rm -f $(TARGET)
