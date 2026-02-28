#include "platform.h"
#include "render.h"
#include "terrain.h"

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

  uint16_t width = 1024;

  LS_ERROR_CHECK(lsAppState_Create(&_AppState, "Engine", vec2s(1600, 1200)));
  
  if (argc == 2)
  {
    terrain t;
    LS_ERROR_CHECK(terrain_init_from_file(pArgs[1], &t));
    LS_ERROR_CHECK(render_init(&_AppState, &t));
    terrain_destroy(&t);
  }
  else if (argc == 1)
  {
    terrain t;
    LS_ERROR_CHECK(terrain_init(&t, width));
    LS_ERROR_CHECK(terrain_generate(&t));
    LS_ERROR_CHECK(render_init(&_AppState, &t));
    terrain_destroy(&t);
  }
  else
  {
    lsFail();
  }

  {
    const float_t updateTimeMs = 1000.0f / 120.f;
    size_t frameCount = 0;
    float_t frameTimesMs = 0;
    float_t cpuTimesMs = 0;

    while (lsAppState_HandleWindowEvents(&_AppState))
    {
      if (lsKeyboardState_IsKeyDown(&_AppState.keyboardState, SDL_SCANCODE_LCTRL) && lsKeyboardState_KeyPress(&_AppState.keyboardState, SDL_SCANCODE_F10))
        LS_ERROR_CHECK(render_writeTerrainToFile());

      const int64_t before = lsGetCurrentTimeNs();

#ifdef _DEBUG
      if (lsKeyboardState_IsKeyDown(&_AppState.keyboardState, SDL_SCANCODE_F8))
      {
        print("Reloading shader...\n");
        system("xcopy /Q /E /Y /I ..\\geologik\\assets\\shaders shaders"); // horrible way to copy shaders over to builds/bin if we haven't recompiled.
        render_reload_shader();
      }
#endif

      {
        render_startFrame(&_AppState);
        render_update(&_AppState);
        render_computeTerrain(&_AppState);
        render_drawScene();
        render_endFrame(&_AppState);
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
  }

  goto epilogue;
epilogue:

  render_destroy();

  return result;
}
