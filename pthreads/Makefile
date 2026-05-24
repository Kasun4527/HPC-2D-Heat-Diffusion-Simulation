# makefile - posix threads heat diffusion

CC = gcc
CFLAGS = -O3 -Wall -std=c99 -D_POSIX_C_SOURCE=200112L -pthread
LDFLAGS = -lm -pthread
TARGET = heat_pthreads
COMMON = ../common

SRCS = heat_pthreads.c $(COMMON)/grid.c $(COMMON)/utils.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run