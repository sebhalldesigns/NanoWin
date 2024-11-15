#ifndef NANOWIN_H
#define NANOWIN_H

#define main SDL_main


// NANOKIT WINDOWING LIBRARY
// NOVEMBER 2024, SEB HALL

typedef struct nWindow* nWindow_h;

nWindow_h NanoWin_CreateWindow(int width, int height, const char* title);

void NanoWin_PollEvents();

#endif //NANOWIN_H