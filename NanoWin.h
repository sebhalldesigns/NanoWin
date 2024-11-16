#ifndef NANOWIN_H
#define NANOWIN_H

#define main SDL_main

#include <NanoGraph.h>
#include <stdbool.h>

// NANOKIT WINDOWING LIBRARY
// NOVEMBER 2024, SEB HALL

typedef struct nWindow* nWindow_h;

nWindow_h NanoWin_CreateWindow(int width, int height, const char* title);

void NanoWin_SetRootNode(nWindow_h window, nGraphNode_h node);

void NanoWin_PollEvents();

void NanoWin_ShowWindow(nWindow_h window);

#endif //NANOWIN_H