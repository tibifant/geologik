#include "render.h"

#include "core.h"
#include "pool.h"
#include "texture.h"
#include "vertexBuffer.h"
#include "framebuffer.h"
#include "shader.h"
#include "dataBlob.h"

//////////////////////////////////////////////////////////////////////////

extern const char _Attrib_Pos[] = "position";
extern const char _Attrib_Offs[] = "offset";
extern const char _Attrib_Normal[] = "normal";
extern const char _Attrib_TexCoord[] = "texCoord";
extern const char _Attrib_Color[] = "color";
extern const char _Attrib_Matrix[] = "matrix";
extern const char _Attrib_Rot[] = "rotation";

//////////////////////////////////////////////////////////////////////////

static struct
{
  struct
  {
    shader shader;
    vertexBuffer<vb_attribute_float<2, _Attrib_Pos>> buffer;
  } plane;

  pool<texture> textures;
  vec3f lookAt, up, cameraDistance;
  matrix vp, vpFar;
  vec2s windowSize;
  float_t frameRatio;
  int64_t lastFrameStartNs;
  float_t ticksSinceOrigin;
} _Render;

//////////////////////////////////////////////////////////////////////////

lsResult render_init(lsAppState *pAppState)
{
  lsResult result = lsR_Success;

  _Render.windowSize = pAppState->windowSize;
  _Render.cameraDistance = vec3f(0, 0, 5.f);
  render_setLookAt(vec2f(0), vec2f(0, 1));
  _Render.lastFrameStartNs = lsGetCurrentTimeNs();

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

  render_clearColor(vec4f(0, 0, 0, 1));
  render_clearDepth();

  //framebuffer_setResolution(&_Render.screenQuad.framebuffer, _Render.windowSize);
  //framebuffer_setResolution(&_Render.screenQuad.tiny, _Render.windowSize / 16);
  //framebuffer_bind(&_Render.screenQuad.framebuffer);
  
  render_clearColor(vec4f(0, 0, 0, 1));
  render_clearDepth();
  
  render_setDepthMode(rCR_Less);
  render_setBlendEnabled(false);
  render_setDepthTestEnabled(false);

  render_setLookAt(_Render.lookAt, _Render.up);
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

  vertexBuffer_destroy(&_Render.plane.buffer);
  shader_destroy(&_Render.plane.shader);
}

void render_setCameraOffset(const vec3f offset)
{
  _Render.cameraDistance = offset;
  render_setLookAt(_Render.lookAt, _Render.up);
}

void render_setLookAt(const vec2f position, const vec2f up)
{
  render_setLookAt(vec3f(position, 0), vec3f(up, 0));
}

void render_setLookAt(const vec3f position, const vec3f up)
{
  _Render.lookAt = position;
  _Render.up = up.Normalize();
  
  const matrix v = matrix::LookAtLH(vec(_Render.lookAt - _Render.cameraDistance), vec(_Render.lookAt), vec(_Render.up));

  _Render.vp = v * matrix::PerspectiveFovLH(lsHALFPIf, vec2f(_Render.windowSize).AspectRatio(), 1, 50);
  _Render.vpFar = v * matrix::PerspectiveFovLH(lsHALFPIf, vec2f(_Render.windowSize).AspectRatio(), 10, 1000);
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

void render_flushRenderQueue()
{

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
