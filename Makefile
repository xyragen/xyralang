CC      := cc
CFLAGS  := -std=gnu11 -O2 -Wall -Wextra -Wpedantic
BUILD   := build
SRC     := src

.PHONY: all clean

all: $(BUILD)/xyra

$(BUILD)/xyra: $(BUILD)/main.o $(BUILD)/lexer.o $(BUILD)/ast.o
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD)/%.o: $(SRC)/%.c
	mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -I$(SRC) -c $< -o $@

clean:
	rm -rf $(BUILD)