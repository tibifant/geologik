#include "platform.h"

#include "GL/glew.h"

#include <Windows.h>
#include <wingdi.h>

static bool _OnErrorCallbackAllowed = true;

void lsKeyboardState_Update(lsKeyboardState *pKeyboardState)
{
  if (pKeyboardState->pKeys != nullptr)
  {
    memcpy(pKeyboardState->lastKeys, pKeyboardState->currentKeys, pKeyboardState->keyCount);
    pKeyboardState->hasPreviousState = true;
  }

  int32_t keyCount = 0;
  pKeyboardState->pKeys = SDL_GetKeyboardState(&keyCount);

  if (keyCount > LS_ARRAYSIZE(pKeyboardState->currentKeys))
    keyCount = (int32_t)LS_ARRAYSIZE(pKeyboardState->currentKeys);

  pKeyboardState->keyCount = (size_t)keyCount;

  memcpy(pKeyboardState->currentKeys, pKeyboardState->pKeys, pKeyboardState->keyCount);
}

bool lsKeyboardState_IsKeyDown(const lsKeyboardState *pKeyboardState, const int32_t key)
{
  if (pKeyboardState->pKeys == nullptr || key >= pKeyboardState->keyCount || key >= LS_ARRAYSIZE(pKeyboardState->currentKeys) || key <= SDLK_UNKNOWN)
    return false;

  return pKeyboardState->pKeys[key] == SDL_TRUE;
}

bool lsKeyboardState_IsKeyUp(const lsKeyboardState *pKeyboardState, const int32_t key)
{
  if (pKeyboardState->pKeys == nullptr || key >= pKeyboardState->keyCount || key >= LS_ARRAYSIZE(pKeyboardState->currentKeys) || key <= SDLK_UNKNOWN)
    return false;

  return pKeyboardState->pKeys[key] == SDL_FALSE;
}

bool lsKeyboardState_KeyPress(const lsKeyboardState *pKeyboardState, const int32_t key)
{
  if (!pKeyboardState->hasPreviousState || key >= pKeyboardState->keyCount || key >= LS_ARRAYSIZE(pKeyboardState->currentKeys) || key <= SDLK_UNKNOWN)
    return false;

  return pKeyboardState->currentKeys[key] == SDL_TRUE && pKeyboardState->lastKeys[key] == SDL_FALSE;
}

bool lsKeyboardState_KeyLift(const lsKeyboardState *pKeyboardState, const int32_t key)
{
  if (!pKeyboardState->hasPreviousState || key >= pKeyboardState->keyCount || key >= LS_ARRAYSIZE(pKeyboardState->currentKeys) || key <= SDLK_UNKNOWN)
    return false;

  return pKeyboardState->currentKeys[key] == SDL_FALSE && pKeyboardState->lastKeys[key] == SDL_TRUE;
}

lsResult lsAppState_Create(lsAppState *pAppState, const char *title, const vec2s size)
{
  lsResult result = lsR_Success;

  pAppState->pWindow = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int32_t)size.x, (int32_t)size.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
  LS_ERROR_IF(pAppState->pWindow == nullptr, lsR_InternalError);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  pAppState->windowSize = size;
  pAppState->glContext = SDL_GL_CreateContext(pAppState->pWindow);
  LS_ERROR_IF(pAppState->glContext == nullptr, lsR_InternalError);

  glewExperimental = GL_TRUE;
  LS_ERROR_IF(glewInit() != GL_NO_ERROR, lsR_InternalError);

  printf("GPU: %s %s\nDriver Version: %s\nGLSL Version: %s\nGLEW: %s\n", glGetString(GL_VENDOR), glGetString(GL_RENDERER), glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION), glewGetString(GLEW_VERSION));

  SDL_GL_SetSwapInterval(1);

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);

  // Set Maxiumum Depth Precision.
  {
    const HDC hdc = wglGetCurrentDC();
    int32_t pixelFormatCount = 1;
    size_t maxDepthBits = 0;

    for (size_t i = 1; i <= pixelFormatCount; i++)
    {
      PIXELFORMATDESCRIPTOR pixelFormat = { 0 };

      pixelFormatCount = DescribePixelFormat(hdc, (int32_t)i, sizeof(PIXELFORMATDESCRIPTOR), &pixelFormat);
      LS_ERROR_IF(pixelFormatCount == 0, lsR_OperationNotSupported);

      if (pixelFormat.cDepthBits > maxDepthBits && pixelFormat.cBlueBits >= 8 && pixelFormat.cRedBits >= 8 && pixelFormat.cGreenBits >= 8)
        maxDepthBits = (size_t)pixelFormat.cDepthBits;
    }

    LS_ERROR_IF(0 != SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, (int32_t)maxDepthBits), lsR_OperationNotSupported);
  }

