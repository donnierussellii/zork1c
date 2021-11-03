# simple terrible makefile

CC = gcc
CFLAGS =
LDFLAGS =

OBJECTS =       _game.o _data.o _parser.o _villain.o tables.o mt.o compress.o


default: zork1

zork1: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o zork1

_game.o: _data.c _game.c _def.h _tables.h
	$(CC) $(CFLAGS) -c _game.c -o $@
_data.o: _data.c _game.c _def.h _tables.h
	$(CC) $(CFLAGS) -c _data.c -o $@
_parser.o: _data.c _game.c _def.h _tables.h
	$(CC) $(CFLAGS) -c _parser.c -o $@
_villain.o: _data.c _game.c _def.h _tables.h
	$(CC) $(CFLAGS) -c _villain.c -o $@

tables.o: tables.c
	$(CC) $(CFLAGS) -c tables.c -o $@
mt.o: mt.c
	$(CC) $(CFLAGS) -c mt.c -o $@

_data.c: compress_data
	./compress_data
_game.c: compress_source
	./compress_source

_def.h: def.h
	cp def.h _def.h

_tables.h: tables.h
	cp tables.h _tables.h

compress_data: compress_data.o compress.o
	$(CC) $(LDFLAGS) compress_data.o compress.o -o $@
compress_source: compress_source.o compress.o
	$(CC) $(LDFLAGS) compress_source.o compress.o -o $@

compress_data.o: compress_data.c
	$(CC) $(CFLAGS) -c compress_data.c -o $@
compress_source.o: compress_source.c
	$(CC) $(CFLAGS) -c compress_source.c -o $@
compress.o: compress.c
	$(CC) $(CFLAGS) -c compress.c -o $@

clean:
	-rm -f *.o compress_data compress_source _* zork1
