#pragma once

#include "platform.h"

enum render_textureId : size_t
{
  rTI_default,
  rTI_dingu,
  rTI_pupu,
  rTI_numbers,
  rTI_spaceship,
  rTI_projectile,
  rTI_asteroid,
  rTI_space,
  rTI_laser,
  rTI_laserHit,
  rTI_galaxy,
  rTI_stone,
  rTI_copper,
  rTI_salt,
  rTI_smoke,
  rTI_spaceshipExhaustColorRamp,
};

lsResult render_init(lsAppState *pAppState);
void render_startFrame(lsAppState *pAppState);
void render_endFrame(lsAppState *pAppState);
void render_destroy();

void render_setCameraOffset(const vec3f offset);
void render_setLookAt(const vec2f position, const vec2f up);
void render_setLookAt(const vec3f position, const vec3f up);
void render_setTicksSinceOrigin(const float_t ticksSinceOrigin);

void render_drawQuad(const matrix &model, const render_textureId textureIndex);
void render_draw2DQuad(const matrix &model, const render_textureId textureIndex);
void render_draw3DQuad(const matrix &model, const render_textureId textureIndex);

void render_drawPlayer(const vec2f position, const float_t rotation);
void render_drawProjectile(const vec2f position, const float_t rotation);
void render_drawAsteroidStone(const size_t index, const vec2f position, const enum gameObject_asteroidSize asteroidSize);
void render_drawAsteroidStone(const size_t index, const vec3f position, const enum gameObject_asteroidSize asteroidSize);
void render_drawAsteroidCopper(const size_t index, const vec2f position, const enum gameObject_asteroidSize asteroidSize);
void render_drawAsteroidCopper(const size_t index, const vec3f position, const enum gameObject_asteroidSize asteroidSize);
void render_drawAsteroidSalt(const size_t index, const vec2f position, const enum gameObject_asteroidSize asteroidSize);
void render_drawAsteroidSalt(const size_t index, const vec3f position, const enum gameObject_asteroidSize asteroidSize);
void render_drawLaser(const vec2f startPosition, const float_t rotation, const float_t length, const bool hit);
void render_drawLaser(const vec3f startPosition, const float_t rotation, const float_t length, const bool hit);
void render_drawScore(const size_t score);
void render_drawRessourceCount(const vec2f position, const render_textureId textureIndex, const size_t resourceCount);
void render_drawIntegerAt(const size_t value, const vec2f positionFirstNumber);
void render_drawStars();
void render_drawParticles(const vec3f *pPositionAlpha, const size_t count, const render_textureId particleTexture, const render_textureId alphaTexture, const vec2f size, const float_t sizeAlphaFactor);

void render_flushRenderQueue();

void render_finalize();

void render_clearColor(const vec4f color);
void render_clearDepth();

enum render_comparisonResult
{
  rCR_Less,
  rCR_LessOrEqual,
  rCR_Equal,
  rCR_GreaterOrEqual,
  rCR_Greater,
  rCR_NotEqual,
  rCR_Always,
};

void render_setDepthMode(const render_comparisonResult mode);
void render_setDepthTestEnabled(const bool enabled);

enum render_blendFunc
{
  rBF_Additive,
  rBF_AlphaBlend,
  rBF_Premultiplied,
  rBF_Override,
  rBF_AlphaMask,
};

void render_setBlendMode(const render_blendFunc mode);
void render_setBlendEnabled(const bool enabled);