#ifdef _DEBUG
  static std::function<lsResult (const GLenum source, const GLenum type, const GLuint id, const GLenum severity, const GLsizei length, const char *msg)> _Callback = [](const GLenum source, const GLenum type, const GLuint id, const GLenum severity, const GLsizei /* lenght */, const char *msg)
  {
    if (type != GL_DEBUG_TYPE_ERROR || type != GL_DEBUG_TYPE_ERROR_ARB)
      return lsR_Success;

    puts("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\nGL ERROR CALLBACK TRIGGERD!");
    printf("source: 0x %" PRIX64 " (", (uint64_t)source);

    switch (source)
    {
    case GL_DEBUG_SOURCE_API: printf("GL_DEBUG_SOURCE_API"); break;
    case GL_DEBUG_SOURCE_APPLICATION: printf("GL_DEBUG_SOURCE_APPLICATION"); break;
    case GL_DEBUG_SOURCE_OTHER: printf("GL_DEBUG_SOURCE_OTHER"); break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: printf("GL_DEBUG_SOURCE_SHADER_COMPILER"); break;
    case GL_DEBUG_SOURCE_THIRD_PARTY: printf("GL_DEBUG_SOURCE_THIRD_PARTY"); break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM: printf("GL_DEBUG_SOURCE_WINDOW_SYSTEM"); break;
    default: printf("<unknown>"); break;
    }

    printf(")\ntype: 0x%" PRIX64 " (", (uint64_t)type);

    switch (type)
    {
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: printf("GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"); break;
    case GL_DEBUG_TYPE_ERROR: printf("GL_DEBUG_TYPE_ERROR"); break;
    case GL_DEBUG_TYPE_MARKER: printf("GL_DEBUG_TYPE_MARKER"); break;
    case GL_DEBUG_TYPE_OTHER: printf("GL_DEBUG_TYPE_OTHER"); break;
    case GL_DEBUG_TYPE_PERFORMANCE: printf("GL_DEBUG_TYPE_PERFORMANCE"); break;
    case GL_DEBUG_TYPE_POP_GROUP: printf("GL_DEBUG_TYPE_POP_GROUP"); break;
    case GL_DEBUG_TYPE_PORTABILITY: printf("GL_DEBUG_TYPE_PORTABILITY"); break;
    case GL_DEBUG_TYPE_PUSH_GROUP: printf("GL_DEBUG_TYPE_PUSH_GROUP"); break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: printf("GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"); break;
    default: printf("<unknown>"); break;
    }

    printf(")\nid: 0x%" PRIX64 " (", (uint64_t)id);

    switch (id)
    {
    case GL_INVALID_ENUM: printf("GL_INVALID_ENUM"); break;
    case GL_INVALID_VALUE: printf("GL_INVALID_VALUE"); break;
    case GL_INVALID_OPERATION: printf("GL_INVALID_OPERATION"); break;
    case GL_STACK_OVERFLOW: printf("GL_STACK_OVERFLOW"); break;
    case GL_STACK_UNDERFLOW: printf("GL_STACK_UNDERFLOW"); break;
    case GL_OUT_OF_MEMORY: printf("GL_OUT_OF_MEMORY"); break;
    case GL_INVALID_FRAMEBUFFER_OPERATION: printf("GL_INVALID_FRAMEBUFFER_OPERATION"); break;
    case GL_CONTEXT_LOST: printf("GL_CONTEXT_LOST"); break;
    case GL_TABLE_TOO_LARGE: printf("GL_TABLE_TOO_LARGE"); break;
    default: printf("<unknown>"); break;
    }

    printf(")\nseverity: 0x%" PRIX64 " (", (uint64_t)severity);

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH: printf("GL_DEBUG_SEVERITY_HIGH"); break;
    case GL_DEBUG_SEVERITY_LOW: printf("GL_DEBUG_SEVERITY_LOW"); break;
    case GL_DEBUG_SEVERITY_MEDIUM: printf("GL_DEBUG_SEVERITY_MEDIUM"); break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: printf("GL_DEBUG_SEVERITY_NOTIFICATION"); break;
    default: printf("<unknown>"); break;
    }

    printf(")\nmessage: '%s'\n", msg);

    BOOL remoteDebuggerPresent = false;

    if (!CheckRemoteDebuggerPresent(GetCurrentProcess(), &remoteDebuggerPresent))
      remoteDebuggerPresent = false;

    if (IsDebuggerPresent() || remoteDebuggerPresent)
      __debugbreak();

    return lsR_Success;
  };

  struct _internal
  {
    static void GLAPIENTRY _ErrorMessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *)
    {
      if (!_OnErrorCallbackAllowed)
        return;

      _Callback(source, type, id, severity, length, message);
    }

    static void GLAPIENTRY _ErrorMessageCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar *message, void *)
    {
      if (!_OnErrorCallbackAllowed)
        return;

      _Callback(0, category, id, severity, length, message);
    }

    static void GLAPIENTRY _ErrorMessageCallbackARB(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *)
    {
      if (!_OnErrorCallbackAllowed)
        return;

      _Callback(source, type, id, severity, length, message);
    }
  };

  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

  glDebugMessageCallback(_internal::_ErrorMessageCallback, nullptr);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

  if (glDebugMessageCallbackAMD)
  {
    glDebugMessageCallbackAMD(_internal::_ErrorMessageCallbackAMD, nullptr);

    if (glDebugMessageEnableAMD)
      glDebugMessageEnableAMD(GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  }

  if (glDebugMessageCallbackARB)
    glDebugMessageCallbackARB(_internal::_ErrorMessageCallbackARB, nullptr);

  if (glGetError != nullptr)
  {
    size_t count = 0;

    while (GL_NO_ERROR != glGetError() && count++ < 128) {}
  }
#endif

  goto epilogue;
epilogue:
  return result;
}

