#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "rpg.h"

#include <gtk/gtk.h>

#include <SDL/SDL_thread.h>
SDL_Thread* thread;

enum FLAGS { SDL_EXIT = 1, SDL_DIRTY = 2, MAP_SAVE = 4, FILE_OPEN = 8, NEW_MAP = 16 };
/*	SDL_EXIT	Is SDL still running?
 *	SDL_DIRTY	Is a redraw needed?
 *	MAP_SAVE	Is a save needed?
 *	FILE_OPEN	Is a file loaded and can it be saved?
 */
void flags(Uint8 flags);

// add list of things to save
SDL_mutex* singleton_mutex;
typedef struct singleton_s
{
	Uint8 		flags; // still in testing
} singleton_t;
singleton_t singleton;

static gboolean delete_event( GtkWidget *widget,
			      GdkEvent	*event,
			      gpointer	 data )
{
	return FALSE;
}

static
void destroy( GtkWidget *widget,
	      gpointer	 data )
{
	//SDL_mutexP(singleton_mutex);
	singleton.flags = SDL_EXIT;
	//SDL_mutexV(singleton_mutex);
	SDL_DestroyMutex(singleton_mutex);

	SDL_WaitThread(thread, NULL);
	gtk_main_quit();
}

int
SDLThread(void *data)
{
	SDL_Surface* screen;
	SDL_Surface* temp;
	SDL_Event event;

	int camera_xvel = 0;
	int camera_yvel = 0;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr,
			"Could not initialize video: %s\n", SDL_GetError());
		gtk_main_quit(); // Seems out of place, is this good form?
		return 1;
	}

	SDL_WM_SetIcon(SDL_LoadBMP("media/icon_64.bmp"), NULL);

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, SDL_DOUBLEBUF | SDL_ANYFORMAT);

	if (data != NULL)
	{
		char* caption = (char*)malloc(14 + strlen(data));
		sprintf(caption, "RPG Editor - %s", data);
		SDL_WM_SetCaption(caption, "RPG Editor");
		if (loadmap_Long(&map, (char*) data) != 0)
			exit(0);
	}
	else
	{
		newmap(&map, "grass.bmp", map.dim_x, map.dim_y);
		SDL_WM_SetCaption("RPG Editor", "RPG Editor");
	}

	// Prepare flags for initial loop
	singleton.flags = SDL_DIRTY | FILE_OPEN;

	while ((singleton.flags & SDL_EXIT) != SDL_EXIT)
	{
		while (SDL_PollEvent(&event))
		{
			int x, y;
			switch (event.type)
			{
				case SDL_QUIT:
					singleton.flags |= SDL_EXIT;
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_a:
							camera_xvel -=5;
							break;
						case SDLK_d:
							camera_xvel +=5;
							break;
						case SDLK_w:
							camera_yvel -=5;
							break;
						case SDLK_s:
							camera_yvel +=5;
							break;
						default: break;
					}
					break;
				case SDL_KEYUP:
					switch (event.key.keysym.sym)
					{
						case SDLK_a:
							camera_xvel +=5;
							singleton.flags |= SDL_DIRTY;
							break;
						case SDLK_d:
							camera_xvel -=5;
							singleton.flags |= SDL_DIRTY;
							break;
						case SDLK_w:
							camera_yvel +=5;
							singleton.flags |= SDL_DIRTY;
							break;
						case SDLK_s:
							camera_yvel -=5;
							singleton.flags |= SDL_DIRTY;
							break;
						default: break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					x = (event.button.x + camera.x) / TILE_SIZE;
					y = (event.button.y + camera.y) / TILE_SIZE;
					switch (event.button.button)
					{
						case SDL_BUTTON_LEFT:
							map.tiles[x][y].u = 0;
							map.tiles[x][y].v = 0;
							map.tiles[x][y].blocked = 0;
							break;
						case SDL_BUTTON_RIGHT:
							map.tiles[x][y].u = 0;
							map.tiles[x][y].v = 0;
							map.tiles[x][y].blocked = 0;
							break;
						case SDL_BUTTON_MIDDLE:
							map.tiles[x][y].blocked ^= 1;
							break;
						singleton.flags |= SDL_DIRTY;
					}
				default: break;
			break;
			}
		}

		SDL_mutexP(singleton_mutex);
		if ( (singleton.flags & SDL_DIRTY) == SDL_DIRTY )
		{
			drawtiles(screen, map);
			singleton.flags -= SDL_DIRTY; // &= and |= don't work here
		}
		SDL_mutexV(singleton_mutex);

		camera.x += camera_xvel;
		camera.y += camera_yvel;

		SDL_Flip(screen);
		SDL_Delay(50);
	}
		
	// Clean up, prepare for a new map to load
	singleton.flags = SDL_EXIT;

	camera.x = 0;
	camera.y = 0;
	freemap(&map);
	SDL_Quit();
	return 0;
}

static
void openMap(gpointer map_name)
{
	if (thread != NULL)
	{
		singleton.flags |= SDL_EXIT;
		SDL_WaitThread(thread, NULL);
	}

	thread = SDL_CreateThread(SDLThread, (char*) map_name);
}

static
void openMapDialog(gpointer data)
{
	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Select Map",
					     NULL,
					     GTK_FILE_CHOOSER_ACTION_OPEN,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					     NULL);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		openMap((char*) gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)));
	}

	gtk_widget_destroy (GTK_WIDGET(dialog));
}

