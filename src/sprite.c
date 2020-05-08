#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "sprite.h"
#include "rpg.h"

sprite_t** npc_list = NULL;
static int npc_list_size = 0;

int loadSprite(char* path, sprite_t* sprite, int width, int height)
{
	if (addSprite(sprite) != 0) return -1;

	SDL_Surface* temp = IMG_Load(path);
	if (temp == NULL)
	{
		fprintf(stderr, "Unable to load sprite: %s\n", SDL_GetError());
		return(1);
	}
	//SDL_SetColorKey(temp, SDL_RLEACCEL,
	//				(Uint16) SDL_MapRGB(temp->format, 0, 0, 255));
	SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL,
			(Uint16) SDL_MapRGB(temp->format, 0, 0, 255));
	sprite->image=SDL_DisplayFormat(temp);
	//sprite->image=SDL_ConvertSurfaceFormat(temp, temp->format->format, 0);
	if (sprite->image == NULL)
	{
		fprintf(stderr, "Unable to convert bitmap: %s\n", SDL_GetError());
		return(1);
	}
	SDL_FreeSurface(temp);

	sprite->animation = DOWN;
	sprite->anim_frame = 0;
	sprite->w=width;
	sprite->h=height;

	sprite->ai=NULL;

	return(0);
}
int loadSprite_ByName(char* name, sprite_t *sprite, int width, int height)
{
	char temp_str[strlen(name)+strlen(sprite_dir)];
	sprintf(temp_str, "%s%s", sprite_dir, name);
	//printf("%s\n", temp_str);
	return loadSprite(temp_str, sprite, width, height);
}

int addSprite(sprite_t *sprite)
{
	if (npc_list == NULL)
	{
		npc_list_size = 1;
		npc_list = malloc(sizeof(sprite_t*));
	}
	else
	{
		npc_list_size++;
		npc_list = realloc(npc_list, npc_list_size * sizeof(sprite_t*));
	}
	
	if (npc_list == NULL)
	{
		fprintf(stderr, "Could not allocate npc_list\n");
		return -1;
	}
	
	npc_list[npc_list_size-1]=sprite; // 0 is 1st indice, so npc_list_size-1 is real coordinates
	return 0;
}

// 0 Sucess, + Minor error, - Major error
int freeSprite(sprite_t *sprite)
{
	if (npc_list_size <= 0)
	{
		fprintf(stderr, "Nothing to free, list is zero.\n");
		return 1;
	}
	
	if (sprite == NULL) 
	{
		fprintf(stderr, "Can not free an already free sprite!");
		return -5;
	}

	if (sprite->ai != NULL)
		sprite->ai(sprite, CLEAR);
	
	SDL_FreeSurface(sprite->image);
	sprite = NULL;
	
	// Using real indices
	int pos;
	for (pos = 0; npc_list[pos] == NULL; pos++);
	
	// It works here (GNU/Linux 2.6) but may cause problems on other platforms,
	// if pos is at end it moves non-existant data to the left.
	//                                 V Thar be the problem (extends past end)
	memmove(npc_list+pos, npc_list+pos+1, npc_list_size-pos); // Taking the return fucks up malloc.
	
	npc_list_size--;
	npc_list = realloc(npc_list, npc_list_size * sizeof(sprite_t*));

	return 0;
}

void freeAllSprite(void)
{
	if (npc_list_size <= 0)
	{
		fprintf(stderr, "Nothing to free, list is zero.");
		return;
	}

	sprite_t* sprite;
	SDL_Surface *surf;

	int pos;
	for (pos=0; pos<npc_list_size; pos++)
	{
		sprite = npc_list[pos];
		surf = sprite->image;

		if (sprite->ai != NULL)
			sprite->ai(sprite, CLEAR);

		printf("%i\n", pos);
		printf("Address of pointer: %p\n", sprite);
		printf("Address of image:   %p\n", surf);

		SDL_FreeSurface(surf);
		npc_list[pos]=NULL;
	}

	npc_list_size=0;
	free(npc_list);
}

