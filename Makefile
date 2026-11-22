CC = gcc
CFLAGS = -std=c11 -Wall -Wextra
OBJS = obj/main.o obj/game.o obj/tablero.o obj/ui_cli.o obj/ai.o
TEST_OBJS = obj/test_ai.o obj/ai.o obj/game.o obj/tablero.o

bin/demo: $(OBJS)
	mkdir -p bin
	$(CC) $(CFLAGS) $(OBJS) -o bin/demo

obj/main.o: src/main.c include/game.h include/tablero.h include/ui_cli.h include/ai.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

obj/game.o: src/game.c include/game.h include/tablero.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/game.c -o obj/game.o

obj/ui_cli.o: src/ui_cli.c include/ui_cli.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/ui_cli.c -o obj/ui_cli.o

obj/tablero.o: src/tablero.c include/tablero.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/tablero.c -o obj/tablero.o

obj/ai.o: src/ai.c include/ai.h include/game.h include/tablero.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/ai.c -o obj/ai.o

bin/test_ai: $(TEST_OBJS)
	mkdir -p bin
	$(CC) $(CFLAGS) $(TEST_OBJS) -o bin/test_ai

obj/test_ai.o: tests/test_ai.c include/ai.h include/game.h include/tablero.h
	mkdir -p obj
	$(CC) $(CFLAGS) -Iinclude -c tests/test_ai.c -o obj/test_ai.o

test: bin/test_ai
	./bin/test_ai

clean:
	rm -rf obj bin

.PHONY: clean test