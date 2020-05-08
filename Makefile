cc=gcc
warnings= -ggdb -Wall -pedantic -Wextra -Waddress -Wimplicit -Wnested-externs
sdl_libs=`sdl-config --libs` -lSDL_image -lSDL_ttf
gtk_libs=`pkg-config --cflags --libs gtk+-2.0`
python_libs=`pkg-config --libs python2`

.Phony: all clean
all: rpg edit
rpg: src/main.o src/rpg.o src/sprite.o src/python.o src/textbox.o
	$(cc) -o rpg ./src/main.o ./src/rpg.o ./src/sprite.o ./src/textbox.o ./src/python.o\
		$(sdl_libs) $(python_libs) $(warnings)
edit: src/editor.c src/rpg.o
	$(cc) -o edit ./src/editor.c ./src/rpg.o $(sdl_libs) $(gtk_libs) $(warnings)
src/main.o: src/main.c
	$(cc) -c -o src/main.o src/main.c `sdl-config --cflags` $(warnings)
src/rpg.o: src/rpg.c src/rpg.h
	$(cc) -c -o src/rpg.o src/rpg.c `sdl-config --cflags` $(warnings)
src/sprite.o: src/sprite.c src/sprite.h
	$(cc) -c -o src/sprite.o src/sprite.c `sdl-config --cflags` $(warnings)
src/textbox.o: src/textbox.c src/textbox.h
	$(cc) -c -o src/textbox.o src/textbox.c `sdl-config --cflags` $(warnings)
src/python.o: src/python.c src/python.h
	$(cc) -c -o src/python.o src/sprite.o src/python.c $(warnings)
clean:
	rm rpg edit src/*.o
remake: clean all