void drawSprite(SDL_Surface* screen, sprite_t *sprite)
{
	SDL_Rect src, dest;
	
	src.x = sprite->animation * TILE_SIZE;
	src.y = ((int) sprite->anim_frame/STEP_RATE) * TILE_SIZE;
	src.w = sprite->w;
	src.h = sprite->h;
	
	dest.x = sprite->screen_x - camera.x;
	dest.y = sprite->screen_y - camera.y;

	//printf("Draw at: %i\t%i\n", dest.x, dest.y);
	
	SDL_BlitSurface(sprite->image, &src, screen, &dest);
}

// returns # of sprites drawn
int drawAllSprite(SDL_Surface* screen)
{
	sprite_t* sprite;
	int ret = 0;

	int pos;
	for (pos=0; pos<npc_list_size; pos++)
	{
		//printf("%i\n", pos);
		sprite = npc_list[pos];
		
		// Draw Check
		// X-axis check
		if( ((sprite->screen_x > camera.x && sprite->screen_x < camera.x + SCREEN_WIDTH) &&
		    ((sprite->screen_x + sprite->w > camera.x && sprite->screen_x < camera.x + SCREEN_WIDTH)) ))
		{
			drawSprite(screen, sprite);
			ret++;
			continue;
		}

		// Y-axis check
		if( ((sprite->screen_y > camera.y && sprite->screen_y < camera.y + SCREEN_HEIGHT) &&
		    ((sprite->screen_y + sprite->h > camera.y && sprite->screen_y < camera.y + SCREEN_HEIGHT)) ))
		{
			drawSprite(screen, sprite);
			ret++;
			continue;
		}

	}
	
	return ret;
}

void moveSprite(sprite_t *sprite, int distance_x, int distance_y)
{
	int next_x, next_y;

	int move_x = distance_x;
	int move_y = distance_y;

	map.tiles[sprite->tile_x][sprite->tile_y].occupied=0;

	next_x = (sprite->screen_x + distance_x) / TILE_SIZE;
	next_y = (sprite->screen_y + distance_y) / TILE_SIZE;

	if ( (next_x == map.dim_x-1)			||
	     (next_y == map.dim_y-1)			||
	     (sprite->screen_x + distance_x < 0)	||
	     (sprite->screen_y + distance_y < 0) )
		//printf("At edge of map.\n");
		;
	else
	{
		if (distance_x != 0)
		{
			if ((map.tiles[next_x][next_y].blocked || map.tiles[next_x][next_y].occupied) ||
			    (map.tiles[next_x][next_y+1].blocked || map.tiles[next_x][next_y+1].occupied))
			{
				move_x = 0;
				//printf("Blocked tile detected (L):\t");
				//printf("[%i,%i]\n", next_x, next_y);
				//printf("Blocked: %i\n",  map.tiles[next_x][next_y].blocked);
				//printf("Occupied: %i\n", map.tiles[next_x][next_y].occupied);
			}
	
			if ((map.tiles[next_x+1][next_y].blocked || map.tiles[next_x+1][next_y].occupied) ||
			    (map.tiles[next_x+1][next_y+1].blocked || map.tiles[next_x+1][next_y+1].occupied))
			{
				move_x = 0;
				//printf("Blocked tile detected (R):\t");
				//printf("[%i,%i]\n", next_x, next_y);
				//printf("Blocked: %i\n",  map.tiles[next_x+1][next_y+1].blocked);
				//printf("Occupied: %i\n", map.tiles[next_x+1][next_y+1].occupied);
			}
		}
	
		if (distance_y != 0)
		{
			if ((map.tiles[next_x][next_y].blocked || map.tiles[next_x][next_y].occupied) ||
			    (map.tiles[next_x+1][next_y].blocked || map.tiles[next_x+1][next_y].occupied))
			{
				move_y = 0;
				//printf("Blocked tile detected (U):\t");
				//printf("[%i,%i]\n", next_x, next_y);
				//printf("Blocked: %i\n",  map.tiles[next_x+1][next_y+1].blocked);
				//printf("Occupied: %i\n", map.tiles[next_x+1][next_y+1].occupied);
			}
	
			if ((map.tiles[next_x][next_y+1].blocked || map.tiles[next_x][next_y+1].occupied) ||
			    (map.tiles[next_x+1][next_y+1].blocked || map.tiles[next_x+1][next_y+1].occupied))
			{
				move_y = 0;
				//printf("Blocked tile detected (D):\t");
				//printf("[%i,%i]\n", next_x, next_y);
				//printf("Blocked: %i\n",  map.tiles[next_x+1][next_y+1].blocked);
				//printf("Occupied: %i\n", map.tiles[next_x+1][next_y+1].occupied);
			}
		}
	
		sprite->screen_x+=move_x;
		sprite->screen_y+=move_y;
	}
	
	int midpoint_x, midpoint_y;
	midpoint_x = sprite->screen_x + sprite->w/2;
	midpoint_y = sprite->screen_y + sprite->h/2;

	sprite->tile_x = (int) midpoint_x / TILE_SIZE;
	sprite->tile_y = (int) midpoint_y / TILE_SIZE;

	map.tiles[sprite->tile_x][sprite->tile_y].occupied=1;

	if (sprite->screen_x + sprite->w >= TILE_SIZE * map.dim_x)
		//sprite->screen_x = (map.dim_x-1)*TILE_SIZE-1;
		sprite->screen_x = map.dim_x * TILE_SIZE - sprite->w;
	if (sprite->screen_x < 0) sprite->screen_x = 0;
	if (sprite->screen_y + sprite->h >= TILE_SIZE * map.dim_y)
		//sprite->screen_y = (TILES_Y-1)*TILE_SIZE-1;
		sprite->screen_y = map.dim_y * TILE_SIZE - sprite->h;
	if (sprite->screen_y < 0) sprite->screen_y = 0;

	sprite->anim_frame++;
	if (sprite->h * (sprite->anim_frame/STEP_RATE) >= sprite->image->h)
		sprite->anim_frame=0;
}

