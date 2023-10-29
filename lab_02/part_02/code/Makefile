CC := gcc
CFLAGS := -std=c99 -Wall -Werror
TARGETS := out/main_01 out/main_02 out/main_03 out/main_04_01 out/main_04_02 out/main_05

all : $(TARGETS)

out/% : %.c
	$(CC) $(CFLAGS) $^ -o $@
