
MPICC = mpicc
CFLAGS = -O3 -Wall -Wextra -std=c99
LDFLAGS = -lm
TARGET = heat_mpi
COMMON = ../common

SRCS = heat_mpi.c $(COMMON)/grid.c $(COMMON)/utils.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(MPICC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(MPICC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)

run: $(TARGET)
	mpirun -np 4 ./$(TARGET)

.PHONY: all clean run