// AI - Sub of Sprite
void callbackSpriteAI(void* sprite, Uint8 event)
{
	sprite_t* npc = (sprite_t*)sprite;
	npc->ai((void*)sprite, event);
}

void callbackAllSpriteAI(Uint8 event)
{
	int pos;
	for (pos=0; pos<npc_list_size; pos++)
	{
		if (npc_list[pos]->ai != NULL)
			npc_list[pos]->ai((void*)npc_list[pos], event);
	}
}

void random_ai(void* sprite, Uint8 event)
{
	sprite_t* npc = (sprite_t*)sprite;
	int seed;

	switch(event)
	{
		case WALK:
			if (npc->aidata[WALK] != NULL)
				seed = npc->aidata[WALK];
			else
				seed = 25;
			
			switch (rand()%seed)
			{
				case UP:
					npc->animation=UP;
					moveSprite(npc, 0, -STEP);
					break;
				case DOWN:
					npc->animation=DOWN;
					moveSprite(npc, 0, STEP);
					break;
				case LEFT:
					npc->animation=LEFT;
					moveSprite(npc, -STEP, 0);
					break;
				case RIGHT:
					npc->animation=RIGHT;
					moveSprite(npc, STEP, 0);
					break;
				default:
					switch(npc->animation)
					{
						case UP:
							moveSprite(npc, 0, -STEP);
							break;
						case DOWN:
							moveSprite(npc, 0, STEP);
							break;
						case LEFT:
							moveSprite(npc, -STEP, 0);
							break;
						case RIGHT:
							moveSprite(npc, STEP, 0);
							break;
					}
					break;
			}
			break;
		case TALK:
			printf("Hello!\n");
			break;
		case CLEAR:
			npc->aidata[WALK] == NULL;
			npc->ai == NULL;
			break;
	}
}