static
void saveMapDialog(gpointer data)
{
	flags(singleton.flags);
	if ((singleton.flags & FILE_OPEN) != FILE_OPEN) return;

	GtkWidget *dialog;
	dialog = gtk_file_chooser_dialog_new("Save Map",
					     NULL,
					     GTK_FILE_CHOOSER_ACTION_SAVE,
					     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					     GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					     NULL);
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), map_dir);

	if (map.name == NULL)
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), "NEW.map");
	else
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), map.name);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		savemap(&map,
			(char *) gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)));
		if (map.name == NULL)
			map.name = (char *) gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
	}

	gtk_widget_destroy (GTK_WIDGET(dialog));
}

static
void newMapDialog(gpointer data)
{
	flags(singleton.flags);
	if ((singleton.flags & FILE_OPEN) == FILE_OPEN) return;

	GtkWidget *dialog, *label, *slider_x, *slider_y;

	dialog = gtk_dialog_new_with_buttons("New Map",
					      NULL,
					      GTK_DIALOG_DESTROY_WITH_PARENT,
					      GTK_STOCK_OK,
					      GTK_RESPONSE_OK,
					      GTK_STOCK_CANCEL,
					      GTK_RESPONSE_CANCEL,
					      NULL);

	label = gtk_label_new("Select Map X and Y dimensions");
	gtk_container_add(GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), label);

	slider_x = gtk_hscale_new_with_range(SCREEN_TILES_X, 500, 1);
	gtk_container_add(GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), slider_x);

	slider_y = gtk_hscale_new_with_range(SCREEN_TILES_Y, 500, 1);
	gtk_container_add(GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), slider_y);

	gtk_widget_show_all (dialog);

	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	switch (result)
	{
		case GTK_RESPONSE_OK:
			if (thread != NULL)
			{
				singleton.flags = SDL_EXIT;
				SDL_WaitThread(thread, NULL);
			}
			
			map.dim_x = gtk_range_get_value(GTK_RANGE (slider_x));
			map.dim_y = gtk_range_get_value(GTK_RANGE (slider_y));
			
			thread = SDL_CreateThread(SDLThread, NULL);
			break;
		case GTK_RESPONSE_CANCEL:
			printf("Canceled\n");
			break;
		default:
			printf("Response: %i", result);
			break;
	}

	gtk_widget_destroy(dialog);
}

int
main(int argc, char** argv)
{
	singleton_mutex = SDL_CreateMutex();

	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *separator;
	GtkWidget *notebook;
	GtkWidget *icon, *image;
	GtkWidget *scroll_win;
	GtkWidget *box;

	gtk_init(&argc, &argv);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (window), "RPG Editor - Toolbox");
	gtk_window_set_default_size(GTK_WINDOW (window), 240, 450);

	g_signal_connect (G_OBJECT (window), "delete_event",
			  G_CALLBACK (delete_event), NULL);

	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (destroy), NULL);

	box = gtk_vbox_new (FALSE, 0);
	gtk_container_add(GTK_CONTAINER (window), box);

	// Open Demo Map
	button = gtk_button_new_with_label("Open Demo Map");
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

	char demo[strlen(map_dir)+9];
	sprintf(demo, "%sdemo.map", map_dir);
	g_signal_connect_swapped(G_OBJECT (button), "clicked",
				 G_CALLBACK (openMap), demo);
	gtk_widget_show(button);

	/* Open Map */
	button = gtk_button_new_with_label("Open Map");
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

	g_signal_connect_swapped(G_OBJECT (button), "clicked",
				 G_CALLBACK (openMapDialog), NULL);
	gtk_widget_show(button);

	/* New Map */
	button = gtk_button_new_with_label("New Map");
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

	g_signal_connect_swapped(G_OBJECT (button), "clicked",
				 G_CALLBACK (newMapDialog), NULL);

	gtk_widget_show(button);

	/* Save Map */
	button = gtk_button_new_with_label("Save Map");
	gtk_box_pack_start(GTK_BOX(box), button, TRUE, TRUE, 0);

	g_signal_connect_swapped(G_OBJECT (button), "clicked",
				 G_CALLBACK (saveMapDialog), NULL);
	gtk_widget_show(button);

	/* Separator */
	separator = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(box), separator, FALSE, FALSE, 8);
	gtk_widget_show(separator);

	// Experimental
	
	icon = gtk_image_new_from_file("media/icon_32.bmp");
	image = gtk_image_new_from_file("media/tiles/free.png");

	scroll_win = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scroll_win),
					      image);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_win),
				    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	notebook = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), TRUE);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), icon, NULL);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll_win, NULL);

	gtk_box_pack_start(GTK_BOX(box), notebook, TRUE, TRUE, 0);
	gtk_widget_show_all(notebook);

	gtk_widget_show(box);
	gtk_widget_show(window);
	gtk_main();
	return 0;
}

void
flags(Uint8 flags)
{
	printf("Flags\t\"%i\t\"\n", flags);
	printf("SDL_EXIT: %i\n", (flags & SDL_EXIT));
	printf("SDL_DIRTY: %i\n", (flags & SDL_DIRTY));
	printf("FILE_OPEN: %i\n", (flags & FILE_OPEN));
	printf("MAP_SAVE: %i\n", (flags & MAP_SAVE));
	printf("NEW_MAP: %i\n\n", (flags & NEW_MAP));
}
