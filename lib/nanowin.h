/***************************************************************
**
** NanoKit Library Header File
**
** File         :  nanowin.h
** Module       :  nanowin
** Author       :  SH
** Created      :  2025-02-23 (YYYY-MM-DD)
** License      :  MIT
** Description  :  NanoKit Window API
**
***************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

/***************************************************************
** MARK: INCLUDES
***************************************************************/

#include <stdint.h>
#include <stdbool.h>

#if _WIN32
    #define WIN32_LEAN_AND_MEAN

    #ifndef UNICODE
    #define UNICODE
    #endif 

    #include <windows.h>
    #include <windowsx.h>

    #include <extern/glad/glad.h>
#endif


/***************************************************************
** MARK: CONSTANTS & MACROS
***************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


/* General Keys */
#define NK_KEYCODE_SPACE            (0x0001U)
#define NK_KEYCODE_BACKSPACE        (0x0002U)
#define NK_KEYCODE_TAB              (0x0003U)
#define NK_KEYCODE_CLEAR            (0x0004U)
#define NK_KEYCODE_RETURN           (0x0005U)
#define NK_KEYCODE_PAUSE            (0x0006U)
#define NK_KEYCODE_ESCAPE           (0x0007U)
#define NK_KEYCODE_DELETE           (0x0008U)

/* Modifier Keys */
#define NK_KEYCODE_SHIFT            (0x0011U)
#define NK_KEYCODE_CONTROL          (0x0012U)
#define NK_KEYCODE_META             (0x0013U)
#define NK_KEYCODE_ALT              (0x0014U)
#define NK_KEYCODE_SUPER            (0x0015U)
#define NK_KEYCODE_HYPER            (0x0016U)

/* Navigation Keys */
#define NK_KEYCODE_PAGE_UP          (0x0021U)
#define NK_KEYCODE_PAGE_DOWN        (0x0022U)
#define NK_KEYCODE_END              (0x0023U)
#define NK_KEYCODE_HOME             (0x0024U)
#define NK_KEYCODE_LEFT             (0x0025U)
#define NK_KEYCODE_UP               (0x0026U)
#define NK_KEYCODE_RIGHT            (0x0027U)
#define NK_KEYCODE_DOWN             (0x0028U)

/* Editing Keys */
#define NK_KEYCODE_SELECT           (0x0031U)
#define NK_KEYCODE_PRINT            (0x0032U)
#define NK_KEYCODE_EXECUTE          (0x0033U)
#define NK_KEYCODE_INSERT           (0x0034U)
#define NK_KEYCODE_HELP             (0x0035U)

/* Function Keys */
#define NK_KEYCODE_F1                (0x0041U)
#define NK_KEYCODE_F2                (0x0042U)
#define NK_KEYCODE_F3                (0x0043U)
#define NK_KEYCODE_F4                (0x0044U)
#define NK_KEYCODE_F5                (0x0045U)
#define NK_KEYCODE_F6                (0x0046U)
#define NK_KEYCODE_F7                (0x0047U)
#define NK_KEYCODE_F8                (0x0048U)
#define NK_KEYCODE_F9                (0x0049U)
#define NK_KEYCODE_F10               (0x004AU)
#define NK_KEYCODE_F11               (0x004BU)
#define NK_KEYCODE_F12               (0x004CU)
#define NK_KEYCODE_F13               (0x004DU)
#define NK_KEYCODE_F14               (0x004EU)
#define NK_KEYCODE_F15               (0x004FU)
#define NK_KEYCODE_F16               (0x0050U)
#define NK_KEYCODE_F17               (0x0051U)
#define NK_KEYCODE_F18               (0x0052U)
#define NK_KEYCODE_F19               (0x0053U)
#define NK_KEYCODE_F20               (0x0054U)
#define NK_KEYCODE_F21               (0x0055U)
#define NK_KEYCODE_F22               (0x0056U)
#define NK_KEYCODE_F23               (0x0057U)
#define NK_KEYCODE_F24               (0x0058U)

/***************************************************************
** MARK: TYPEDEFS
***************************************************************/

typedef enum
{
    NK_POINTER_ACTION_PRIMARY           = 0x01,
    NK_POINTER_ACTION_SECONDARY         = 0x02,
    NK_POINTER_ACTION_TERTIARY          = 0x03,
    NK_POINTER_ACTION_EXTENDED_1        = 0x04,
    NK_POINTER_ACTION_EXTENDED_2        = 0x05
} nkPointerAction_t;

