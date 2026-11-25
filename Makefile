CC = gcc
CFLAGS = -std=c11 -Wall -Wextra
OBJS = obj/main.o obj/game.o obj/tablero.o obj/ui_cli.o obj/ranking.o obj/ai.o

bin/batalla_naval.exe: $(OBJS)
	mkdir -p bin
	$(CC) $(CFLAGS) $(OBJS) -o bin/batalla_naval.exe

obj/main.o: src/main.c include/game.h include/tablero.h include/ui_cli.h include/ranking.h include/ai.h include/config.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

obj/game.o: src/game.c include/game.h include/tablero.h include/config.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/game.c -o obj/game.o

obj/ui_cli.o: src/ui_cli.c include/ui_cli.h include/game.h include/config.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/ui_cli.c -o obj/ui_cli.o

obj/tablero.o: src/tablero.c include/tablero.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/tablero.c -o obj/tablero.o

obj/ranking.o: src/ranking.c include/ranking.h include/config.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/ranking.c -o obj/ranking.o

obj/ai.o: src/ai.c include/ai.h include/game.h
	mkdir -p obj
	$(CC) $(CFLAGS) -c src/ai.c -o obj/ai.o

clean:
	rm -rf obj bin

.PHONY: clean test