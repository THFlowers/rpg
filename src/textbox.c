#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "rpg.h"
#include "textbox.h"
#include "colors.h"

void newTextBox(textbox_t* textbox, char* text, int x, int y)
{
	textbox->text = text;
	textbox->text_position = 0;
	textbox->position.x = x;
	textbox->position.y = y;
	if (renderTextBox(textbox) != 0)
		exit(1);
}

int renderTextBox(textbox_t* textbox)
{
	SDL_Surface	*temp;
	SDL_Rect	rect;
	SDL_VideoInfo*	videoinfo;
	//SDL_RendererInfo rendererInfo;
	int color;
	
	int	length;
	char*	lines[4];
	int	line = 0;

	int width  = 0;
	int height = 0;
	int lineskip;
	
	int i = 0;
	int start = 0;
	int w = 0;

	if (textbox->text_position > 0)
	{
		SDL_FreeSurface(textbox->render);
		i = textbox->text_position;
		start = i;
	}

	while (i <= strlen(textbox->text)) // Don't go past the end of the text
	{
		// Remaining length
		length = strlen(textbox->text+start);
		// Move forward one line length past start
		i += 25;

		// Shorter than 25 char case: for short lines and end of wrapping.
		if (length <= 25)
		{
			// Line is all text past 'start'
			lines[line] = strdup(textbox->text+start);

			TTF_SizeText(font, lines[line], &w, NULL);
			if (w > width) width = w;

			break;
		}

		// Search backwards for space or separating dash '-'
		for (; i > start; i--)
		{
			char* ch = textbox->text+i;
			if (strncmp(ch, " ", 1) == 0)
			{
				i++; // Skip the space, so no gap in new line
				break;
			}
			else if (strncmp(ch, "-", 1) == 0)
				break;
		}

		// Line is start -> i; by copying i-start characters (Selected area start->i minus leading characters)
		lines[line] = strndup(textbox->text+start, i-start); //TODO: Doesn't work with non GNU / newer

		// Get the width (w) of the current line, save it if over current maximum line width (width)
		TTF_SizeText(font, lines[line], &w, NULL);
		if (w > width) { width = w; }

		// Check for bounds and then iterate position in lines
		if (line == 3)
		{
			// Save i position for next render
			textbox->text_position = i;
			break;
		}
		line++;

		// Beginning of next line is current 'end'; the text was wrapped
		start = i;
	}

	videoinfo = SDL_GetVideoInfo();
	//SDL_GetRenderDriverInfo(0, &rendererInfo); // TODO
	lineskip = TTF_FontLineSkip(font);

	height = (line+1) * (lineskip + height);

	/*
	Uint32 format = rendererInfo.texture_formats[0];
	textbox->render = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, width, height,
													 SDL_BITSPERPIXEL(format), format);
	 */
	textbox->render = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
		 videoinfo->vfmt->BitsPerPixel,
		 videoinfo->vfmt->Rmask,
		 videoinfo->vfmt->Gmask,
		 videoinfo->vfmt->Bmask,
		 videoinfo->vfmt->Amask);

	if (textbox->render == NULL)
	{
		fprintf(stderr, "Something went horribly wrong.\n");
		return 1;
	}

	color = SDL_MapRGB(textbox->render->format, white.r, white.g, white.b);
	SDL_FillRect(textbox->render, NULL, color);

	printf("Number of lines: %i\n", line+1);
	for (i=0; i<=line; i++)
	{
		printf("Line %i: %s\n", i, lines[i]);

		temp = TTF_RenderText_Solid(font, lines[i], black);
		if (temp == NULL)
		{
			fprintf(stderr, "Could not render text: %s\n", TTF_GetError());
			exit(1);
		}
		
		rect.x = 0;
		rect.y = i * lineskip;
		rect.w = 0;
		rect.h = 0;
		
		SDL_BlitSurface(temp, NULL, textbox->render, &rect);
		SDL_FreeSurface(temp);
		
		free(lines[i]);
	}

	printf("\n");
	return 0;
}

void drawTextBox(SDL_Surface* surface, textbox_t* textbox)
{
	SDL_Rect dest;

	dest.x = textbox->position.x;
	dest.y = textbox->position.y;
	dest.w = textbox->render->w;
	dest.h = textbox->render->h;

	SDL_BlitSurface(textbox->render, NULL, surface, &dest);
}
