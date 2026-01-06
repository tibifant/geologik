#pragma once

#include "core.h"

template <typename T>
struct queue
{
  T *pStart = nullptr;
  T *pFront = nullptr;
  T *pBack = nullptr;
  T *pLast = nullptr;
  size_t capacity = 0, count = 0;
};

template <typename T>
lsResult queue_reserve(queue<T> *pQueue, const size_t newCapacity)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pQueue == nullptr, lsR_ArgumentNull);

  if (pQueue->capacity < newCapacity)
  {
    const size_t offsetStart = pQueue->pFront - pQueue->pStart;
    const size_t offsetEnd = pQueue->pBack - pQueue->pStart;
    LS_ERROR_CHECK(lsRealloc(&pQueue->pStart, newCapacity));

    pQueue->pFront = pQueue->pStart + offsetStart;
    pQueue->pBack = pQueue->pStart + offsetEnd;
    pQueue->pLast = pQueue->pStart + pQueue->capacity;

    if (offsetStart > 0 && offsetStart + pQueue->count > pQueue->capacity)
    {
      const size_t wrappedCount = pQueue->count - (pQueue->capacity - offsetStart);
      memmove(pQueue->pLast, pQueue->pStart, wrappedCount * sizeof(T));
      pQueue->pBack = pQueue->pLast + wrappedCount;
    }

    pQueue->capacity = newCapacity;
    pQueue->pLast = pQueue->pStart + pQueue->capacity;
  }

epilogue:
  return result;
}

template <typename T>
lsResult queue_pushBack(queue<T> *pQueue, const T *pItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pQueue == nullptr || pItem == nullptr, lsR_ArgumentNull);

  if (pQueue->count + 1 >= pQueue->capacity)
    LS_ERROR_CHECK(queue_reserve(pQueue, lsMax(64ULL, pQueue->capacity * 2)));

  pQueue->count++;
  *pQueue->pBack = *pItem;
  pQueue->pBack++;

  if (pQueue->pBack == pQueue->pLast)
    pQueue->pBack = pQueue->pStart;

epilogue:
  return result;
}

template <typename T>
lsResult queue_pushBack(queue<T> *pQueue, const T &item)
{
  return queue_pushBack(pQueue, &item);
}

template <typename T>
lsResult queue_popFront(queue<T> *pQueue, _Out_ T *pItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pQueue == nullptr || pItem == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(pQueue->count == 0, lsR_ResourceNotFound);

  *pItem = *pQueue->pFront;

  pQueue->pFront++;
  pQueue->count--;

  if (pQueue->pFront == pQueue->pLast)
    pQueue->pFront = pQueue->pStart;

epilogue:
  return result;
}

template <typename T>
lsResult queue_get(const queue<T> *pQueue, const size_t index, _Out_ T *pItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pQueue == nullptr || pItem == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(index >= pQueue->count, lsR_ResourceNotFound);

  const size_t offset = pQueue->pFront - pQueue->pStart;
  *pItem = pQueue->pStart[(offset + index) % pQueue->capacity];

epilogue:
  return result;
}

template <typename T>
lsResult queue_getPtr(queue<T> *pQueue, const size_t index, _Out_ T **ppItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pQueue == nullptr || ppItem == nullptr, lsR_ArgumentNull);
  LS_ERROR_IF(index >= pQueue->count, lsR_ResourceNotFound);

  const size_t offset = pQueue->pFront - pQueue->pStart;
  *ppItem = &pQueue->pStart[(offset + index) % pQueue->capacity];

epilogue:
  return result;
}

template <typename T>
T & queue_get(queue<T> *pQueue, const size_t index)
{
  T *ret = nullptr;
  queue_getPtr(pQueue, index, &ret);
  return *ret;
}

template <typename T>
void queue_clear(queue<T> *pQueue)
{
  if (pQueue == nullptr)
    return;

  pQueue->pFront = pQueue->pBack = pQueue->pStart;
  pQueue->count = 0;
}

template <typename T>
bool queue_any(queue<T> *pQueue)
{
  if (pQueue != nullptr)
    return pQueue->pBack != pQueue->pFront;

  return false;
}

template <typename T>
void queue_destroy(queue<T> *pQueue)
{
  lsFreePtr(&pQueue->pStart);

  pQueue->pFront = nullptr;
  pQueue->pBack = nullptr;
  pQueue->pLast = nullptr;
  pQueue->capacity = 0;
  pQueue->count = 0;
}

template <typename T>
lsResult queue_clone(const queue<T> *pSource, queue<T> *pTarget)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pSource == nullptr || pTarget == nullptr, lsR_ArgumentNull);

  queue_clear(pTarget);
  LS_ERROR_CHECK(queue_reserve(pTarget, pSource->count));

  const size_t offsetStart = pSource->pFront - pSource->pStart;
  
  if (offsetStart > 0 && offsetStart + pSource->count > pSource->capacity)
  {
    const size_t wrappedCount = pSource->count - (pSource->capacity - offsetStart);
    lsMemcpy(pTarget->pFront, pSource->pFront, pSource->count - wrappedCount);
    lsMemcpy(pTarget->pFront + pSource->count - wrappedCount, pSource->pStart, wrappedCount);
  }
  else
  {
    lsMemcpy(pTarget->pFront, pSource->pFront, pSource->count);
  }

  pTarget->count = pSource->count;
  pTarget->pBack += pTarget->count;

epilogue:
  return result;
}
