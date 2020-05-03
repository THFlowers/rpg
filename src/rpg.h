#ifndef RPG
#define RPG

#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480

#define	TILE_SIZE		32

#define SCREEN_TILES_X		SCREEN_WIDTH/TILE_SIZE
#define SCREEN_TILES_Y		SCREEN_HEIGHT/TILE_SIZE

#define STEP			3
#define STEP_RATE		3

enum DIRECTION {UP, RIGHT, DOWN,  LEFT};
enum AI_EVENTS {WALK, TALK, CLEAR, NULL_EVENT};

static char* sprite_dir = "media/sprites/";
static char*  tiles_dir = "media/tiles/";
static char*    map_dir = "./";
static char*     py_dir = "./";

typedef struct tile_s {
	SDL_Surface* source;
	Uint8 rid; /* Resource ID */
	Uint16 u, v;
	Uint8 occupied;
	Uint8 blocked;
} tile_t;

typedef struct map_s {
	char* name;
	Uint8 num_resources;
	char** resource_names;
	SDL_Surface** resource;
	Uint16 dim_x;
	Uint16 dim_y;
	tile_t** tiles;
} map_t;

typedef struct point_s {
	int x;
	int y;
} point_t;

extern Uint8 dirty_screen;
extern map_t map;
extern point_t camera;

void drawtiles(SDL_Surface* surface, map_t map);
int  savemap(map_t* map, char* filename);
int  loadmap(map_t* map, char* name);
int  loadmap_Long(map_t* map, char* location);
int  newmap(map_t* map, char* tileset, int width, int height);
void freemap(map_t* map);

#endif
