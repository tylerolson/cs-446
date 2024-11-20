# Compiler
CC = gcc -pthread

# Compiler flags
CFLAGS = -Wall -Werror -g

# Source files
SRCS = sched.c

# Object files
OBJS = $(SRCS:.c=.o)

# Output executable
EXEC = sched

# Default target
all: $(EXEC)

# Link object files to create executable
$(EXEC): $(OBJS)
	$(CC) $(OBJS) -o $(EXEC)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executable
clean:
	rm -f $(OBJS) $(EXEC)

# Optional: target to run the program
run: $(EXEC)
	./$(EXEC)
