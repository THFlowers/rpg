#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "rpg.h"

map_t map;
point_t camera;

/* Map */
int savemap(map_t* map, char* filename)
{
	FILE* fp;
	int i;
	int x, y;

	fp = fopen(filename, "wb");
	if (fp == NULL)
	{
		fprintf(stderr, "Map save failure: Unable to open file for writing\n");
		return 1;
	}

	fwrite((void*)&map->dim_x, sizeof(Uint16), 1, fp);
	fwrite((void*)&map->dim_y, sizeof(Uint16), 1, fp);
	fwrite((void*)&map->num_resources, sizeof(Uint16), 1, fp);

	for (i=0; i<map->num_resources; i++)
	{
		int size = strlen(map->resource_names[i]);
		printf("Resource[%i]: %s\n", i, map->resource_names[i]);
		fwrite((void*)&size, sizeof(int), 1, fp);
		fwrite(map->resource_names[i], sizeof(char), size, fp);
	}

	for (x=0; x < map->dim_x; x++)
	{
		for (y=0; y < map->dim_y; y++)
		{
			fwrite((void*)&map->tiles[x][y].rid, sizeof(Uint8), 1, fp);
			fwrite((void*)&map->tiles[x][y].u, sizeof(Uint16), 1, fp);
			fwrite((void*)&map->tiles[x][y].v, sizeof(Uint16), 1, fp);
			fwrite((void*)&map->tiles[x][y].blocked, sizeof(Uint8), 1, fp);
		}
	}

	fclose(fp);
	return 0;
}

int loadmap_Long(map_t* map, char* location)
{
	FILE* fp;
	int i;
	int x,y;

	if (map->tiles != NULL)
		freemap(map);

	if (map->name == NULL)
	{
		map->name = (char*)malloc(strlen(location)+1);
		strcpy(map->name, location);
	}
		
	
	fp = fopen(location, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Map load failure: Unable to open file.\n");
		return 1;
	}
	printf("%s", map->name);

	fread((void*)&map->dim_x, sizeof(Uint16), 1, fp);
	fread((void*)&map->dim_y, sizeof(Uint16), 1, fp);
	fread((void*)&map->num_resources, sizeof(Uint16), 1, fp);

	map->resource = calloc(map->num_resources, sizeof(SDL_Surface*));
	if (map->resource == NULL)
	{
		fprintf(stderr, "Malloc could not allocate space for map->resource.\n");
		exit(1);
	}

	map->resource_names = calloc(map->num_resources, sizeof(char*));
	if (map->resource_names == NULL)
	{
		fprintf(stderr, "Malloc could not allocate space for map->resource_names.\n");
		exit(1);
	}

	for (i=0; i<map->num_resources; i++)
	{
		int size;
		char* name;
		char* temp_str;
		SDL_Surface* temp;

		fread((void*)&size, sizeof(int), 1, fp);

		name = (char*)malloc(size+1);
		fread((void*)name, sizeof(char), size, fp);
		name[size]='\0';

		map->resource_names[i]=name;
		printf("Resource[%i]: %s\n", i, map->resource_names[i]);

		size = strlen(tiles_dir)+strlen(name)+1;
		temp_str = (char*) malloc(size);
		snprintf(temp_str, size, "%s%s", tiles_dir, map->resource_names[i]);

		if ((temp = IMG_Load(temp_str)) == NULL)
		{
			fprintf(stderr, "Unable to load graphic: %s\n", SDL_GetError());
			exit(1);
		}

		map->resource[i] = SDL_DisplayFormat(temp);
		if (map->resource[i] == NULL)
		{
			fprintf(stderr, "Unable to convert graphic: %s\n", SDL_GetError());
			exit(1);
		}

		free(temp_str);
	}

	map->tiles = calloc(map->dim_x, sizeof(tile_t*));
	if (map->tiles == NULL)
	{
		fprintf(stderr, "Malloc could not allocate space for map->tiles.\n");
		exit(1);
	}

	for (i=0; i < map->dim_x; i++)
	{
		map->tiles[i] = (tile_t*)calloc(map->dim_y, sizeof(tile_t));
		if (map->tiles[i] == NULL)
		{
			fprintf(stderr, "Malloc could not allocate space for map->tiles.\n");
			exit(1);
		}
	}

	printf("\nmap_x %i, map_y %i\n", map->dim_x, map->dim_y);
	printf("%i bytes allocated for %s\n\n",
		sizeof(map) + sizeof(tile_t) * map->dim_y * map->dim_x,
		map->name);

	for (x=0; x < map->dim_x; x++)
	{
		for (y=0; y < map->dim_y; y++)
		{
			fread((void*)&map->tiles[x][y].rid, sizeof(Uint8), 1, fp);
			fread((void*)&map->tiles[x][y].u, sizeof(Uint16), 1, fp);
			fread((void*)&map->tiles[x][y].v, sizeof(Uint16), 1, fp);
			fread((void*)&map->tiles[x][y].blocked, sizeof(Uint8), 1, fp);

			if (map->resource[ map->tiles[x][y].rid ] == NULL)
			{
				fprintf(stderr, "Unable to load map %s: resource %i not loaded.",
					map->name, map->tiles[x][y].rid);
				return(1);
			}
			map->tiles[x][y].source=map->resource[ map->tiles[x][y].rid ];
			map->tiles[x][y].occupied = 0;
		}
	}

	fclose(fp);
	return 0;
}
int  loadmap(map_t* map, char* name)
{
	if (map->tiles != NULL)
		freemap(map);
	
	map->name=(char*)malloc(strlen(name)+1);
	strcpy(name, map->name);
	
	char temp_str[strlen(name)+strlen(map_dir)+1];
	sprintf(temp_str, "%s%s", map_dir, name);
	printf("%s\n", temp_str);
	return loadmap(map, temp_str);
}

