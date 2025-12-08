#include "gameView.h"
#include "game.h"
#include "render.h"

//////////////////////////////////////////////////////////////////////////

struct gameView : lsAppView
{
  game game;
  size_t playerIndex;
  vec2f lookAtPos;
  float_t lookAtRotation;
  float_t lookAtDistance;

#pragma pack(1)
  struct particle
  {
    vec2f position;
    float_t transparency;
  } particles[512];

  _STATIC_ASSERT(sizeof(particle) == sizeof(vec3f));
  _STATIC_ASSERT(sizeof(particle) == sizeof(float_t) * 3);

  vec2f particleVelocities[LS_ARRAYSIZE_C_STYLE(particles)];

  size_t lastParticleIndex = 0;
  bool lagSwitch = false;
};

//////////////////////////////////////////////////////////////////////////

lsResult gameView_update(lsAppView *pSelf, lsAppView **ppNext, lsAppState *pAppState);
void gameView_destroy(lsAppView **ppSelf, lsAppState *pAppState);

//////////////////////////////////////////////////////////////////////////

lsResult gameView_init(_Out_ lsAppView **ppView, lsAppState *pAppState)
{
  lsResult result = lsR_Success;

  gameView *pView = nullptr;

  LS_ERROR_IF(ppView == nullptr || pAppState == nullptr, lsR_ArgumentNull);

  LS_ERROR_CHECK(lsAllocZero(&pView));

  pView->pUpdate = gameView_update;
  pView->pDestroy = gameView_destroy;

  *ppView = pView;

  LS_ERROR_CHECK(game_init());
  pView->playerIndex = game_addPlayer();

  printf("Connected as player index %" PRIu64 ".\n", pView->playerIndex);

  while (pView->game.entities.count == 0)
  {
    LS_ERROR_CHECK(game_observe(&pView->game));
    Sleep(1);
  }

  size_t playerEntityIndex; // is identical to the game object index.
  game_getActiveActorForPlayer(&pView->game, pView->playerIndex, &playerEntityIndex);
  gameObject *pGameObject = pool_get(&pView->game.gameObjects, playerEntityIndex);

  pView->lookAtPos = pGameObject->position;
  pView->lookAtRotation = pGameObject->rotation;
  pView->lookAtDistance = 35.f;

epilogue:
  if (LS_FAILED(result))
    lsFreePtr(&pView);

  return result;
}

//////////////////////////////////////////////////////////////////////////

