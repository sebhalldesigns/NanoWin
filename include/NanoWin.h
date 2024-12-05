#ifndef NANOWIN_H
#define NANOWIN_H

/* 
** NanoWin.h
** NanoKit Windowing Library
** November 2024, Seb Hall
*/

#include <stdbool.h>
#include <stdint.h>

typedef uintptr_t NWindow_h;

typedef struct
{
    float width;
    float height;
    
    float startX;
    float startY;

    const char* title;
} NWindowCreationInfo_t;

// for single-window platforms such as WASM a window is created by default
NWindow_h NanoWin_GetDefaultWindow();

// for multi-window platforms such as desktop, a window must be created
NWindow_h NanoWin_CreateWindow(NWindowCreationInfo_t creationInfo);

void NanoWin_DestroyWindow(NWindow_h window);

void NanoWin_PollEvents();

#endif //NANOWIN_H