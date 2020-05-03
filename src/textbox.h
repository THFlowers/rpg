#ifndef RPG_TEXTBOX
#define RPG_TEXTBOX

extern TTF_Font* font;

typedef struct textbox_s {
	/* Text */
	char* text;
	int text_position;
	/* Screen */
	point_t position;
	SDL_Surface* render;
} textbox_t;


void newTextBox(textbox_t* textbox, char* text, int x, int y);
int  renderTextBox(textbox_t* textbox);
void drawTextBox(SDL_Surface* surface, textbox_t* textbox);

#endif