typedef enum
{
    NK_WINDOW_VISIBILITY_VISIBLE        = 0x01,
    NK_WINDOW_VISIBILITY_HIDDEN         = 0x02,
    NK_WINDOW_VISIBILITY_MINIMIZED      = 0x03,
    NK_WINDOW_VISIBILITY_MAXIMIZED      = 0x04,
    NK_WINDOW_VISIBILITY_FULLSCREEN     = 0x05
} nkWindowVisibility_t;

typedef enum
{
    NK_WINDOW_FOCUS_FOCUSED          = 0x01,
    NK_WINDOW_FOCUS_UNFOCUSED        = 0x02
} nkWindowFocus_t;

/* General Window Events */
typedef void (*nkWindowResizeCallback_t)(void *window, float width, float height);
typedef void (*nkWindowDrawCallback_t)(void *window);
typedef void (*nkWindowCloseCallback_t)(void *window);
typedef void (*nkWindowVisibilityChangeCallback_t)(void *window, nkWindowVisibility_t visibility);
typedef void (*nkWindowFocusChangeCallback_t)(void *window, nkWindowFocus_t focus);

/* Pointer Events */
typedef void (*nkWindowPointerMoveCallback_t)(void *window, float x, float y);
typedef void (*nkWindowPointerActionBeginCallback_t)(void *window, nkPointerAction_t action, float x, float y);
typedef void (*nkWindowPointerActionEndCallback_t)(void *window, nkPointerAction_t action, float x, float y);
typedef void (*nkWindowScrollCallback_t)(void *window, float deltaX, float deltaY);

/* Keyboard Events */
typedef void (*nkWindowKeyDownCallback_t)(void *window, uint32_t keycode);
typedef void (*nkWindowKeyUpCallback_t)(void *window, uint32_t keycode);
typedef void (*nkWindowCodepointInputCallback_t)(void *window, uint32_t codepoint);

typedef struct nkWindow_t
{
    struct nkWindow_t *Next;

    const char *Title;
    float Width;
    float Height;

    nkWindowVisibility_t Visibility;
    nkWindowFocus_t Focus;

    nkWindowResizeCallback_t ResizeCallback;
    nkWindowDrawCallback_t DrawCallback;
    nkWindowCloseCallback_t CloseCallback;
    nkWindowVisibilityChangeCallback_t VisibilityChangeCallback;
    nkWindowFocusChangeCallback_t FocusChangeCallback;

    nkWindowPointerMoveCallback_t PointerMoveCallback;
    nkWindowPointerActionBeginCallback_t PointerActionBeginCallback;
    nkWindowPointerActionEndCallback_t PointerActionEndCallback;
    nkWindowScrollCallback_t ScrollCallback;

    nkWindowKeyDownCallback_t KeyDownCallback;
    nkWindowKeyUpCallback_t KeyUpCallback;
    nkWindowCodepointInputCallback_t CodepointInputCallback;

    #if _WIN32
        HWND WindowHandle;
        HINSTANCE InstanceHandle;
        HDC DrawingContext;
        HGLRC GLRenderContext;
        PAINTSTRUCT PaintStruct;
    #else 
        #error "Unsupported platform!"
    #endif
} nkWindow_t;

/***************************************************************
** MARK: FUNCTION DEFS
***************************************************************/

bool nkWindow_Create(nkWindow_t *window, const char *title, float width, float height); 
void nkWindow_SetTitle(nkWindow_t *window, const char *title);
void nkWindow_SetSize(nkWindow_t *window, float width, float height);
void nkWindow_SetVisibility(nkWindow_t *window, nkWindowVisibility_t visibility);
void nkWindow_SetFocus(nkWindow_t *window, nkWindowFocus_t focus);
void nkWindow_Destroy(nkWindow_t *window);
bool nkWindow_IsPointerActionDown(nkWindow_t *window, nkPointerAction_t action);
bool nkWindow_IsKeyDown(nkWindow_t *window, uint32_t keycode);

/* polls for events, returning true if application should stay open */
bool nkWindow_PollEvents(void);

#ifdef __cplusplus
}
#endif

#endif /* WINDOW_H */