int newmap(map_t* map, char* tileset, int width, int height)
{
	SDL_Surface* temp;
	char* temp_str;
	int i, size;

	map->dim_x = width;
	map->dim_y = height;

	map->num_resources = 1;
	map->resource = calloc(map->num_resources, sizeof(SDL_Surface*));
	if (map->resource == NULL)
	{
		fprintf(stderr, "Malloc could not allocate space for map->resource[0].\n");
		exit(1);
	}

	map->resource_names = calloc(map->num_resources, sizeof(char*));
	if (map->resource_names == NULL)
	{
		fprintf(stderr, "Malloc could not allocate space for map->resources_names.\n");
		exit(1);
	}

	size = strlen(tileset)+1;
	map->resource_names[0] = malloc(size);
	snprintf(map->resource_names[0], size, "%s", tileset);

	temp_str = (char*) malloc(strlen(tiles_dir)+size);
	sprintf(temp_str, "%s%s", tiles_dir, map->resource_names[0]);
	printf("%s\n", temp_str);
	temp = IMG_Load(temp_str);
	if (temp == NULL)
	{
		fprintf(stderr, "Unable to load graphic: %s\n", SDL_GetError());
		exit(1);
	}
	map->resource[0] = SDL_DisplayFormat(temp);
	if (map->resource[0] == NULL)
	{
		fprintf(stderr, "Unable to convert graphic: %s\n", SDL_GetError());
		exit(1);
	}

	map->tiles = calloc(map->dim_x, sizeof(tile_t*));
	if (map->tiles == NULL)
	{
		fprintf(stderr, "Malloc could not allocate space for map->tiles.\n");
		exit(1);
	}

	for (i=0; i < map->dim_x; i++)
	{
		map->tiles[i] = calloc(map->dim_y, sizeof(tile_t));
		if (map->tiles[i] == NULL)
		{
			fprintf(stderr, "Malloc could not allocate space for map->tiles.\n");
			exit(1);
		}
	}

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

			map->tiles[x][y].u = 0;
			map->tiles[x][y].v = 0;
			map->tiles[x][y].occupied = 0;
			map->tiles[x][y].blocked  = 0;
		}
	}

	free(temp_str);
	return 0;
}

void freemap(map_t *map)
{
	int i;

	free(map->name);
	map->name=NULL;
	
	for (i=0; i<map->num_resources; i++)
	{
		free(map->resource_names[i]);
		SDL_FreeSurface(map->resource[i]);
	}
	free(map->resource_names);
	free(map->resource);
	map->resource_names = NULL;
	map->resource = NULL;

	map->num_resources = 0;

	for (i=0; i<map->dim_x; i++)
		free(map->tiles[i]);
	free(map->tiles);
	map->tiles = NULL;

	map->dim_x = 0;
	map->dim_y = 0;
}


// Tiles - Sub of Map
void drawtiles(SDL_Surface* surface, map_t map)
{
	SDL_Rect src, dest;
	int x,y;

	if (camera.x < 0) camera.x = 0;
	if (camera.x + SCREEN_WIDTH > map.dim_x * TILE_SIZE)
		camera.x = map.dim_x * TILE_SIZE - SCREEN_WIDTH;
	if (camera.y < 0) camera.y = 0;
	if (camera.y + SCREEN_HEIGHT > map.dim_y * TILE_SIZE)
		camera.y = map.dim_y * TILE_SIZE - SCREEN_HEIGHT;

	src.x=0; src.y=0;
	src.w=TILE_SIZE;
	src.h=TILE_SIZE;

	dest.x=0; dest.y=0;
	dest.w = TILE_SIZE;
	dest.h = TILE_SIZE;

	for (x = camera.x / TILE_SIZE; x <= camera.x / TILE_SIZE + SCREEN_TILES_X; x++)
	{
		for (y = camera.y / TILE_SIZE; y <= (camera.y / TILE_SIZE + SCREEN_TILES_Y); y++)
		{
			if ((x==map.dim_x) || (y==map.dim_y))
				break;
			dest.x = x * TILE_SIZE - camera.x;
			dest.y = y * TILE_SIZE - camera.y;

			src.x  = map.tiles[x][y].u * TILE_SIZE;
			src.y  = map.tiles[x][y].v * TILE_SIZE;

			src.w  = TILE_SIZE;
			dest.w = TILE_SIZE;

			src.h  = TILE_SIZE;
			dest.h = TILE_SIZE;

			/* Old manual clipping using mod-32 mathematics
			 * was used to make sure edge tiles would be within
			 * the screen bounds, otherwise problems on windows.
			if (x == camera.x / TILE_SIZE)
			{
				int offset = camera.x % TILE_SIZE;
				src.w  =  TILE_SIZE - offset;
				dest.w =  TILE_SIZE - offset;
				src.x  += offset;
				dest.x += offset;
			}

			if (x == camera.x / TILE_SIZE + SCREEN_TILES_X)
			{
				int offset = (camera.x + SCREEN_WIDTH) % TILE_SIZE;
				src.w  =  offset;
				dest.w =  offset;
			}

			if (y == camera.y / TILE_SIZE)
			{
				int offset = camera.y % TILE_SIZE;
				src.h  = TILE_SIZE - offset;
				dest.h = TILE_SIZE - offset;
				src.y  += offset;
				dest.y += offset;
			}

			if (y == camera.y / TILE_SIZE + SCREEN_TILES_Y)
			{
				int offset = (camera.y + SCREEN_WIDTH) % TILE_SIZE;
				src.h = offset;
				dest.h = offset;
			}
			*/

			SDL_BlitSurface(map.resource[ map.tiles[x][y].rid ], &src, surface, &dest);
		}
	}
}
