#include "platform.h"
#include "render.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////

static lsAppState _AppState = { };

lsResult MainGameLoop(int32_t argc, const char **pArgs);

//////////////////////////////////////////////////////////////////////////

int32_t main(int32_t argc, char **pArgv)
{
  return LS_SUCCESS(MainGameLoop(argc, const_cast<const char **>(pArgv))) ? EXIT_SUCCESS : EXIT_FAILURE;
}

//////////////////////////////////////////////////////////////////////////

lsResult MainGameLoop(int32_t argc, const char **pArgs)
{
  lsResult result = lsR_Success;

  (void)argc;
  (void)pArgs;

  LS_ERROR_CHECK(lsAppState_Create(&_AppState, "Engine", vec2s(1600, 1200)));
  // TODO terrain init

  LS_ERROR_CHECK(render_init(&_AppState));

  const float_t updateTimeMs = 1000.0f / 120.f;
  size_t frameCount = 0;
  float_t frameTimesMs = 0;
  float_t cpuTimesMs = 0;

  while (lsAppState_HandleWindowEvents(&_AppState))
  {
    const int64_t before = lsGetCurrentTimeNs();

    lsAppView *pNext = _AppState.pCurrentView;

    LS_ERROR_CHECK(_AppState.pCurrentView->pUpdate(_AppState.pCurrentView, &pNext, &_AppState));

    if (pNext != nullptr && pNext != _AppState.pCurrentView)
    {
      _AppState.pCurrentView->pDestroy(&_AppState.pCurrentView, &_AppState);
      lsAssert(_AppState.pCurrentView == nullptr);

      _AppState.pCurrentView = pNext;
    }

    const int64_t afterCPU = lsGetCurrentTimeNs();

    render_finalize();

    const int64_t afterRender = lsGetCurrentTimeNs();

    lsAppState_Swap(&_AppState);

    const float_t ms = (afterRender - before) * 1e-6f;
    const int64_t sleepMs = (int64_t)floorf(updateTimeMs - ms - 0.5f /* if vsync, leave vsync 0.5ms to play with */);
    
    if (sleepMs > 0)
      Sleep((DWORD)sleepMs);

    frameTimesMs += ms;
    cpuTimesMs += (afterCPU - before) * 1e-6f;
    frameCount++;

    if (frameCount >= 1000)
    {
      printf("Render tFPS: ~ %7.3f (~ %6.2f ms) | CPU tFPS: ~%7.3f (%6.2f ms)\n", frameCount / (frameTimesMs * 0.001), frameTimesMs / frameCount, frameCount / (cpuTimesMs * 0.001), cpuTimesMs / frameCount);
      frameTimesMs = 0;
      cpuTimesMs = 0;
      frameCount = 0;
    }
  }

  goto epilogue;
epilogue:
  if (_AppState.pCurrentView)
    _AppState.pCurrentView->pDestroy(&_AppState.pCurrentView, &_AppState);
  
  render_destroy();

  return result;
}