bool lsAppState_HandleWindowEvents(lsAppState *pAppState)
{
  lsKeyboardState_Update(&pAppState->keyboardState);

  pAppState->keyReleased = false;
  pAppState->keyPressed = false;

  SDL_Event event;

  while (SDL_PollEvent(&event))
  {
    switch (event.type)
    {
    case SDL_KEYDOWN:
    {
      pAppState->keyDown = true;
      pAppState->keyPressed = true;
      pAppState->key = event.key.keysym.scancode;

      if (pAppState->key != SDL_SCANCODE_ESCAPE)
        break;

#ifndef _MSC_VER
      __attribute__((fallthrough));
#endif
    }

    case SDL_QUIT:
    {
      pAppState->quit = true;
      return false;
    }

    case SDL_KEYUP:
    {
      pAppState->keyDown = false;
      pAppState->keyReleased = true;
      break;
    }

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
      const bool setButton = (event.type == SDL_MOUSEBUTTONDOWN);

      switch (event.button.button)
      {
      case SDL_BUTTON_LEFT:
        pAppState->leftMouseDown = setButton;
        break;

      case SDL_BUTTON_RIGHT:
        pAppState->rightMouseDown = setButton;
        break;
      }

      break;
    }

    case SDL_MOUSEMOTION:
    {
      pAppState->mousePos = vec2i32(event.motion.x, event.motion.y);
      pAppState->relativeMousePos = vec2i32(event.motion.xrel, event.motion.yrel);
      break;
    }

    case SDL_WINDOWEVENT:
    {
      if (event.window.event == SDL_WINDOWEVENT_MAXIMIZED || event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || event.window.event == SDL_WINDOWEVENT_RESTORED)
      {
        int32_t x, y;
        SDL_GetWindowSize(pAppState->pWindow, &x, &y);
        pAppState->windowSize = vec2s(x, y);
      }

      break;
    }
    }
  }

  return true;
}

void lsAppState_Swap(lsAppState *pAppState)
{
  SDL_GL_SwapWindow(pAppState->pWindow);
}

