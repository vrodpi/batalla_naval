CC = gcc
CFLAGS = -std=c11 -Wall -Wextra
OBJS = obj/main.o obj/game.o obj/tablero.o obj/ui_cli.o

bin/demo: $(OBJS)
	mkdir -p bin
	$(CC) $(CFLAGS) $(OBJS) -o bin/demo

obj/main.o: src/main.c include/game.h include/tablero.h include/ui_cli.h
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

clean:
	rm -rf obj bin

.PHONY: clean