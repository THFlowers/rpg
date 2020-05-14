### WARNING: OLD CODE, NOT REPRESENTATIVE OF CURRENT SKILLS

The project was started while I was still in high school and had no idea what I was doing.  I had no formal training and was slapping this together for fun.

This was abandoned due to issues loading maps (incompatible broken branches and endianess issues), and inability to figure out how context menus could be implemented (ie to talk or save the game)

See "Why is this here?" to see why I'm releasing this code. 


## RPG

An rpg like graphical demo written between June 2010 and December 2010.  My first non toy project.  Based on examples from "Programming Linux Games" from No Starch Press, and with a little input from (the now defunct) gpwiki.org (Game Programming Wiki)

It consists of a sprite that can walk around a tile based map. Like old 2d rpg games, the world scrolls around you, attempting to keep the player in the center of the screen.

Text boxes can be displayed over the map.  But there is no interactivity.  The default map and script starts with the user's sprite under the 

npc's can be created and controlled through a provided AI or python code (never finished, only random walks supported).

Scene can be scripted (to the limited state completed) with an embedded python interpreter.

### Video of the project
Incomplete handling of edge tiles: https://www.youtube.com/watch?v=XV7Nrsjl-MA

Playing around (excuse the music): https://www.youtube.com/watch?v=_o5eiAIVpHo

### Libraries Required
* Python 2.7
* SDL 1.2
* SDL_ttf
* SDL_image
* Gtk2 (editor only)

### Build

Use the provided Makefile,

  > make all

CMakeLists.txt is for CLion compatibility, mostly so the correct headers are located.

Attempts to build with Msys2 and CLion were a failure. WSL still has issues building and running with CLion, but command line invocation of the makefile works perfectly fine if an XServer is installed.

### Run

  > ./rpg [-s ./default.py ./demo.map]

Loads script "default.py" over map "demo.map".

Must have exactly these two parameters or none, in which case the above literals are used.

  > ./edit

Starts the map based editor.

GUI should be mostly self explanatory.  Open or create a map, select tile set, then start painting tiles to create your desired level design.


### Coding Style

Coding style used here is erratic and imitates unix instead of using normal conventions.  For example, most functions return an error code of 0 for success and non-zero for failure.  Also, function naming scheme is not stable.

### Accomplishments

* Moderate level of competency with C programming
    * Was still new to the language and self taught
* Displays knowledge of GNU Make, esp. in a Linux environment (pkg-config)
* Uses SDL for a game like program
* Uses uint8, atexit() and other moderately advanced features of C
* Dynamic memory management of most data
* Attempts to use binary data files
* Multi-threaded editor uses SDL in one thread and GTK2 in another
* Embedded python interpreter and custom extension module

### Errors/Mistakes
There are numerous errors, like reversing row/col precedence in 2d arrays.  Storing binary data without worrying about endianness, and seeking when un-needed (sparse files).

Command line parsing has no error handling what-so-ever.

There are at least two abandoned branches, both with incompatible map handling.  Both are sensitive to endianess, so files are CPU dependent and sparse.  Also, tile set names were stored in \<string size\>\<string\> format.  Instead of storing a null byte, or using plain text for the entire file.

OOP was not used at all and the structure used here should really be refactored to be more OOP like.

Global variables were used.  Both global between compilation units, and within compilation units (basically, singleton pattern, though I had never heard the term at that point)

There was an idea to separate game scripts and maps, so that a scene can be reused. But both have numerous issues, including no checks in the script to see if the map is compatible with the desired code (ie placing textbox outside of bounds)

Game timing is based on frame rate, and animation is based on SDL's key repeat rate.  There is no concept of delaying to force a framerate, dropping frames, or of animation tics.

Likewise motion is totally free, not square oriented.  Instead of picking a direction then an animation playing to move to the next square (ala 2d Pokemon).

## Why is this here?

Despite all the issues pointed out above, I am still proud of this code.  It was my first "larger" project and I had a lot of fun (and frustration) creating it.

I have held on to it for 10 years, moving it from machine to machine.  From hard drive, to flash drive, to google drive, just archiving it.  This should no longer be hidden, and maybe resurrected or reattempted.

It shows my skills at the time, both the ugly failures and the good use of available libraries.  It's also proof of how much experience I have with C/C++, that I was at this level in 2010.

It may be useful to work on it over this Summer to clean it up, redo the incorrect parts, and thereby show off my current skills.

Also, it has inspired, and may be the perfect code base to test, a new research idea: Generating IntelliJ code inspection patterns to guide api version migration.  Since this project uses 3 outdated libraries it can be used not only to test the patterns generated from those libraries, but also how multiple libraries interfere with each other.
