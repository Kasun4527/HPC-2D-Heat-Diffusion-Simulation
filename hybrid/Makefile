# Makefile for Hybrid (MPI + OpenMP) Heat Diffusion

MPICC = mpicc
CFLAGS = -O3 -Wall -Wextra -std=c99 -fopenmp
LDFLAGS = -lm -fopenmp
TARGET = heat_hybrid
COMMON = ../common

SRCS = heat_hybrid.c $(COMMON)/grid.c $(COMMON)/utils.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(MPICC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(MPICC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	mpirun -np 2 ./$(TARGET)

.PHONY: all clean run
