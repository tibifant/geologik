#pragma once

#include "core.h"
#include "vmath.h"

#ifdef _WIN32
#include "SDL.h"
#else
#include <SDL2/SDL.h>
#endif

struct lsKeyboardState
{
  const uint8_t *pKeys;
  size_t keyCount = 0;
  uint8_t currentKeys[512];
  uint8_t lastKeys[512];
  bool hasPreviousState;
};

void lsKeyboardState_Update(lsKeyboardState *pKeyboardState);
bool lsKeyboardState_IsKeyDown(const lsKeyboardState *pKeyboardState, const int32_t key);
bool lsKeyboardState_IsKeyUp(const lsKeyboardState *pKeyboardState, const int32_t key);
bool lsKeyboardState_KeyPress(const lsKeyboardState *pKeyboardState, const int32_t key);
bool lsKeyboardState_KeyLift(const lsKeyboardState *pKeyboardState, const int32_t key);

struct lsAppState;

struct lsAppView
{
  typedef lsResult(Update)(lsAppView *pSelf, lsAppView **ppNext, lsAppState *pAppState);
  typedef void(Destroy)(lsAppView **ppSelf, lsAppState *pAppState);

  Update *pUpdate;
  Destroy *pDestroy;
};

struct lsAppState
{
  SDL_Window *pWindow;
  SDL_GLContext glContext;
  vec2i32 mousePos, relativeMousePos;
  vec2s windowSize;
  SDL_Keycode key;
  bool rightMouseDown, leftMouseDown, keyDown, keyPressed, keyReleased, quit;
  lsKeyboardState keyboardState;

  lsAppView *pCurrentView;
};

lsResult lsAppState_Create(lsAppState *pAppState, const char *title, const vec2s size);
bool lsAppState_HandleWindowEvents(lsAppState *pAppState);
void lsAppState_Swap(lsAppState *pAppState);
