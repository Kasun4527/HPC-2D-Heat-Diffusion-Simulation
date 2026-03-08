# Makefile for OpenMP Heat Diffusion

CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c99 -fopenmp
LDFLAGS = -lm -fopenmp
TARGET = heat_openmp
COMMON = ../common

SRCS = heat_openmp.c $(COMMON)/grid.c $(COMMON)/utils.c
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
