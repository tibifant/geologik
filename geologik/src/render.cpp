#include "render.h"

#include "pool.h"
#include "texture.h"
#include "vertexBuffer.h"
#include "framebuffer.h"
#include "shader.h"
#include "objReader.h"
#include "dataBlob.h"
#include "game.h"

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

  struct
  {
    shader shader;
    vertexBuffer<vb_attribute_float<2, _Attrib_Pos>> buffer;
  } numbers;

  struct
  {
    shader shader;
    vertexBuffer<vb_attribute_float<3, _Attrib_Pos>, vb_attribute_float<3, _Attrib_Normal>, vb_attribute_float<2, _Attrib_TexCoord>> buffer;
    texture texture;
  } spaceship;
  
  struct
  {
    vertexBuffer<vb_attribute_float<3, _Attrib_Pos>, vb_attribute_float<3, _Attrib_Normal>, vb_attribute_float<2, _Attrib_TexCoord>> buffer;
    texture texture;
  } projectile;

  struct
  {
    shader shader;
    instancedVertexBuffer<vertexBuffer<vb_attribute_float<3, _Attrib_Pos>, vb_attribute_float<3, _Attrib_Normal>, vb_attribute_float<2, _Attrib_TexCoord>>, vb_attribute_mat4<_Attrib_Matrix>, vb_attribute_float<3, _Attrib_Rot>> buffer[4];
    dataBlob instanceData[4];
    texture normal[4];
  } asteroid;

  struct
  {
    shader shader;
    instancedVertexBuffer<vertexBuffer<vb_attribute_float<3, _Attrib_Pos>, vb_attribute_float<3, _Attrib_Normal>>, vb_attribute_mat4<_Attrib_Matrix>, vb_attribute_float<3, _Attrib_Rot>> buffer;
    dataBlob instanceData;
    texture reflectionTexture;
  } copper;

  struct
  {
    shader shader;
    instancedVertexBuffer<vertexBuffer<vb_attribute_float<3, _Attrib_Pos>, vb_attribute_float<3, _Attrib_Normal>, vb_attribute_float<2, _Attrib_TexCoord>>, vb_attribute_mat4<_Attrib_Matrix>, vb_attribute_float<3, _Attrib_Rot>> buffer[2 + 4 + 4];
    dataBlob instanceData[2 + 4 + 4];
    texture aoTexture[LS_ARRAYSIZE_C_STYLE(buffer)];
    texture reflectionTexture;
  } salt;

  struct
  {
    shader shader;
    instancedVertexBuffer<vertexBuffer<vb_attribute_float<2, _Attrib_Pos>>, vb_attribute_float<2, _Attrib_Offs>, vb_attribute_float<1, _Attrib_TexCoord>> buffer;
  } particles;

  struct
  {
    shader shader, blurShader;
    vertexBuffer<vb_attribute_float<3, _Attrib_Pos>> buffer;
    texture gradient;
    matrix lastVP;
    vertexBuffer<vb_attribute_float<2, _Attrib_Pos>> blurBuffer;
    framebuffer framebuffer;
    texture bokeh;
    float_t rotationZ;
  } stars;

  struct
  {
    shader shader;
    vertexBuffer<vb_attribute_float<3, _Attrib_Pos>, vb_attribute_float<2, _Attrib_TexCoord>> buffer;
  } galaxy;

  struct
  {
    shader shader, blurShader;
    vertexBuffer<vb_attribute_float<2, _Attrib_Pos>> buffer, blurBuffer;
    framebuffer framebuffer, tiny;
  } screenQuad;

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

  // Create Screen Quad.
  {
    LS_ERROR_CHECK(shader_createFromFile(&_Render.screenQuad.shader, "shaders/screen.vert", "shaders/screen.frag"));
    LS_ERROR_CHECK(shader_createFromFile(&_Render.screenQuad.blurShader, "shaders/blur.vert", "shaders/blur.frag"));

    float_t renderData[] = { 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0 };
    LS_ERROR_CHECK(vertexBuffer_create(&_Render.screenQuad.buffer, &_Render.screenQuad.shader));
    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.screenQuad.buffer, renderData, LS_ARRAYSIZE(renderData)));
    LS_ERROR_CHECK(vertexBuffer_create(&_Render.screenQuad.blurBuffer, &_Render.screenQuad.blurShader));
    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.screenQuad.blurBuffer, renderData, LS_ARRAYSIZE(renderData)));

    LS_ERROR_CHECK(framebuffer_create(&_Render.screenQuad.framebuffer, _Render.windowSize, 8, true));
    LS_ERROR_CHECK(framebuffer_create(&_Render.screenQuad.tiny, _Render.windowSize, 0, false));
  }

  // Create Plane.
  {
    LS_ERROR_CHECK(shader_createFromFile(&_Render.plane.shader, "shaders/plane.vert", "shaders/plane.frag"));

    float_t renderData[] = { 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 };
    LS_ERROR_CHECK(vertexBuffer_create(&_Render.plane.buffer, &_Render.plane.shader));
    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.plane.buffer, renderData, LS_ARRAYSIZE(renderData)));
  }

  // Numbers
  {
    LS_ERROR_CHECK(shader_createFromFile(&_Render.numbers.shader, "shaders/numbers.vert", "shaders/numbers.frag"));

    float_t renderData[] = { 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0 };
    LS_ERROR_CHECK(vertexBuffer_create(&_Render.numbers.buffer, &_Render.numbers.shader));
    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.numbers.buffer, renderData, LS_ARRAYSIZE(renderData)));
  }

  // Load Spaceship.
  {
    LS_ERROR_CHECK(shader_createFromFile(&_Render.spaceship.shader, "shaders/spaceship.vert", "shaders/spaceship.frag"));

    LS_ERROR_CHECK(vertexBuffer_create(&_Render.spaceship.buffer, &_Render.spaceship.shader));

    objInfo info;
    LS_ERROR_CHECK(objReader_loadFromFile(&info, "models/spaceship.obj"));

    dataBlob verts;

    for (size_t i = 0; i < info.triangles.count; i++)
    {
      auto &tri = queue_get(&info.triangles, i);

      for (size_t j = 0; j < 3; j++)
      {
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].position.asArray, 3));
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].normal.asArray, 3));
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].texCoord.asArray, 2));
      }
    }

    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.spaceship.buffer, verts.pData, verts.size));

    dataBlob_destroy(&verts);
    queue_destroy(&info.triangles);
    queue_destroy(&info.vertices);

    LS_ERROR_CHECK(texture_create(&_Render.spaceship.texture, "models/spaceship.png"));
  }

  // Load Projectile.
  {
    LS_ERROR_CHECK(vertexBuffer_create(&_Render.projectile.buffer, &_Render.spaceship.shader));

    objInfo info;
    LS_ERROR_CHECK(objReader_loadFromFile(&info, "models/projectile.obj"));

    dataBlob verts;

    for (size_t i = 0; i < info.triangles.count; i++)
    {
      auto &tri = queue_get(&info.triangles, i);

      for (size_t j = 0; j < 3; j++)
      {
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].position.asArray, 3));
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].normal.asArray, 3));
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].texCoord.asArray, 2));
      }
    }

    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.projectile.buffer, verts.pData, verts.size));

    dataBlob_destroy(&verts);
    queue_destroy(&info.triangles);
    queue_destroy(&info.vertices);

    LS_ERROR_CHECK(texture_create(&_Render.projectile.texture, "models/projectile.png"));
  }

  // Load Stone Asteroid.
  {
    LS_ERROR_CHECK(shader_createFromFile(&_Render.asteroid.shader, "shaders/asteroid.vert", "shaders/asteroid.frag"));

    static const char *modelNames[] = { "models/asteroid.obj", "models/asteroid_frag0.obj", "models/asteroid_frag1.obj", "models/asteroid_sfrag0.obj" };
    static const char *textureNames[] = { "models/asteroid_normal.png", "models/asteroid_frag0_normal.png", "models/asteroid_frag1_normal.png", "models/asteroid_sfrag0_normal.png" };

    static_assert(LS_ARRAYSIZE(modelNames) == LS_ARRAYSIZE(_Render.asteroid.buffer), "Invalid Configuration");
    static_assert(LS_ARRAYSIZE(textureNames) == LS_ARRAYSIZE(_Render.asteroid.buffer), "Invalid Configuration");

    for (size_t i = 0; i < LS_ARRAYSIZE(modelNames); i++)
    {
      LS_ERROR_CHECK(instancedVertexBuffer_create(&_Render.asteroid.buffer[i], &_Render.asteroid.shader));

      objInfo info;
      LS_ERROR_CHECK(objReader_loadFromFile(&info, modelNames[i]));

      dataBlob verts;

      for (size_t j = 0; j < info.triangles.count; j++)
      {
        auto &tri = queue_get(&info.triangles, j);

        for (size_t k = 0; k < 3; k++)
        {
          LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[k].position.asArray, 3));
          LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[k].normal.asArray, 3));
          LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[k].texCoord.asArray, 2));
        }
      }

      LS_ERROR_CHECK(instancedVertexBuffer_setInstancedVertexBuffer(&_Render.asteroid.buffer[i], verts.pData, verts.size));

      dataBlob_destroy(&verts);
      queue_destroy(&info.triangles);
      queue_destroy(&info.vertices);

      LS_ERROR_CHECK(texture_create(&_Render.asteroid.normal[i], textureNames[i]));
    }
  }

  // Load Copper Asteroid.
  {
    LS_ERROR_CHECK(shader_createFromFile(&_Render.copper.shader, "shaders/copper.vert", "shaders/copper.frag"));

    LS_ERROR_CHECK(instancedVertexBuffer_create(&_Render.copper.buffer, &_Render.copper.shader));
    
    objInfo info;
    LS_ERROR_CHECK(objReader_loadFromFile(&info, "models/copper.obj"));

    dataBlob verts;

    for (size_t i = 0; i < info.triangles.count; i++)
    {
      auto &tri = queue_get(&info.triangles, i);

      for (size_t j = 0; j < 3; j++)
      {
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].position.asArray, 3));
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].normal.asArray, 3));
      }
    }

    LS_ERROR_CHECK(instancedVertexBuffer_setInstancedVertexBuffer(&_Render.copper.buffer, verts.pData, verts.size));

    dataBlob_destroy(&verts);
    queue_destroy(&info.triangles);
    queue_destroy(&info.vertices);

    LS_ERROR_CHECK(texture_create(&_Render.copper.reflectionTexture, "textures/reflection.png"));
  }

  // Load Salt Asteroid.
  {
    LS_ERROR_CHECK(shader_createFromFile(&_Render.salt.shader, "shaders/salt.vert", "shaders/salt.frag"));

    static const char * modelNames[] = { "models/crystal.obj", "models/crystal2.obj", "models/crystal_frac0.obj", "models/crystal_frac1.obj", "models/crystal_frac2.obj", "models/crystal_frac3.obj", "models/crystal_sfrag0.obj", "models/crystal_sfrag1.obj", "models/crystal_sfrag2.obj", "models/crystal_sfrag3.obj" };
    static const char * textureNames[] = { "models/salt.png", "models/salt2.png", "models/saltfrac0.png", "models/saltfrac1.png", "models/saltfrac2.png", "models/saltfrac3.png", "models/saltsfrac0.png", "models/saltsfrac1.png", "models/saltsfrac2.png", "models/saltsfrac3.png" };

    static_assert(LS_ARRAYSIZE(modelNames) == LS_ARRAYSIZE(_Render.salt.buffer), "Invalid Configuration");
    static_assert(LS_ARRAYSIZE(textureNames) == LS_ARRAYSIZE(_Render.salt.buffer), "Invalid Configuration");

    for (size_t i = 0; i < LS_ARRAYSIZE(modelNames); i++)
    {
      LS_ERROR_CHECK(instancedVertexBuffer_create(&_Render.salt.buffer[i], &_Render.salt.shader));

      objInfo info;
      LS_ERROR_CHECK(objReader_loadFromFile(&info, modelNames[i]));

      dataBlob verts;

      for (size_t j = 0; j < info.triangles.count; j++)
      {
        auto &tri = queue_get(&info.triangles, j);

        for (size_t k = 0; k < 3; k++)
        {
          LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[k].position.asArray, 3));
          LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[k].normal.asArray, 3));
          LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[k].texCoord.asArray, 2));
        }
      }

      LS_ERROR_CHECK(instancedVertexBuffer_setInstancedVertexBuffer(&_Render.salt.buffer[i], verts.pData, verts.size));

      dataBlob_destroy(&verts);
      queue_destroy(&info.triangles);
      queue_destroy(&info.vertices);

      LS_ERROR_CHECK(texture_create(&_Render.salt.aoTexture[i], textureNames[i]));
    }

    LS_ERROR_CHECK(texture_create(&_Render.salt.reflectionTexture, "textures/reflection_streaks.png"));
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

    LS_ERROR_CHECK(texture_create(&tex, "textures/dingu.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_dingu));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/pupu.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_pupu));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/numbersprites.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_numbers));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/spaceship.png", true));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_spaceship));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/projectile.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_projectile));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/asteroid.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_asteroid));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/space.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_space));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/laser.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_laser));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/lasertip.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_laserHit));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/galaxy.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_galaxy));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/stone.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_stone));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/copper.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_copper));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/salt.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_salt));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/smoke.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_smoke));
    lsZeroMemory(&tex);

    LS_ERROR_CHECK(texture_create(&tex, "textures/particletex.png"));
    LS_ERROR_CHECK(pool_insertAt(&_Render.textures, tex, rTI_spaceshipExhaustColorRamp));
    lsZeroMemory(&tex);
  }

  // Init Particles.
  {
    LS_ERROR_CHECK(shader_createFromFile(&_Render.particles.shader, "shaders/particle.vert", "shaders/particle.frag"));
    LS_ERROR_CHECK(instancedVertexBuffer_create(&_Render.particles.buffer, &_Render.particles.shader));

    const float_t vertexBuffer[] = { 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0 };
    instancedVertexBuffer_setInstancedVertexBuffer(&_Render.particles.buffer, vertexBuffer, LS_ARRAYSIZE(vertexBuffer));
    instancedVertexBuffer_setInstancedVertexCount(&_Render.particles.buffer, 6);
  }

  // Init Stars.
  {
    LS_ERROR_CHECK(texture_create(&_Render.stars.gradient, "textures/startex.png"));

    LS_ERROR_CHECK(shader_createFromFile(&_Render.stars.shader, "shaders/star.vert", "shaders/star.frag"));
    LS_ERROR_CHECK(vertexBuffer_create(&_Render.stars.buffer, &_Render.stars.shader));
    _Render.stars.buffer.renderMode = GL_LINES;

    const float_t levelSize = (float_t)game_getLevelSize();
    constexpr float_t invRandMax = 1.f / (float_t)0xFFFF;
    const float_t randScale = levelSize * 3.f * invRandMax;
    constexpr float_t maxDepth = 250.f;

    constexpr size_t starCount = 1024;
    vec3f data[starCount * 2 + 1];

    for (size_t i = 0; i < starCount; i++)
    {
      const float_t depth = (lsGetRand() % 0xFFFF) * invRandMax;
      const vec3f pos = vec3f((lsGetRand() % 0xFFFF) * randScale - levelSize, (lsGetRand() % 0xFFFF) * randScale - levelSize, (1.f - depth * depth) * maxDepth + 50.f);

      data[i * 2] = pos;
      data[i * 2 + 1] = pos;
    }

    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.stars.buffer, data, LS_ARRAYSIZE(data)));

    LS_ERROR_CHECK(shader_createFromFile(&_Render.stars.blurShader, "shaders/starBlur.vert", "shaders/starBlur.frag"));

    float_t renderData[] = { 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0 };
    LS_ERROR_CHECK(vertexBuffer_create(&_Render.stars.blurBuffer, &_Render.stars.blurShader));
    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.stars.blurBuffer, renderData, LS_ARRAYSIZE(renderData)));

    LS_ERROR_CHECK(framebuffer_create(&_Render.stars.framebuffer, _Render.windowSize / 2, 0, false));
    LS_ERROR_CHECK(texture_create(&_Render.stars.bokeh, "textures/bokeh.png"));
  }

  // Load Galaxy.
  {
    LS_ERROR_CHECK(shader_createFromFile(&_Render.galaxy.shader, "shaders/galaxy.vert", "shaders/galaxy.frag"));

    LS_ERROR_CHECK(vertexBuffer_create(&_Render.galaxy.buffer, &_Render.galaxy.shader));

    objInfo info;
    LS_ERROR_CHECK(objReader_loadFromFile(&info, "models/galaxy.obj"));

    dataBlob verts;

    for (size_t i = 0; i < info.triangles.count; i++)
    {
      auto &tri = queue_get(&info.triangles, i);

      for (size_t j = 0; j < 3; j++)
      {
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].position.asArray, 3));
        LS_ERROR_CHECK(dataBlob_append(&verts, tri.v[j].texCoord.asArray, 2));
      }
    }

    LS_ERROR_CHECK(vertexBuffer_setVertexBuffer(&_Render.galaxy.buffer, verts.pData, verts.size));

    dataBlob_destroy(&verts);
    queue_destroy(&info.triangles);
    queue_destroy(&info.vertices);
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

  framebuffer_setResolution(&_Render.screenQuad.framebuffer, _Render.windowSize);
  framebuffer_setResolution(&_Render.screenQuad.tiny, _Render.windowSize / 16);
  framebuffer_setResolution(&_Render.stars.framebuffer, _Render.windowSize);
  framebuffer_bind(&_Render.screenQuad.framebuffer);
  
  render_clearColor(vec4f(0, 0, 0, 1));
  render_clearDepth();
  
  render_setDepthMode(rCR_Less);
  render_setBlendEnabled(false);
  render_setDepthTestEnabled(false);

  render_setLookAt(_Render.lookAt, _Render.up);

  // Clear all Instance Data Blobs.
  {
    for (size_t i = 0; i < LS_ARRAYSIZE(_Render.asteroid.instanceData); i++)
      dataBlob_reset(&_Render.asteroid.instanceData[i]);

    for (size_t i = 0; i < LS_ARRAYSIZE(_Render.salt.instanceData); i++)
      dataBlob_reset(&_Render.salt.instanceData[i]);

    dataBlob_reset(&_Render.copper.instanceData);
  }
}

