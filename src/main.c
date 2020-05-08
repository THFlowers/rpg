#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>

#include "rpg.h"
#include "sprite.h"
#include "textbox.h"
#include "python.h"

//Uint8 fullscreen = SDL_FALSE;
Uint8 dirty_screen = SDL_TRUE;
map_t map;
point_t camera;
TTF_Font* font;

//void demomap(map_t* map);

int
main(int argc, char** argv)
{
	argc--; //Don't count 0th (our name) element
	if (argc > 3)
	{
		fprintf(stderr, "Too many args! Usage: ./rpg [-s ./default.py] [./demo.map]");
		return 1;
	}

	char* py_name;
	if (argc >= 2 && strcmp(argv[1], "-s") == 0)
	{
		py_name=(char*)malloc(strlen(argv[2]));
		sprintf(py_name, "%s", argv[2]);
	}
	else
	{
		py_name=(char*)malloc(strlen(py_dir)+strlen("default.py")+1);
		sprintf(py_name, "%s%s", py_dir, "default.py");
	}
	printf("%s\n", py_name);

	char* map_name;
	if (argc == 1 || argc == 3)
	{
		map_name=(char*)malloc(strlen(argv[argc]));
		sprintf(map_name, "%s", argv[argc]);
	}
	else
	{
		map_name=(char*)malloc(strlen(map_dir)+strlen("demo.map"));
		sprintf(map_name, "%s%s", map_dir, "demo.map");
	}
	printf("%s\n", map_name);

	FILE* py_file;
	py_file=fopen(py_name, "r");
	if (py_file == NULL)
	{
		fprintf(stderr, "Could not open file: %s\n", py_name);
		return 10;
	}
	
	//SDL_Window* window;
	SDL_Surface* screen;

	sprite_t user;
	sprite_t npc;
	SDL_Event event;

	int key, oldkey;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "Could not initialize video: %s\n", SDL_GetError());
		return 1;
	}
	atexit(SDL_Quit);

	SDL_WM_SetCaption("RPG", "RPG");
	SDL_WM_SetIcon(IMG_Load("media/icon_64.bmp"), NULL);

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_DOUBLEBUF | SDL_ANYFORMAT);
	/*
	window = SDL_CreateWindow("RPG",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		0);
	 */

	if (screen == NULL)
	{
		fprintf(stderr, "Unable to set create window: %s\n", SDL_GetError());
		return 1;
	}
	/*
	SDL_SetWindowIcon(window, IMG_Load("media/icon_64.bmp"));

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL)
	{
		fprintf(stderr, "Unable to create renderer: %s\n", SDL_GetError());
		return 1;
	}
	*/

	//SDL_Surface* screen = SDL_GetWindowSurface(window);
	SDL_SetClipRect(screen, NULL);

	if (loadmap_Long(&map, map_name) != 0)
	{
		fprintf(stderr, "Could not load map %s\n", map_name);
		return 1;
	}

	if (TTF_Init() != 0)
	{
		fprintf(stderr, "Could not initialize SDL_TTF: %s\n", TTF_GetError());
		return 1;
	}
	atexit(TTF_Quit);

	font = TTF_OpenFont("media/VeraSe.ttf", 15);
	if (font == NULL)
	{
		fprintf(stderr, "Could not load font: %s\n", TTF_GetError());
		return 1;
	}

	Py_Initialize();
	if (Py_IsInitialized() == 0)
	{
		fprintf(stderr, "Could not initialize python interpreter!\n");
	}
	atexit(Py_Finalize);

	initrpg();

	PyRun_SimpleFile(py_file, py_name);
	fclose(py_file);

	free(map_name);
	free(py_name);

	if (loadSprite("media/sprites/mnv2.bmp", &user, TILE_SIZE, TILE_SIZE))
		exit(1);

	user.tile_x=2; user.tile_y=2;
	user.animation=RIGHT;
	user.screen_x= TILE_SIZE * user.tile_x;
	user.screen_y= TILE_SIZE * user.tile_y;
	moveSprite(&user, 0, 0);
	user.ai=NULL;

	/* Kept to archive C interface
	npc.tile_x=10; npc.tile_y=10;
	npc.screen_x= TILE_SIZE  * npc.tile_x;
	npc.screen_y= TILE_SIZE * npc.tile_y;
	npc.ai=random_ai;
	npc.aidata[WALK]=19;
	*/

	textbox_t tb;
	char* tb_txt = "Hello 0123456789 0123456789. This is weird how it keeps going. This line should max-out. If not this will certainly do it!";
	newTextBox(&tb, tb_txt, user.screen_x, user.screen_y);

	camera.x=0;
	camera.y=0;

	SDL_EnableKeyRepeat(2, 10);
	dirty_screen = 1;

	int gameover = 0;
	while (gameover == 0)
	{
		while(SDL_PollEvent(&event)) {
			switch(event.type)
			{
				case SDL_QUIT:
					gameover=1;
				case SDL_KEYDOWN:
					key = event.key.keysym.sym;
					switch (key)
					{
						case SDLK_ESCAPE:
							gameover=1;
							break;
						case SDLK_PRINT:
						//case SDLK_PRINTSCREEN:
							SDL_SaveBMP(screen, "screenshots/screenshot.bmp");
							break;
						case SDLK_F11:
							SDL_WM_ToggleFullScreen(screen);
							/*
							if (!fullscreen)
								SDL_SetWindowFullscreen(window, SDL_TRUE);
							else
								SDL_RestoreWindow(window);
							// Needed?  I doubt but who knows
							screen = SDL_GetWindowSurface(window);
							fullscreen = !fullscreen;
							 */
							break;
						case SDLK_RIGHT:
						case SDLK_l:
							dirty_screen = 1;
							if (key == oldkey)
								moveSprite(&user, STEP, 0);
							else
							{
								user.animation=RIGHT;
								user.anim_frame=STEP_RATE - 1;
							}
							break;
						case SDLK_LEFT:
						case SDLK_h:
							dirty_screen = 1;
							if (key == oldkey)
								moveSprite(&user, -STEP, 0);
							else
							{
								user.animation=LEFT;
								user.anim_frame=STEP_RATE - 1;
							}
							break;
						case SDLK_UP:
						case SDLK_k:
							dirty_screen = 1;
							if (key == oldkey)
								moveSprite(&user, 0, -STEP);
							else
							{
								user.animation=UP;
								user.anim_frame=STEP_RATE - 1;
							}
							break;
						case SDLK_DOWN:
						case SDLK_j:
							dirty_screen = 1;
							if (key == oldkey)
								moveSprite(&user, 0, STEP);
							else
							{
								user.animation=DOWN;
								user.anim_frame=STEP_RATE - 1;
							}
							break;
						case SDLK_z:
							//renderTextBox(&tb);
							break;
					}
					oldkey = key;
					break;
			}
		}

		callbackAllSpriteAI(WALK);

		camera.x=user.screen_x + TILE_SIZE/2 - SCREEN_WIDTH/2;
		camera.y=user.screen_y + TILE_SIZE/2 - SCREEN_HEIGHT/2;

		if (dirty_screen)
		{
			drawtiles(screen, map);
			dirty_screen = 0;
		}

		// if a sprite is drawn then next frame needs tile refresh
		if (drawAllSprite(screen) != 0) dirty_screen = 1;
		drawTextBox(screen, &tb);

		SDL_Flip(screen);
		//SDL_RenderPresent(renderer);
		SDL_Delay(50);
	}
	
	freemap(&map);
	freeAllSprite();

	printf("\nExit without major error: cleanup\n\n");
	return 0;
}

