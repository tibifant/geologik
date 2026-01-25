#include "render.h"

#include "core.h"
#include "pool.h"
#include "texture.h"
#include "vertexBuffer.h"
#include "gpuBuffer.h"
#include "framebuffer.h"
#include "shader.h"
#include "dataBlob.h"
#include "terrain.h"

//////////////////////////////////////////////////////////////////////////

extern const char _Attrib_Pos[] = "position";
extern const char _Attrib_Offs[] = "offset";
extern const char _Attrib_Normal[] = "normal";
extern const char _Attrib_TexCoord[] = "texCoord";
extern const char _Attrib_Color[] = "color";
extern const char _Attrib_Matrix[] = "matrix";
extern const char _Attrib_Rot[] = "rotation";

//////////////////////////////////////////////////////////////////////////

struct camera_3d
{
  matrix view;
  matrix projection;

  matrix viewProjection;
};

struct camera_3d_free_floating : camera_3d
{
  vec3f position;
  vec3f direction;
};

void camera_3d_free_floating_create(camera_3d_free_floating &cam, const vec3f position, const vec3f direction, const vec2f windowSize);
void camera_3d_free_floating_move(camera_3d_free_floating &cam, const vec3f dir);
void camera_3d_free_floating_rotate(camera_3d_free_floating &cam, const vec2f dir);
void camera_3d_free_floating_set_rotation(camera_3d_free_floating &cam, const vec2f dir);
void camera_3d_free_floating_update(camera_3d_free_floating &cam);

//////////////////////////////////////////////////////////////////////////

static struct
{
  struct
  {
    shader computeShader;
    gpu_buffer gpuBuffer;
  } erosion;

  struct
  {
    shader renderShader;
    vertexBuffer<vb_attribute_uint<2, _Attrib_Pos>> buffer;
  } terrain;

  struct
  {
    shader shader;
    vertexBuffer<vb_attribute_float<2, _Attrib_Pos>> buffer;
  } plane;

  pool<texture> textures;
  camera_3d_free_floating camera;
  matrix vp;
  vec2s windowSize;
  float_t frameRatio;
  int64_t lastFrameStartNs;
  float_t ticksSinceOrigin;
} _Render;

//////////////////////////////////////////////////////////////////////////

void camera_3d_free_floating_create(camera_3d_free_floating &cam, const vec3f position, const vec3f direction, const vec2f windowSize)
{
  cam.position = position;
  cam.direction = direction;

  cam.projection = matrix::PerspectiveFovRH(lsHALFPIf * 0.75f, windowSize.AspectRatio(), 0.001f, 1024 * 8.f);
  camera_3d_free_floating_update(cam);
}

void camera_3d_free_floating_move(camera_3d_free_floating &cam, const vec3f dir)
{
  const vec3f v = (vec3f)(cam.view.TransformVector3(dir));
  cam.position += v;
}

void camera_3d_free_floating_rotate(camera_3d_free_floating &cam, const vec2f dir)
{
  cam.direction = (matrix::RotationX(dir.y) * matrix::RotationZ(dir.x)).TransformVector3(cam.direction);
}

void camera_3d_free_floating_set_rotation(camera_3d_free_floating &cam, const vec2f dir)
{
  cam.direction = (matrix::RotationX(dir.y) * matrix::RotationZ(dir.x)).TransformVector3(vec3f(0, 1, 0));
}

void camera_3d_free_floating_update(camera_3d_free_floating &cam)
{
  cam.view = matrix::LookToRH(cam.position, cam.direction, vec3f(0, 0, -1));
  cam.viewProjection = cam.view * cam.projection;
}

//////////////////////////////////////////////////////////////////////////

constexpr size_t quadCountX = 128;
constexpr size_t quadCountY = 128;

lsResult set_terrain_vertexData()
{
  lsResult result = lsR_Success;

  vec2u32 quadData[] = { vec2u32(0, 0), vec2u32(0, 1), vec2u32(1, 0), vec2u32(0, 1), vec2u32(1, 1), vec2u32(1, 0) };
  constexpr size_t quadDataSize = LS_ARRAYSIZE(quadData);
  static vec2u32 renderData[quadCountX * quadCountY * quadDataSize];

  for (uint32_t y = 0; y < quadCountY; y++)
  {
    for (uint32_t x = 0; x < quadCountX; x++)
    {
      const size_t idx = (y * quadCountX + x) * quadDataSize;

      for (size_t i = 0; i < quadDataSize; i++)
        renderData[idx + i] = quadData[i] + vec2u32(x, y);
    }
  }

  LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.terrain.buffer, renderData, LS_ARRAYSIZE(renderData)));