lsResult gameView_update(lsAppView *pSelf, lsAppView **ppNext, lsAppState *pAppState)
{
  lsResult result = lsR_Success;

  gameView *pView = static_cast<gameView *>(pSelf);

  (void)ppNext;

  render_startFrame(pAppState);
  
  size_t playerEntityIndex; // is identical to the game object index.
  actor *pActor = game_getActiveActorForPlayer(&pView->game, pView->playerIndex, &playerEntityIndex);
  gameObject *pGameObject = pool_get(&pView->game.gameObjects, playerEntityIndex);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_W) != pActor->thrusterForward)
    (pActor->isPlayer ? game_spaceship_thruster_set_state : game_projectile_thruster_set_state)(pView->playerIndex, 0, !pActor->thrusterForward);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_S) != pActor->thrusterBack)
    (pActor->isPlayer ? game_spaceship_thruster_set_state : game_projectile_thruster_set_state)(pView->playerIndex, 1, !pActor->thrusterBack);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_D) != pActor->thrusterTurnRight)
    (pActor->isPlayer ? game_spaceship_thruster_set_state : game_projectile_thruster_set_state)(pView->playerIndex, 2, !pActor->thrusterTurnRight);

  if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_A) != pActor->thrusterTurnLeft)
    (pActor->isPlayer ? game_spaceship_thruster_set_state : game_projectile_thruster_set_state)(pView->playerIndex, 3, !pActor->thrusterTurnLeft);

  if (lsKeyboardState_KeyPress(&pAppState->keyboardState, SDL_SCANCODE_SPACE))
    (pActor->isPlayer ? game_spaceship_fire : game_projectile_trigger)(pView->playerIndex);

  if (lsKeyboardState_KeyPress(&pAppState->keyboardState, SDL_SCANCODE_TAB))
    game_switch(pView->playerIndex);

  if (pActor->hasLaser)
    if (lsKeyboardState_IsKeyDown(&pAppState->keyboardState, SDL_SCANCODE_LCTRL) != pActor->laserOn)
      game_spaceship_laser_set_state(pView->playerIndex, !pActor->laserOn);

  if (lsKeyboardState_KeyPress(&pAppState->keyboardState, SDL_SCANCODE_RETURN))
    pView->lagSwitch = !pView->lagSwitch;

  LS_ERROR_CHECK(game_tick());

  if (!pView->lagSwitch)
    LS_ERROR_CHECK(game_observe(&pView->game));
  
  game_predict(&pView->game);

  // Handle Camera.
  {
    vec2f lookDirection = vec2f(cosf(pGameObject->rotation), sinf(pGameObject->rotation));

    if (vec2f::Dot(lookDirection, pGameObject->velocity) < 0)
      lookDirection = -lookDirection;

    const vec2f predictedFuturePosition = pGameObject->position + pGameObject->velocity.Length() * lookDirection * 45.f;

    pView->lookAtPos = lsLerp(pView->lookAtPos, predictedFuturePosition, 0.05f);
    pView->lookAtRotation = lsLerp(pView->lookAtRotation, pGameObject->rotation, 0.05f);
    pView->lookAtDistance = lsLerp(pView->lookAtDistance, (pActor->isPlayer ? 1.f : 2.5f) * lsLerp(5.5f, 35.f, pGameObject->velocity.Length() * 0.5f), 0.05f);

    render_setLookAt(pView->lookAtPos, vec2f(cosf(pView->lookAtRotation), sinf(pView->lookAtRotation)));
    render_setCameraOffset(vec3f(0, 0, pView->lookAtDistance));
  }

  // Handle Particles.
  {
    for (size_t i = 0; i < LS_ARRAYSIZE(pView->particles); i++)
    {
      pView->particles[i].transparency = lsLerp(pView->particles[i].transparency, 0.f, 0.02f);
      pView->particles[i].position += pView->particleVelocities[i];
      pView->particleVelocities[i] *= 0.99f;
    }
  }

  // Update events.
  {
    while (game_hasAnyEvent(&pView->game))
    {
      gameEvent evnt = game_getNextEvent(&pView->game);

      switch (evnt.type)
      {
      case geT_projectileExplosion:
      case geT_teleportation:
      case geT_asteroidDestroyed:
      {
        for (size_t i = 0; i < 64; i++)
        {
          pView->particles[pView->lastParticleIndex % LS_ARRAYSIZE(pView->particles)].position = evnt.position;
          pView->particles[pView->lastParticleIndex % LS_ARRAYSIZE(pView->particles)].transparency = 1.f;
          pView->particleVelocities[pView->lastParticleIndex % LS_ARRAYSIZE(pView->particles)] = vec2f((float_t)(lsGetRand() % 1024) - 512, (float_t)(lsGetRand() % 1024) - 512).Normalize() * (float_t)(lsGetRand() % 1024) * 0.00025f;
          pView->lastParticleIndex++;
        }

        break;
      }
      }
    }
  }

  // Draw Scene
  {
    render_drawStars();
    render_drawParticles(reinterpret_cast<const vec3f *>(pView->particles), LS_ARRAYSIZE(pView->particles), rTI_smoke, rTI_spaceshipExhaustColorRamp, vec2f(1.5), 2.5f);

    const float_t ticksSinceOrigin = (pView->game.lastPredictTimeNs - pView->game.gameStartTimeNs) / (1e9f / pView->game.tickRate);

    render_setTicksSinceOrigin(ticksSinceOrigin);

    for (auto o : pView->game.gameObjects)
    {
      switch (o.pItem->type)
      {
      case goT_asteroid:
      {        
        switch (o.pItem->associatedData.asteroidType)
        {
        case goAT_stone:
        {
          render_drawAsteroidStone(o.index, o.pItem->position, o.pItem->associatedData.asteroidSize);
        
          break;
        }
        
        case goAT_copper:
        {
          render_drawAsteroidCopper(o.index, o.pItem->position, o.pItem->associatedData.asteroidSize);
        
          break;
        }

        case goAT_salt:
        {
          render_drawAsteroidSalt(o.index, o.pItem->position, o.pItem->associatedData.asteroidSize);

          break;
        }
        
        case goAT_gold:
        case goAT_ice:
        default:
          lsAssert(false);
        }
         
        break;
      }

      case goT_spaceship:
      {
        pView->particles[pView->lastParticleIndex % LS_ARRAYSIZE(pView->particles)].position = o.pItem->position;
        pView->particles[pView->lastParticleIndex % LS_ARRAYSIZE(pView->particles)].transparency = 1.f;
        pView->particleVelocities[pView->lastParticleIndex % LS_ARRAYSIZE(pView->particles)] = -o.pItem->velocity + vec2f((float_t)(lsGetRand() % 1024) - 512, (float_t)(lsGetRand() % 1024) - 512) * 0.000035f;
        pView->lastParticleIndex++;

        // Will be rendererd later.

        break;
      }

      case goT_projectile:
      {
        render_drawProjectile(o.pItem->position, o.pItem->rotation);

        pView->particles[pView->lastParticleIndex % LS_ARRAYSIZE(pView->particles)].position = o.pItem->position;
        pView->particles[pView->lastParticleIndex % LS_ARRAYSIZE(pView->particles)].transparency = 1.f;
        pView->particleVelocities[pView->lastParticleIndex % LS_ARRAYSIZE(pView->particles)] = -o.pItem->velocity + vec2f((float_t)(lsGetRand() % 1024) - 512, (float_t)(lsGetRand() % 1024) - 512) * 0.000035f;
        pView->lastParticleIndex++;

        break;
      }
      }
    }

    render_flushRenderQueue();
  }

  // Draw Spaceships.
  {
    for (const auto &_player : pView->game.players)
    {
      const size_t idx = _player.pItem->entityIndex;
      actor *pPlayerActor = pool_get(&pView->game.actors, game_getFirstComponentIndex(&pView->game, idx, ct_actor));

      gameObject *pPlayerGameObject = pool_get(&pView->game.gameObjects, idx);

      if (pPlayerActor->hasLaser && pPlayerActor->laserOn)
        render_drawLaser(pPlayerGameObject->position, pPlayerGameObject->rotation, pPlayerActor->currentLaserLength, pPlayerActor->hasLaserHit);

      render_drawPlayer(pPlayerGameObject->position, pPlayerGameObject->rotation);
    }
  }

  render_endFrame(pAppState);

  // Draw UI
  {
    playerData *pPlayerData = pool_get(&pView->game.players, pView->playerIndex);

    render_drawScore(pPlayerData->score);

    render_drawRessourceCount(vec2f(50, 50), rTI_stone, pPlayerData->resourceCount[goAT_stone]);
    render_drawRessourceCount(vec2f(50, 100), rTI_copper, pPlayerData->resourceCount[goAT_copper]);

    render_drawRessourceCount(vec2f(50, 150), rTI_salt, pPlayerData->resourceCount[goAT_salt]);
  }

epilogue:
  return result;
}

void gameView_destroy(lsAppView **ppSelf, lsAppState *pAppState)
{
  (void)pAppState;

  lsFreePtr(ppSelf);
}