// Here in-case generation or testing need to be done
/*
void demomap(map_t* map)
{
	SDL_Surface* temp;

	map->name = "demo";
	SDL_WM_SetCaption("RPG - demo", "RPG");

	map->dim_x = TILES_X;
	map->dim_y = TILES_Y;

	map->num_resources = 1;

	map->resource = (SDL_Surface**)malloc(map->num_resources * sizeof(SDL_Surface*));
	if (map->resource == NULL)
	{
		fprintf(stderr, "Malloc could not allocate %llu bytes for map->resource.\n",
			map->num_resources * sizeof(SDL_Surface*));
		exit(1);
	}

	map->resource_names = (char**)malloc(map->num_resources * sizeof(char*));
	if (map->resource_names == NULL)
	{
		fprintf(stderr, "Malloc could not allocate space for map->resources_names.\n");
		exit(1);
	}

	map->resource_names[0] = "grass.bmp";

	char temp_str[strlen(tiles_dir)+strlen(map->resource_names[0])];
	sprintf(temp_str, "%s%s", tiles_dir, map->resource_names[0]);
	printf("%s\n", temp_str);
	printf("Len:%i\tSize:%i\n", strlen(temp_str), sizeof(temp_str));
	temp = IMG_Load(temp_str);
	if (temp == NULL)
	{
		fprintf(stderr, "Unable to load graphic: %s\n", SDL_GetError());
		exit(1);
	}
	//map->resource[0] = SDL_DisplayFormat(temp);
	map->resource[0] = SDL_ConvertSurfaceFormat(temp, temp->format->format, 0);
	if (map->resource[0] == NULL)
	{
		fprintf(stderr, "Unable to convert graphic: %s\n", SDL_GetError());
		exit(1);
	}

	map->tiles = (tile_t**)malloc(map->dim_x * sizeof(tile_t*));
	if (map->tiles == NULL)
	{
		fprintf(stderr, "Malloc could not allocate %d bytes for map->tiles.\n", map->dim_x * sizeof(tile_t*));
		exit(1);
	}

	int i;
	for (i=0; i < map->dim_x; i++)
	{
		map->tiles[i] = (tile_t*)malloc(map->dim_y * sizeof(tile_t));
		if (map->tiles[i] == NULL)
		{
			fprintf(stderr, "Malloc could not allocate %d bytes for map->tiles.\n", map->dim_x * sizeof(tile_t*));
			exit(1);
		}
	}

	printf("%i bytes allocated for %s\n", sizeof(tile_t) * map->dim_y * map->dim_x, map->name);

	int x,y;
	for (x=0; x < map->dim_x; x++)
	{
		for (y=0; y < map->dim_y; y++)
		{
			map->tiles[x][y].rid=0;
			if (map->resource[ map->tiles[x][y].rid ] == NULL)
			{
				fprintf(stderr, "Unable to load map %s: resource %i not loaded.",
					map->name, map->tiles[x][y].rid);
				exit(1);
			}
			map->tiles[x][y].source= map->resource[ map->tiles[x][y].rid ];

			//map->tiles[x][y].x = rand()%2;
			map->tiles[x][y].u = 0;
			map->tiles[x][y].v = 0;
			map->tiles[x][y].occupied = 0;
			map->tiles[x][y].blocked  = 0;
			if ((x >= map->dim_x/2) && (y==map->dim_y/2))
			{
				map->tiles[x][y].blocked = 0;
				map->tiles[x][y].u=3;
			}
			if (x == 0)
			{
				map->tiles[x][y].blocked = 1;
				map->tiles[x][y].u=4;
			}
			if ((x <= map->dim_x/3) && (y == map->dim_y-3))
			{
				map->tiles[x][y].blocked = 1;
				map->tiles[x][y].u=4;
			}
		}
	}
}
*/
