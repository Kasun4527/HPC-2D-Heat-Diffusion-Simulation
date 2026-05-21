# Makefile for Serial Heat Diffusion

CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=199309L
LDFLAGS = -lm

TARGET = heat_serial
COMMON_DIR = ../common
COMMON_OBJS = $(COMMON_DIR)/grid.o $(COMMON_DIR)/utils.o

all: $(TARGET)

$(TARGET): heat_serial.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) heat_serial.o $(COMMON_OBJS) $(LDFLAGS)

heat_serial.o: heat_serial.c
	$(CC) $(CFLAGS) -c heat_serial.c -o heat_serial.o

$(COMMON_DIR)/grid.o: $(COMMON_DIR)/grid.c $(COMMON_DIR)/grid.h
	$(CC) $(CFLAGS) -c $(COMMON_DIR)/grid.c -o $(COMMON_DIR)/grid.o

$(COMMON_DIR)/utils.o: $(COMMON_DIR)/utils.c $(COMMON_DIR)/utils.h
	$(CC) $(CFLAGS) -c $(COMMON_DIR)/utils.c -o $(COMMON_DIR)/utils.o

clean:
	rm -f $(TARGET) heat_serial.o $(COMMON_OBJS)

.PHONY: all clean