epilogue:
  return result;
}

lsResult render_init(lsAppState *pAppState, const size_t terrainWidth)
{
  lsResult result = lsR_Success;

  _Render.windowSize = pAppState->windowSize;
  camera_3d_free_floating_create(_Render.camera, vec3f(1, 1, 0), vec3f(1, 1, 1).Normalize(), vec2f(_Render.windowSize));
  render_updateCamera(pAppState);
  _Render.lastFrameStartNs = lsGetCurrentTimeNs();

  // Create Erosion Buffer & Shader.
  {
    terrain t;

    LS_ERROR_CHECK(terrain_init(&t, (uint16_t)terrainWidth));
    LS_ERROR_CHECK(terrain_generate(&t));

    LS_ERROR_CHECK(gpuBuffer_create(&_Render.erosion.gpuBuffer));
    LS_ERROR_CHECK(gpuBuffer_set(&_Render.erosion.gpuBuffer, t.pTiles, t.width * t.width));

    terrain_destroy(&t);

    //LS_ERROR_CHECK(shader_createFromFile_compute(&_Render.erosion.computeShader, "shaders/erosion.comp"));
  }

  // Create Terrain.
  {
    LS_ERROR_CHECK(shader_createFromFile_vertex_fragment(&_Render.terrain.renderShader, "shaders/terrain.vert", "shaders/terrain.frag"));

    LS_ERROR_CHECK(vertexBuffer_create(&_Render.terrain.buffer, &_Render.terrain.renderShader));
    LS_ERROR_CHECK(set_terrain_vertexData());
  }

  // Create Plane.
  {
    LS_ERROR_CHECK(shader_createFromFile_vertex_fragment(&_Render.plane.shader, "shaders/plane.vert", "shaders/plane.frag"));

    float_t renderData[] = { 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 };
    LS_ERROR_CHECK(vertexBuffer_create(&_Render.plane.buffer, &_Render.plane.shader));
    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.plane.buffer, renderData, LS_ARRAYSIZE(renderData)));
  }

  // Create Default Texture.
  {
    texture defaultTexture;
    LS_ERROR_CHECK(texture_create(&defaultTexture, true));

    const uint32_t textureData[] = { 0xFF333333, 0xFFAAAAAA, 0xFFAAAAAA, 0xFF333333 };
    LS_ERROR_CHECK(texture_set(&defaultTexture, reinterpret_cast<const uint8_t *>(textureData), vec2s(2)));

    size_t _unused;
    LS_ERROR_CHECK(pool_add(&_Render.textures, defaultTexture, &_unused));
  }

  // Load textures.
  {
    texture tex;

    //LS_ERROR_CHECK(texture_create(&tex, "textures/particletex.png"));
    //LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_spaceshipExhaustColorRamp));
    //lsZeroMemory(&tex);
  }

epilogue:
  return result;
}

void render_startFrame(lsAppState *pAppState)
{
  const int64_t now = lsGetCurrentTimeNs();
  _Render.frameRatio = (float_t)(now - _Render.lastFrameStartNs) / (1e9f / 60.f);
  _Render.lastFrameStartNs = now;
  _Render.windowSize = pAppState->windowSize;

  SDL_GL_MakeCurrent(pAppState->pWindow, pAppState->glContext);
  glViewport(0, 0, (GLsizei)pAppState->windowSize.x, (GLsizei)pAppState->windowSize.y);

  render_clearColor(vec4f(0.5f, 0.7f, 0.9f, 1));
  render_clearDepth();

  render_setDepthMode(rCR_Less);
  render_setBlendEnabled(false);
  render_setDepthTestEnabled(true);
}

void render_endFrame(lsAppState *pAppState)
{
  (void)pAppState;

  framebuffer_unbind();

  render_setDepthTestEnabled(false);
}

void render_destroy()
{
  for (auto _item : _Render.textures)
    texture_destroy(_item.pItem);

  pool_destroy(&_Render.textures);

  vertexBuffer_destroy(&_Render.terrain.buffer);
  shader_destroy(&_Render.terrain.renderShader);

  gpuBuffer_detroy(&_Render.erosion.gpuBuffer);
  shader_destroy(&_Render.erosion.computeShader);

  vertexBuffer_destroy(&_Render.plane.buffer);
  shader_destroy(&_Render.plane.shader);
}

