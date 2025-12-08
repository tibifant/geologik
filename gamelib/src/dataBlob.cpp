#include "dataBlob.h"

//////////////////////////////////////////////////////////////////////////

void dataBlob_createFromForeign(dataBlob *pBlob, const void *pData, const size_t bytes)
{
  if (pBlob == nullptr)
    return;

  pBlob->pData = reinterpret_cast<uint8_t *>(const_cast<void *>(pData));
  pBlob->isForeign = true;
  pBlob->size = bytes;
  pBlob->readPosition = 0;
}

lsResult dataBlob_reserve(dataBlob *pBlob, const size_t minimumSize)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pBlob == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(pBlob->isForeign, lsR_ResourceStateInvalid);

  if (pBlob->capacity < minimumSize)
  {
    LS_ERROR_CHECK(lsRealloc(&pBlob->pData, minimumSize));
    pBlob->capacity = minimumSize;
  }

epilogue:
  return result;
}

void dataBlob_destroy(dataBlob *pBlob)
{
  if (!pBlob->isForeign)
    lsFreePtr(&pBlob->pData);

  pBlob->size = 0;
  pBlob->capacity = 0;
}

void dataBlob_reset(dataBlob *pBlob)
{
  if (pBlob == nullptr)
    return;

  pBlob->readPosition = 0;

  if (!pBlob->isForeign)
    pBlob->size = 0;
}
