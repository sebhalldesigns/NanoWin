
/******************************************************************************
 * NanoWin.c
 * 
 * NanoWin is a simple windowing library built on top of SDL2. It provides a 
 * simple API for creating windows and handling events.
 * 
 * November 2024, Seb Hall
 *****************************************************************************/

#include "NanoWin.h"
#include <SDL2/SDL.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

/******************************************************************************
 * MARK: TYPE DEFINITIONS
 *****************************************************************************/

typedef struct nWindow
{
    uint32_t windowId;
    SDL_Window* sdlWindow;
    SDL_Renderer* sdlRenderer;

    char* title;
    
    int width;
    int height;

    nWindow_h next;
} nWindow;

/******************************************************************************
 * MARK: LOCAL FUNCTION PROTOTYPES
 *****************************************************************************/

static int EventFilter(void*, SDL_Event *event);
static void RenderWindow(nWindow_h window);

/******************************************************************************
 * MARK: LOCAL VARIABLES
 *****************************************************************************/

static bool sdlInitialized = false;
static nWindow_h windows = NULL;

/******************************************************************************
 * MARK: GLOBAL FUNCTION IMPLEMENTATIONS
 *****************************************************************************/

nWindow_h NanoWin_CreateWindow(int width, int height, const char* title)
{

    if (!sdlInitialized)
    {
        if (SDL_Init(SDL_INIT_VIDEO) != 0)
        {
            printf("Failed to initialize SDL: %s\n", SDL_GetError());
            exit(1);
        }

        sdlInitialized = true;
    }

    nWindow* window = malloc(sizeof(nWindow));
    window->next = NULL;
    window->title = title;
    window->width = width;
    window->height = height;

    window->sdlWindow = SDL_CreateWindow(
        title, 
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, 
        width, 
        height, 
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    window->windowId = SDL_GetWindowID(window->sdlWindow);

    window->sdlRenderer = SDL_CreateRenderer(window->sdlWindow, -1, SDL_RENDERER_ACCELERATED);

    if (windows == NULL)
    {
        windows = window;
    }
    else
    {
        nWindow_h current = windows;
        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = window;
    }

    printf("CREATED WINDOW: %s %lu\n", title, window);

    SDL_SetEventFilter(EventFilter, NULL);
    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

    return window;
}

void NanoWin_PollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            exit(0);
        }
    }
}

/******************************************************************************
 * MARK: LOCAL FUNCTION IMPLEMENTATIONS
 *****************************************************************************/

static int EventFilter(void*, SDL_Event *event) 
{

    if (event->type == SDL_WINDOWEVENT && event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) 
    {

        nWindow_h current = windows;

        while (current != NULL) 
        {
            if (current->windowId == event->window.windowID) 
            {   
                printf("Window \"%s\" size changed to %d x %d\n", current->title, event->window.data1, event->window.data2);
                current->width = event->window.data1;
                current->height = event->window.data2;
                RenderWindow(current);
                return 1;
            }

            current = current->next;
        }

    }

    return 1;
}

static void RenderWindow(nWindow_h window)
{
    SDL_SetRenderDrawColor(window->sdlRenderer, 0, 255, 0, 255);
    SDL_RenderClear(window->sdlRenderer);
    SDL_RenderPresent(window->sdlRenderer);
}