void render_setTicksSinceOrigin(const float_t ticksSinceOrigin)
{
  _Render.ticksSinceOrigin = ticksSinceOrigin;
}

void render_drawQuad(const matrix &model, const render_textureId textureIndex)
{
  texture *pTex = pool_get(&_Render.textures, textureIndex);
  texture_bind(pTex, 0);
  shader_bind(&_Render.plane.shader);
  shader_setUniform(&_Render.plane.shader, "texture", pTex);
  shader_setUniform(&_Render.plane.shader, "matrix", model);
  vertexBuffer_render(&_Render.plane.buffer);
}

void render_draw2DQuad(const matrix &model, const render_textureId textureIndex)
{
  render_drawQuad(model * matrix::Scale(2.f / _Render.windowSize.x, 2.f / _Render.windowSize.y, 0) * matrix::Translation(-1.f, -1.f, 0) * matrix::Scale(1, -1, 0), textureIndex);
}

void render_draw3DQuad(const matrix &model, const render_textureId textureIndex)
{
  render_drawQuad(model * _Render.vp, textureIndex);
}

void render_drawTerrain(const uint32_t width)
{
  shader_bind(&_Render.terrain.renderShader);
  shader_setUniform(&_Render.terrain.renderShader, "width", width);
  shader_setUniform(&_Render.terrain.renderShader, "vp", _Render.vp.Transpose());

  for (uint32_t y = 0; y < width; y += quadCountY)
  {
    for (uint32_t x = 0; x < width; x += quadCountX)
    {
      shader_setUniform(&_Render.terrain.renderShader, "offset", vec2u32(x, y));
      vertexBuffer_render(&_Render.terrain.buffer);
    }
  }
}

void render_updateCamera(lsAppState *pAppState)
{
  vec3f movementDir = vec3f(0);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_W))
    movementDir = vec3f(0, 1.f, 0);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_A))
    movementDir += vec3f(-1.f, 0, 0);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_S))
    movementDir += vec3f(0, -1.f, 0);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_D))
    movementDir += vec3f(1.f, 0, 0);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_R))
    movementDir += vec3f(0, 0, 1.f);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_F))
    movementDir += vec3f(0, 0, -1.f);

  if (movementDir != vec3f(0))
    camera_3d_free_floating_move(_Render.camera, movementDir);

  vec2f rotationDir = (((vec2f)(pAppState->mousePos) - vec2f(pAppState->windowSize) * 0.5) / vec2f(pAppState->windowSize)) * vec2f(lsTWOPIf, lsPIf);
  camera_3d_free_floating_set_rotation(_Render.camera, rotationDir);

  camera_3d_free_floating_update(_Render.camera);
  _Render.vp = _Render.camera.viewProjection;
}

//////////////////////////////////////////////////////////////////////////

void render_clearColor(const vec4f color)
{
  glClearColor(color.x, color.y, color.z, color.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_clearDepth()
{
  glClear(GL_DEPTH_BUFFER_BIT);
}

void render_setDepthMode(const render_comparisonResult mode)
{
  GLenum param;

  switch (mode)
  {
  case rCR_Less: param = GL_LESS; break;
  case rCR_LessOrEqual: param = GL_LEQUAL; break;
  case rCR_Equal: param = GL_EQUAL; break;
  case rCR_GreaterOrEqual: param = GL_GEQUAL; break;
  case rCR_Greater: param = GL_GREATER; break;
  case rCR_NotEqual: param = GL_NOTEQUAL; break;
  case rCR_Always: param = GL_ALWAYS; break;
  default: return;
  }

  glDepthFunc(param);
}

void render_setDepthTestEnabled(const bool enabled)
{
  (enabled ? glEnable : glDisable)(GL_DEPTH_TEST);
}

void render_setBlendMode(const render_blendFunc mode)
{
  switch (mode)
  {
  case rBF_Additive:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBlendEquation(GL_FUNC_ADD);
    break;

  case rBF_AlphaBlend:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    break;

  case rBF_Premultiplied:
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    break;

  case rBF_Override:
    glBlendFunc(GL_ONE, GL_ZERO);
    glBlendEquation(GL_FUNC_ADD);
    break;

  case rBF_AlphaMask:
    glBlendFuncSeparate(GL_ZERO, GL_ONE, GL_ZERO, GL_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);
    break;

  default:
    return;
  }
}

void render_setBlendEnabled(const bool enabled)
{
  (enabled ? glEnable : glDisable)(GL_BLEND);
}

void render_finalize()
{
  glFlush();
  glFinish();
}
