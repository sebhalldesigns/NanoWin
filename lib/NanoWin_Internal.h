#ifndef NANOWIN_INTERNAL_H
#define NANOWIN_INTERNAL_H

/* 
** NanoWin_Internal.h
** NanoKit Windowing Library
** November 2024, Seb Hall
*/

#include <stdbool.h>
#include <stdint.h>

typedef struct NWindow_t 
{
    float width;
    float height;
    
    const char* title;
} NWindow_t;



#endif // NANOWIN_INTERNAL_H