void render_endFrame(lsAppState *pAppState)
{
  (void)pAppState;
  
  framebuffer_unbind();
  
  render_setDepthTestEnabled(false);
  
  // Draw Blur.
  {
    framebuffer_bind(&_Render.screenQuad.tiny);

    texture_bind(&_Render.screenQuad.framebuffer, 0);
    shader_bind(&_Render.screenQuad.blurShader);

    shader_setUniform(&_Render.screenQuad.blurShader, "texture", &_Render.screenQuad.framebuffer);

    vertexBuffer_render(&_Render.screenQuad.blurBuffer);

    framebuffer_unbind();
  }

  // Draw FX.
  {
    texture_bind(&_Render.screenQuad.framebuffer, 0);
    texture_bindDepthStencil(&_Render.screenQuad.framebuffer, 1);
    texture_bind(&_Render.screenQuad.tiny, 2);
    shader_bind(&_Render.screenQuad.shader);
  
    shader_setUniform(&_Render.screenQuad.shader, "sampleCount", (int32_t)_Render.screenQuad.framebuffer.sampleCount);
    shader_setUniform(&_Render.screenQuad.shader, "texture", &_Render.screenQuad.framebuffer);
    shader_setUniformDepthStencil(&_Render.screenQuad.shader, "depth", &_Render.screenQuad.framebuffer);
    shader_setUniform(&_Render.screenQuad.shader, "tiny", &_Render.screenQuad.tiny);
    shader_setUniform(&_Render.screenQuad.shader, "invTinySize", vec2f(1) / (vec2f)_Render.screenQuad.tiny.size);
    
    vertexBuffer_render(&_Render.screenQuad.buffer);
  }
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

void render_drawPlayer(const vec2f position, const float_t rotation)
{
  render_setDepthTestEnabled(true);
  render_setDepthMode(rCR_Less);

  texture_bind(&_Render.spaceship.texture, 0);

  texture *pTex = pool_get(&_Render.textures, rTI_space);
  texture_bind(pTex, 1);
  
  shader_bind(&_Render.spaceship.shader);
  shader_setUniform(&_Render.spaceship.shader, "texture", &_Render.spaceship.texture);
  shader_setUniform(&_Render.spaceship.shader, "spaceTex", pTex);
  shader_setUniform(&_Render.spaceship.shader, "rotMat", matrix::Scale(1, -1, 1) * matrix::RotationRollPitchYawFromVector(vec3f(rotation, 0, lsHALFPIf)));
  shader_setUniform(&_Render.spaceship.shader, "matrix", matrix::Scale(0.5, -0.5, 0.5) * matrix::RotationX(lsHALFPIf) * matrix::RotationZ(rotation) * matrix::TranslationFromVector(vec4f(position, 0, 0)) * _Render.vp);
  shader_setUniform(&_Render.spaceship.shader, "ambient", vec3f(0.02));
  shader_setUniform(&_Render.spaceship.shader, "lightDir", vec3f(0.7f, 0.1f, 0.5f).Normalize());

  glDisable(GL_CULL_FACE);
  vertexBuffer_render(&_Render.spaceship.buffer);
}

void render_drawProjectile(const vec2f position, const float_t rotation)
{
  render_setDepthTestEnabled(true);
  render_setDepthMode(rCR_Less);

  texture_bind(&_Render.projectile.texture, 0);

  texture *pTex = pool_get(&_Render.textures, rTI_space);
  texture_bind(pTex, 1);

  shader_bind(&_Render.spaceship.shader);
  shader_setUniform(&_Render.spaceship.shader, "texture", &_Render.projectile.texture);
  shader_setUniform(&_Render.spaceship.shader, "spaceTex", pTex);
  shader_setUniform(&_Render.spaceship.shader, "rotMat", matrix::Scale(1, -1, 1) * matrix::RotationRollPitchYawFromVector(vec3f(lsPIf, 0, lsHALFPIf - rotation)) * matrix::RotationX(lsPIf));
  shader_setUniform(&_Render.spaceship.shader, "matrix", matrix::Scale(0.2, -0.2, 0.2) * matrix::RotationRollPitchYawFromVector(vec3f(lsPIf, 0, lsHALFPIf - rotation)) * matrix::TranslationFromVector(vec4f(position, 0, 0)) * _Render.vp);
  shader_setUniform(&_Render.spaceship.shader, "ambient", vec3f(0.02));
  shader_setUniform(&_Render.spaceship.shader, "lightDir", vec3f(0.7f, 0.1f, 0.5f).Normalize());

  glDisable(GL_CULL_FACE);
  vertexBuffer_render(&_Render.projectile.buffer);
}

void render_drawAsteroidStone(const size_t index, const vec2f position, const enum gameObject_asteroidSize asteroidSize)
{
  render_drawAsteroidStone(index, vec3f(position, 0), asteroidSize);
}

void render_drawAsteroidStone(const size_t index, const vec3f position, const enum gameObject_asteroidSize asteroidSize)
{
  constexpr float_t scaleFactor = 1.f / 512.f * lsTWOPIf;

  const float_t initialRotationX = ((int64_t)(index * 0x124890FA19025922 + 0x128490) % 1024 - 512) * scaleFactor;
  const float_t initialRotationY = ((int64_t)(index * 0x7D1FB6D8D6FC9921 + 0x128490) % 1024 - 512) * scaleFactor;
  const float_t initialRotationZ = ((int64_t)(index * 0x16D9B69D48BCC042 + 0x128490) % 1024 - 512) * scaleFactor;
  const float_t angularVelocityX = ((int64_t)(index * 0x191390F1480ABC12 + 0xABC32985) % 1024 - 512) * scaleFactor * 0.00025f;
  const float_t angularVelocityY = ((int64_t)(index * 0x0E07039610629A04 + 0xABC32985) % 1024 - 512) * scaleFactor * 0.00025f;
  const float_t angularVelocityZ = ((int64_t)(index * 0x1D198C765DD4A8F1 + 0xABC32985) % 1024 - 512) * scaleFactor * 0.00025f;

  const vec3f rotation = vec3f(initialRotationX, initialRotationY, initialRotationZ) + vec3f(angularVelocityX, angularVelocityY, angularVelocityZ) * _Render.ticksSinceOrigin;
  
  float_t scale;
  size_t modelIndex;

  switch (asteroidSize)
  {
  default:
  case goAS_Large:
  {
    scale = 0.035;
    modelIndex = 0;
    break;
  }
  case goAS_Medium:
  {
    scale = 0.035;
    modelIndex = 1 + ((index * 0x2150124980 + 0x8139405) % 2);
    break;
  }
  case goAS_Small:
  {
    scale = 0.035;
    modelIndex = 3;
    break;
  }
  }
  
  dataBlob_appendValue(&_Render.asteroid.instanceData[modelIndex], matrix::Scale(scale, scale, scale) * matrix::TranslationFromVector(vec4f(position, 0)));
  dataBlob_appendValue(&_Render.asteroid.instanceData[modelIndex], rotation);
}

void render_drawAsteroidCopper(const size_t index, const vec2f position, const enum gameObject_asteroidSize asteroidSize)
{
  render_drawAsteroidCopper(index, vec3f(position, 0), asteroidSize);
}

void render_drawAsteroidCopper(const size_t index, const vec3f position, const enum gameObject_asteroidSize asteroidSize)
{
  constexpr float_t scaleFactor = 1.f / 512.f * lsTWOPIf;
  const float_t initialRotationX = ((int64_t)(index * 0x124890FA19025922 + 0x128490) % 1024 - 512) * scaleFactor;
  const float_t initialRotationY = ((int64_t)(index * 0x7D1FB6D8D6FC9921 + 0x128490) % 1024 - 512) * scaleFactor;
  const float_t initialRotationZ = ((int64_t)(index * 0x16D9B69D48BCC042 + 0x128490) % 1024 - 512) * scaleFactor;
  const float_t angularVelocityX = ((int64_t)(index * 0x191390F1480ABC12 + 0xABC32985) % 1024 - 512) * scaleFactor * 0.00025f;
  const float_t angularVelocityY = ((int64_t)(index * 0x0E07039610629A04 + 0xABC32985) % 1024 - 512) * scaleFactor * 0.00025f;
  const float_t angularVelocityZ = ((int64_t)(index * 0x1D198C765DD4A8F1 + 0xABC32985) % 1024 - 512) * scaleFactor * 0.00025f;

  const vec3f rotation = vec3f(initialRotationX, initialRotationY, initialRotationZ) + vec3f(angularVelocityX, angularVelocityY, angularVelocityZ) * _Render.ticksSinceOrigin;

  float_t scale;

  switch (asteroidSize)
  {
  default:
  case goAS_Large:
  {
    scale = 0.5f;
    break;
  }
  case goAS_Medium:
  {
    scale = 0.25f;
    break;
  }
  case goAS_Small:
  {
    scale = 0.125f;
    break;
  }
  }

  dataBlob_appendValue(&_Render.copper.instanceData, matrix::Scale(scale, scale, scale) * matrix::TranslationFromVector(vec4f(position, 0)));
  dataBlob_appendValue(&_Render.copper.instanceData, rotation);
}

void render_drawAsteroidSalt(const size_t index, const vec2f position, const enum gameObject_asteroidSize asteroidSize)
{
  render_drawAsteroidSalt(index, vec3f(position, 0), asteroidSize);
}

void render_drawAsteroidSalt(const size_t index, const vec3f position, const enum gameObject_asteroidSize asteroidSize)
{
  size_t modelIndex;
  float_t scale;

  switch (asteroidSize)
  {
  default:
  case goAS_Large:
  {
    scale = 1.25f;
    modelIndex = (index * 0x2150124980 + 0x8139405) % 2;
    break;
  }
  case goAS_Medium:
  {
    scale = 1.85f;
    modelIndex = 2 + ((index * 0x2150124980 + 0x8139405) % 4);
    break;
  }
  case goAS_Small:
  {
    scale = .65f;
    modelIndex = 6 + ((index * 0x2150124980 + 0x8139405) % 4);
    break;
  }
  }

  constexpr float_t scaleFactor = 1.f / 512.f * lsTWOPIf;
  const float_t initialRotationX = ((int64_t)(index * 0x124890FA19025922 + 0x128490) % 1024 - 512) * scaleFactor;
  const float_t initialRotationY = ((int64_t)(index * 0x7D1FB6D8D6FC9921 + 0x128490) % 1024 - 512) * scaleFactor;
  const float_t initialRotationZ = ((int64_t)(index * 0x16D9B69D48BCC042 + 0x128490) % 1024 - 512) * scaleFactor;
  const float_t angularVelocityX = ((int64_t)(index * 0x191390F1480ABC12 + 0xABC32985) % 1024 - 512) * scaleFactor * 0.00025f;
  const float_t angularVelocityY = ((int64_t)(index * 0x0E07039610629A04 + 0xABC32985) % 1024 - 512) * scaleFactor * 0.00025f;
  const float_t angularVelocityZ = ((int64_t)(index * 0x1D198C765DD4A8F1 + 0xABC32985) % 1024 - 512) * scaleFactor * 0.00025f;

  const vec3f rotation = vec3f(initialRotationX, initialRotationY, initialRotationZ) + vec3f(angularVelocityX, angularVelocityY, angularVelocityZ) * _Render.ticksSinceOrigin;
  
  dataBlob_appendValue(&_Render.salt.instanceData[modelIndex], matrix::Scale(scale, scale, scale) * matrix::TranslationFromVector(vec4f(position, 0)));
  dataBlob_appendValue(&_Render.salt.instanceData[modelIndex], rotation);
}

void render_drawLaser(const vec2f startPosition, const float_t rotation, const float_t length, const bool hit)
{
  render_drawLaser(vec3f(startPosition, 0), rotation, length, hit);
}

void render_drawLaser(const vec3f startPosition, const float_t rotation, const float_t length, const bool hit)
{
  render_setDepthTestEnabled(false);
  render_setBlendEnabled(true);
  render_setBlendMode(rBF_Additive);

  texture *pLaserTex = pool_get(&_Render.textures, rTI_laser);
  texture_bind(pLaserTex, 0);

  const matrix laserModel = matrix::Translation(-0.5f, 0, 0) * matrix::Scale(0.25f, length + (lsGetRand() % 0x10000) / (float_t)0x10000 * 0.05f, 1) * matrix::RotationZ(rotation - lsHALFPIf + (lsGetRand() % 0x10000) / (float_t)0x10000 * 0.005f) * matrix::TranslationFromVector(startPosition) * _Render.vp;

  shader_bind(&_Render.plane.shader);
  shader_setUniform(&_Render.plane.shader, "texture", pLaserTex);
  shader_setUniform(&_Render.plane.shader, "matrix", laserModel);
  vertexBuffer_render(&_Render.plane.buffer);

  if (hit)
  {
    texture *pLaserHitTex = pool_get(&_Render.textures, rTI_laserHit);
    texture_bind(pLaserHitTex, 0);

    const float_t scale = 0.6f + (lsGetRand() % 0x10000) / (float_t)0x10000 * 0.05f;

    const matrix laserHitModel = matrix::Translation(-0.5f, -0.5f, 0) * matrix::Scale(scale, scale, 1) * matrix::RotationZ((lsGetRand() % 0x10000) / (float_t)0x10000 * lsTWOPIf) * matrix::TranslationFromVector(startPosition + vec3f(vec2f(cosf(rotation), sinf(rotation)) * length, 0)) * _Render.vp;

    shader_setUniform(&_Render.plane.shader, "texture", pLaserHitTex);
    shader_setUniform(&_Render.plane.shader, "matrix", laserHitModel);
    vertexBuffer_render(&_Render.plane.buffer);
  }

  render_setBlendEnabled(false);
}

void render_drawScore(const size_t score)
{
  render_drawIntegerAt(score, vec2f(50.f, _Render.windowSize.y - 100.f));
}

void render_drawRessourceCount(const vec2f position, const render_textureId textureIndex, const size_t resourceCount)
{
  render_setBlendMode(rBF_AlphaBlend);
  render_setBlendEnabled(true);

  render_draw2DQuad(matrix::Scale(1, -1, 0) * matrix::Translation(0, 1, 0) * matrix::Scale(50.f, 50.f, 0) * matrix::Translation(position.x, position.y, 0), textureIndex);

  render_drawIntegerAt(resourceCount, vec2f(position.x + 52.f, position.y));
}

void render_drawIntegerAt(const size_t integer, const vec2f positionFirstNumber)
{
  render_setDepthTestEnabled(false);
  render_setBlendEnabled(true);
  render_setBlendMode(rBF_AlphaBlend);

  texture *pTex = pool_get(&_Render.textures, rTI_numbers);
  const vec2f scale = ((vec2f)pTex->resolution / (vec2f)_Render.windowSize) * vec2f(0.1f, 1.f);

  texture_bind(pTex, 0);
  shader_bind(&_Render.numbers.shader);
  shader_setUniform(&_Render.numbers.shader, "texture", pTex);
  shader_setUniform(&_Render.numbers.shader, "scale", scale);

  uint8_t scores[20];
  size_t index = LS_ARRAYSIZE(scores);
  size_t value = integer;

  do
  {
    index--;
    scores[index] = (uint8_t)(value % 10);
    value /= 10;
    
  } while (value > 0);

  vec2f position = positionFirstNumber / (vec2f)_Render.windowSize;

  for (; index < LS_ARRAYSIZE(scores); index++)
  {
    shader_setUniform(&_Render.numbers.shader, "index", (float_t)scores[index]);
    shader_setUniform(&_Render.numbers.shader, "offset", position);
    vertexBuffer_render(&_Render.numbers.buffer);
    position.x += scale.x;
  }

  render_setBlendEnabled(false);
}

void render_drawStars()
{
  framebuffer_bind(&_Render.stars.framebuffer);

  render_clearColor(vec4f(0, 0, 0, 1));
  render_clearDepth();
  render_setDepthTestEnabled(false);
  render_setBlendMode(rBF_Additive);
  render_setBlendEnabled(true);

  glLineWidth(3.f);

  const matrix vp = matrix::TranslationFromVector(vec3f(vec2f((float_t)game_getLevelSize()) * -0.5f, 0)) * matrix::RotationZ(_Render.stars.rotationZ) * matrix::TranslationFromVector(vec3f(vec2f((float_t)game_getLevelSize()) * 0.5f, 0)) * _Render.vpFar;

  texture_bind(&_Render.stars.gradient, 0);
  shader_bind(&_Render.stars.shader);
  shader_setUniform(&_Render.stars.shader, "texture", &_Render.stars.gradient);
  shader_setUniform(&_Render.stars.shader, "matrix", vp);
  shader_setUniform(&_Render.stars.shader, "lastMatrix", _Render.stars.lastVP);
  vertexBuffer_render(&_Render.stars.buffer);

  texture *pGalaxyTex = pool_get(&_Render.textures, rTI_galaxy);
  const float_t levelSize = (float_t)game_getLevelSize();

  texture_bind(pGalaxyTex, 0);
  shader_bind(&_Render.galaxy.shader);
  shader_setUniform(&_Render.galaxy.shader, "texture", pGalaxyTex);
  shader_setUniform(&_Render.galaxy.shader, "matrix", matrix::Scale(levelSize * 4.f, levelSize * 4.f, 200.f) * matrix::Translation(levelSize * -2.f, levelSize * 2.5f, 200.f) * vp);
  vertexBuffer_render(&_Render.galaxy.buffer);
  
  render_clearDepth();

  _Render.stars.lastVP = vp;
  
  _Render.stars.rotationZ += 0.0015f * _Render.frameRatio;
  
  framebuffer_bind(&_Render.screenQuad.framebuffer);
  
  render_setBlendEnabled(false);
  
  // Draw FX.
  {
    texture_bind(&_Render.stars.framebuffer, 0);
    texture_bind(&_Render.stars.bokeh, 1);
    shader_bind(&_Render.stars.blurShader);
  
    shader_setUniform(&_Render.stars.blurShader, "texture", &_Render.stars.framebuffer);
    shader_setUniform(&_Render.stars.blurShader, "weight", &_Render.stars.bokeh);
    shader_setUniform(&_Render.stars.blurShader, "invSize", vec2f(1) / (vec2f)_Render.screenQuad.framebuffer.size);
    shader_setUniform(&_Render.stars.blurShader, "weightSize", (vec2f)_Render.stars.bokeh.resolution);
    
    vertexBuffer_render(&_Render.stars.blurBuffer);
  }
}

void render_drawParticles(const vec3f *pPositionAlpha, const size_t count, const render_textureId particleTexture, const render_textureId alphaTexture, const vec2f size, const float_t sizeAlphaFactor)
{
  render_setDepthTestEnabled(false);
  render_setBlendMode(rBF_AlphaBlend);
  render_setBlendEnabled(true);

  glPointSize(0.01f * lsMax(_Render.windowSize));

  instancedVertexBuffer_setInstanceBuffer(&_Render.particles.buffer, pPositionAlpha, count, true);

  texture *pParticleTexture = pool_get(&_Render.textures, particleTexture);
  texture *pAlphaTexture = pool_get(&_Render.textures, alphaTexture);

  texture_bind(pParticleTexture, 0);
  texture_bind(pAlphaTexture, 1);

  shader_bind(&_Render.particles.shader);

  shader_setUniform(&_Render.particles.shader, "texture", pParticleTexture);
  shader_setUniform(&_Render.particles.shader, "alphaTexture", pAlphaTexture);
  shader_setUniform(&_Render.particles.shader, "matrix", _Render.vp);
  shader_setUniform(&_Render.particles.shader, "scale", size);
  shader_setUniform(&_Render.particles.shader, "scaleAlphaFactor", sizeAlphaFactor);
  shader_setUniform(&_Render.particles.shader, "maxAlpha", 0.2f);
  shader_setUniform(&_Render.particles.shader, "minColor", 0.7f);

  instancedVertexBuffer_render(&_Render.particles.buffer);

  render_setBlendEnabled(false);
}

void render_flushRenderQueue()
{
  // Draw Stone Asteroids.
  {
    render_setDepthTestEnabled(true);
    render_setDepthMode(rCR_Less);

    glDisable(GL_CULL_FACE);

    shader_bind(&_Render.asteroid.shader);

    shader_setUniform(&_Render.asteroid.shader, "vp", _Render.vp);
    shader_setUniform(&_Render.asteroid.shader, "ambient", vec3f(0.01f));
    shader_setUniform(&_Render.asteroid.shader, "baseColor", vec3f(0.75f));
    shader_setUniform(&_Render.asteroid.shader, "lightDir", vec3f(0.7f, 0.1f, 0.5f).Normalize());

    for (size_t i = 0; i < LS_ARRAYSIZE(_Render.asteroid.buffer); i++)
    {
      if (_Render.asteroid.instanceData[i].size == 0)
        continue;

      instancedVertexBuffer_setInstanceBuffer(&_Render.asteroid.buffer[i], _Render.asteroid.instanceData[i].pData, _Render.asteroid.instanceData[i].size);

      texture_bind(&_Render.asteroid.normal[i], 0);
      shader_setUniform(&_Render.asteroid.shader, "normalTexture", &_Render.asteroid.normal[i]);

      instancedVertexBuffer_render(&_Render.asteroid.buffer[i]);
    }
  }
  
  // Draw Copper Asteroids.
  {
    instancedVertexBuffer_setInstanceBuffer(&_Render.copper.buffer, _Render.copper.instanceData.pData, _Render.copper.instanceData.size);

    render_setDepthTestEnabled(true);
    render_setDepthMode(rCR_Less);

    shader_bind(&_Render.copper.shader);
    texture_bind(&_Render.copper.reflectionTexture, 0);

    shader_setUniform(&_Render.copper.shader, "vp", _Render.vp);
    shader_setUniform(&_Render.copper.shader, "ambient", vec3f(0.02f));
    shader_setUniform(&_Render.copper.shader, "baseColor", vec3f(0.95f, 0.6f, 0.4f));
    shader_setUniform(&_Render.copper.shader, "lightDir", vec3f(0.7f, 0.1f, 0.5f).Normalize());
    shader_setUniform(&_Render.copper.shader, "reflectivity", 0.5f);
    shader_setUniform(&_Render.copper.shader, "reflectionTexture", &_Render.copper.reflectionTexture);

    glDisable(GL_CULL_FACE);
    instancedVertexBuffer_render(&_Render.copper.buffer);
  }

  // Draw Salt Asteroids.
  {
    render_setDepthTestEnabled(true);
    render_setDepthMode(rCR_Less);

    glDisable(GL_CULL_FACE);

    shader_bind(&_Render.salt.shader);

    shader_setUniform(&_Render.salt.shader, "vp", _Render.vp);
    shader_setUniform(&_Render.salt.shader, "ambient", vec3f(0.02f));
    shader_setUniform(&_Render.salt.shader, "lightDir", vec3f(0.7f, 0.1f, 0.5f).Normalize());
    shader_setUniform(&_Render.salt.shader, "reflectivity", 0.3f);
    shader_setUniform(&_Render.salt.shader, "reflectionTexture", &_Render.salt.reflectionTexture);

    texture_bind(&_Render.salt.reflectionTexture, 0);

    for (size_t i = 0; i < LS_ARRAYSIZE(_Render.salt.buffer); i++)
    {
      if (_Render.salt.instanceData[i].size == 0)
        continue;

      instancedVertexBuffer_setInstanceBuffer(&_Render.salt.buffer[i], _Render.salt.instanceData[i].pData, _Render.salt.instanceData[i].size);
      
      texture_bind(&_Render.salt.aoTexture[i], 1);
      shader_setUniform(&_Render.salt.shader, "texture", &_Render.salt.aoTexture[i]);

      instancedVertexBuffer_render(&_Render.salt.buffer[i]);
    }
  }
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
