# rm and create build directory and then compile an executable from src_main/prog.c

# Variables

# no optimization, debugging info, all warnings

CC = gcc
CFLAGS = -Wall -Wextra -no-pie
SRC = src_main/prog.c
OBJ = $(SRC:.c=.o)
EXEC = prog_to_run
BUILD_DIR = build

# Rules
all: $(EXEC)

$(EXEC): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -rf $(BUILD_DIR)

run:
	./$(BUILD_DIR)/$(EXEC)