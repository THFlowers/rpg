#include <stdio.h>
#include <SDL/SDL.h>
#include "rpg.h"

#ifndef _sprite
#define _sprite

typedef struct sprite_s {
	/* Private */
	SDL_Surface* image;
	int w;
	int h;
	/* Public */
	int screen_x, screen_y;
	int tile_x, tile_y;
	int animation;
	int anim_frame;
	void (*ai)(void* self, Uint8 event);
	void* aidata[NULL_EVENT];
} sprite_t;

int  loadSprite(char* path, sprite_t *sprite, int width, int height);
int  loadSprite_ByName(char* name, sprite_t *sprite, int width, int height);
int  freeSprite(sprite_t *sprite);
void freeAllSprite(void);
int  addSprite(sprite_t *sprite);
void drawSprite(SDL_Surface* screen, sprite_t *sprite);
void moveSprite(sprite_t* sprite, int distance_x, int distance_y);
int  drawAllSprite(SDL_Surface* screen);

void callbackSpriteAI(void* sprite, Uint8 event);
void callbackAllSpriteAI(Uint8 event);
void random_ai(void* sprite, Uint8 event);

#endif
