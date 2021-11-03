# simple terrible makefile

CC = gcc
CFLAGS =
LDFLAGS =

OBJECTS = _game.o _parser.o _villains.o _data.o tables.o mt.o compress.o


default: zork1

zork1: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o zork1

_game.o: _game.c _def.h _tables.h
	$(CC) $(CFLAGS) -c _game.c -o $@
_parser.o: _parser.c _def.h _tables.h
	$(CC) $(CFLAGS) -c _parser.c -o $@
_villains.o: _villains.c _def.h _tables.h
	$(CC) $(CFLAGS) -c _villains.c -o $@
_data.o: _data.c
	$(CC) $(CFLAGS) -c _data.c -o $@
tables.o: tables.c
	$(CC) $(CFLAGS) -c tables.c -o $@
mt.o: mt.c
	$(CC) $(CFLAGS) -c mt.c -o $@
compress.o: compress.c
	$(CC) $(CFLAGS) -c compress.c -o $@

_def.h: def.h _data.h
	cp def.h _def.h
_tables.h: tables.h
	cp tables.h _tables.h

_data.c _data.h: compress_data
	./compress_data
compress_data: compress_data.o compress.o
	$(CC) $(LDFLAGS) compress_data.o compress.o -o $@
compress_data.o: compress_data.c
	$(CC) $(CFLAGS) -c compress_data.c -o $@

_game.c: compress_source game.c
	./compress_source game.c _game.c
_parser.c: compress_source parser.c
	./compress_source parser.c _parser.c
_villains.c: compress_source villains.c
	./compress_source villains.c _villains.c
compress_source: compress_source.o compress.o
	$(CC) $(LDFLAGS) compress_source.o compress.o -o $@
compress_source.o: compress_source.c
	$(CC) $(CFLAGS) -c compress_source.c -o $@

clean:
	-rm -f *.o _* compress_data compress_source zork1
