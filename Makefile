CC := gcc
CC_FLAGS := -Wall -Werror -std=c11 -O0
OUT := intrin_test

$(OUT): intrin_test.c
	$(CC) -o $@ $^ $(CC_FLAGS)

run: $(OUT)
	./$(OUT)
