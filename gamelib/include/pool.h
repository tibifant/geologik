#pragma once

#include "core.h"

//////////////////////////////////////////////////////////////////////////

template <typename T>
struct pool;

template <typename T>
struct pool_iterator
{
  pool<T> *pPool = nullptr;
  size_t blockIndex = 0, blockSubIndex = 0, iteratedItem = 0;

  struct pool_item
  {
    size_t index;
    T *pItem;
    size_t _iteratedIndex;
  };

  pool_iterator(pool<T> *pPool);
  pool_item operator *();
  const pool_item operator *() const;
  bool operator != (const size_t maxCount) const;
  pool_iterator &operator++();
};

template <typename T>
struct pool
{
  size_t count = 0;
  size_t blockCount = 0;
  uint64_t *pBlockEmptyMask = nullptr;
  T **ppBlocks = nullptr;

  static constexpr size_t BlockSize = sizeof(uint64_t) * CHAR_BIT;

  inline pool_iterator<T> begin() { return pool_iterator(this); };
  inline size_t end() { return count; };

  inline struct
  {
    pool<T> *pPool;
    size_t poolIndex, iteratedIndex;

    inline pool_iterator<T> begin()
    {
      auto it = pool_iterator(pPool);

      it.iteratedItem = iteratedIndex;
      it.blockIndex = poolIndex / pool<T>::BlockSize;
      it.blockSubIndex = poolIndex % pool<T>::BlockSize;
      
      return it;
    };
    
    inline size_t end() { return pPool->count; };

  } IterateFromIteratedIndex(const size_t poolIndex, const size_t iteratedIndex) { return { this, poolIndex, iteratedIndex }; }
};

//////////////////////////////////////////////////////////////////////////

template <typename T>
lsResult pool_add(pool<T> *pPool, const T *pItem, _Out_ size_t *pIndex)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pPool == nullptr || pItem == nullptr || pIndex == nullptr, lsR_ArgumentNull);

  bool found = false;
  size_t blockIndex = 0, blockSubIndex = 0;

  // Try to find an empty spot.
  for (size_t i = 0; i < pPool->blockCount; i++)
  {
    if (pPool->pBlockEmptyMask[i] != (uint64_t)-1)
    {
      unsigned long subIndex = 0;
      lsAssert(0 != _BitScanForward64(&subIndex, ~pPool->pBlockEmptyMask[i]));
      lsAssert(((pPool->pBlockEmptyMask[i] >> subIndex) & 1) == 0);

      blockIndex = i;
      blockSubIndex = subIndex;
      found = true;

      break;
    }
  }

  // Spot found? No? Then allocate a new block!
  if (!found)
  {
    size_t newBlockCount = pPool->blockCount + 1;

    blockIndex = pPool->blockCount;
    blockSubIndex = 0;

    LS_ERROR_CHECK(lsRealloc(&pPool->pBlockEmptyMask, newBlockCount));
    LS_ERROR_CHECK(lsRealloc(&pPool->ppBlocks, newBlockCount));
    LS_ERROR_CHECK(lsAllocZero(&pPool->ppBlocks[blockIndex], pool<T>::BlockSize));

    pPool->pBlockEmptyMask[blockIndex] = 0;
    pPool->blockCount = newBlockCount;
  }

  pPool->ppBlocks[blockIndex][blockSubIndex] = *pItem;
  pPool->pBlockEmptyMask[blockIndex] |= ((uint64_t)1 << blockSubIndex);
  *pIndex = blockIndex * pool<T>::BlockSize + blockSubIndex;
  pPool->count++;

epilogue:
  return result;
}

template <typename T>
lsResult pool_add(pool<T> *pPool, const T &item, _Out_ size_t *pIndex)
{
  return pool_add(pPool, &item, pIndex);
}

template <typename T>
lsResult pool_insertAt(pool<T> *pPool, const T *pItem, const size_t index, const bool allowOverride = false)
{
  lsResult result = lsR_Success;

  const size_t blockIndex = index / pool<T>::BlockSize;
  const size_t blockSubIndex = index % pool<T>::BlockSize;

  LS_ERROR_IF(pPool == nullptr || pItem == nullptr, lsR_ArgumentNull);

  if (pPool->blockCount <= blockIndex)
  {
    size_t newBlockCount = blockIndex + 1;

    LS_ERROR_CHECK(lsRealloc(&pPool->pBlockEmptyMask, newBlockCount));
    LS_ERROR_CHECK(lsRealloc(&pPool->ppBlocks, newBlockCount));

    for (size_t i = pPool->blockCount; i < newBlockCount; i++)
    {
      LS_ERROR_CHECK(lsAllocZero(&pPool->ppBlocks[i], pool<T>::BlockSize));
      pPool->pBlockEmptyMask[i] = 0;
    }

    pPool->blockCount = newBlockCount;
  }

  const bool isOverride = pPool->pBlockEmptyMask[blockIndex] & ((uint64_t)1 << blockSubIndex);
  LS_ERROR_IF(isOverride && !allowOverride, lsR_ResourceAlreadyExists);

  pPool->ppBlocks[blockIndex][blockSubIndex] = *pItem;
  pPool->pBlockEmptyMask[blockIndex] |= ((uint64_t)1 << blockSubIndex);

  if (!isOverride)
    pPool->count++;

epilogue:
  return result;
}

template <typename T>
lsResult pool_insertAt(pool<T> *pPool, const T &item, const size_t index, const bool allowOverride = false)
{
  return pool_insertAt(pPool, &item, index, allowOverride);
}

template <typename T>
T * pool_get(pool<T> *pPool, const size_t index)
{
  const size_t blockIndex = index / pool<T>::BlockSize;
  const size_t blockSubIndex = index % pool<T>::BlockSize;

  lsAssert(pPool->blockCount > blockIndex);
  lsAssert((pPool->pBlockEmptyMask[blockIndex] >> blockSubIndex) & 1);

  return &pPool->ppBlocks[blockIndex][blockSubIndex];
}

template <typename T>
const T * pool_get(const pool<T> *pPool, const size_t index)
{
  const size_t blockIndex = index / pool<T>::BlockSize;
  const size_t blockSubIndex = index % pool<T>::BlockSize;

  lsAssert(pPool->blockCount > blockIndex);
  lsAssert((pPool->pBlockEmptyMask[blockIndex] >> blockSubIndex) & 1);

  return &pPool->ppBlocks[blockIndex][blockSubIndex];
}

template <typename T>
lsResult pool_get_safe(const pool<T> *pPool, const size_t index, _Out_ T *pItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pPool == nullptr || pItem == nullptr, lsR_ArgumentNull);

  const size_t blockIndex = index / pool<T>::BlockSize;
  const size_t blockSubIndex = index % pool<T>::BlockSize;

  LS_ERROR_IF(pPool->blockCount <= blockIndex, lsR_ResourceNotFound);
  LS_ERROR_IF((pPool->pBlockEmptyMask[blockIndex] & (1 << blockSubIndex)) == 0, lsR_ResourceNotFound);

  *pItem = pPool->ppBlocks[blockIndex][blockSubIndex];

epilogue:
  return result;
}

template <typename T>
lsResult pool_get_safe(pool<T> *pPool, const size_t index, _Out_ T **ppItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pPool == nullptr || ppItem == nullptr, lsR_ArgumentNull);

  const size_t blockIndex = index / pool<T>::BlockSize;
  const size_t blockSubIndex = index % pool<T>::BlockSize;

  LS_ERROR_IF(pPool->blockCount <= blockIndex, lsR_ResourceNotFound);
  LS_ERROR_IF((pPool->pBlockEmptyMask[blockIndex] & (1 << blockSubIndex)) == 0, lsR_ResourceNotFound);

  *ppItem = &pPool->ppBlocks[blockIndex][blockSubIndex];

epilogue:
  return result;
}

template <typename T>
lsResult pool_get_safe(const pool<T> *pPool, const size_t index, _Out_ T * const *ppItem)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pPool == nullptr || ppItem == nullptr, lsR_ArgumentNull);

  const size_t blockIndex = index / pool<T>::BlockSize;
  const size_t blockSubIndex = index % pool<T>::BlockSize;

  LS_ERROR_IF(pPool->blockCount <= blockIndex, lsR_ResourceNotFound);
  LS_ERROR_IF((pPool->pBlockEmptyMask[blockIndex] & (1 << blockSubIndex)) == 0, lsR_ResourceNotFound);

  *ppItem = &pPool->ppBlocks[blockIndex][blockSubIndex];

epilogue:
  return result;
}

template <typename T>
lsResult pool_remove_safe(pool<T> *pPool, const size_t index, _Out_ T *pItem = nullptr)
{
  lsResult result = lsR_Success;

  LS_ERROR_IF(pPool == nullptr, lsR_ArgumentNull);

  const size_t blockIndex = index / pool<T>::BlockSize;
  const size_t blockSubIndex = index % pool<T>::BlockSize;

  LS_ERROR_IF(pPool->blockCount <= blockIndex, lsR_ResourceNotFound);
  LS_ERROR_IF((pPool->pBlockEmptyMask[blockIndex] & ((uint64_t)1 << blockSubIndex)) == 0, lsR_ResourceNotFound);

  if (pItem != nullptr)
    *pItem = pPool->ppBlocks[blockIndex][blockSubIndex];

  pPool->pBlockEmptyMask[blockIndex] &= ~(uint64_t)((uint64_t)1 << blockSubIndex);
  pPool->count--;

epilogue:
  return result;
}

template <typename T>
void pool_clear(pool<T> *pPool)
{
  if (pPool == nullptr)
    return;

  for (size_t i = 0; i < pPool->blockCount; i++)
    pPool->pBlockEmptyMask[i] = 0;

  pPool->count = 0;
}

template <typename T>
void pool_destroy(pool<T> *pPool)
{
  if (pPool == nullptr)
    return;

  for (size_t i = 0; i < pPool->blockCount; i++)
    lsFreePtr(&pPool->ppBlocks[i]);

  lsFreePtr(&pPool->ppBlocks);
  lsFreePtr(&pPool->pBlockEmptyMask);

  pPool->blockCount = 0;
  pPool->count = 0;
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline pool_iterator<T>::pool_iterator(pool<T> *pPool) :
  pPool(pPool)
{
  if (pPool == nullptr || pPool->count == 0)
    return;

  while (true)
  {
    if (pPool->pBlockEmptyMask[blockIndex] == 0)
    {
      blockIndex++;

      lsAssert(blockIndex < pPool->blockCount);
    }
    else
    {
      unsigned long subIndex = 0;

      if (0 == _BitScanForward64(&subIndex, pPool->pBlockEmptyMask[blockIndex]))
      {
        blockIndex++;
        blockSubIndex = 0;
        lsAssert(blockIndex < pPool->blockCount);
      }
      else
      {
        blockSubIndex += subIndex;
        lsAssert(blockSubIndex < pool<T>::BlockSize);
        break;
      }
    }
  }
}

template<typename T>
inline typename pool_iterator<T>::pool_item pool_iterator<T>::operator*()
{
  pool_iterator<T>::pool_item ret;
  ret.index = blockIndex * pool<T>::BlockSize + blockSubIndex;
  ret.pItem = &pPool->ppBlocks[blockIndex][blockSubIndex];
  ret._iteratedIndex = iteratedItem;

  return ret;
}

template<typename T>
inline const typename pool_iterator<T>::pool_item pool_iterator<T>::operator*() const
{
  pool_iterator<T>::pool_item ret;
  ret.index = blockIndex * pool<T>::BlockSize + blockSubIndex;
  ret.pItem = &pPool->ppBlocks[blockIndex][blockSubIndex];
  ret._iteratedIndex = iteratedItem;
  d
  return ret;
}

template<typename T>
inline bool pool_iterator<T>::operator!=(const size_t maxCount) const
{
  return iteratedItem < maxCount;
}

template<typename T>
inline pool_iterator<T> &pool_iterator<T>::operator++()
{
  if (iteratedItem + 1 < pPool->count)
  {
    blockSubIndex++;

    while (true)
    {
      if (pPool->pBlockEmptyMask[blockIndex] == 0)
      {
        blockIndex++;

        lsAssert(blockIndex < pPool->blockCount);
      }
      else
      {
        unsigned long subIndex = 0;
        const uint64_t shift = blockSubIndex;

        if (shift >= 0x40 || 0 == _BitScanForward64(&subIndex, pPool->pBlockEmptyMask[blockIndex] >> shift))
        {
          blockIndex++;
          blockSubIndex = 0;
          lsAssert(blockIndex < pPool->blockCount);
        }
        else
        {
          blockSubIndex += subIndex;
          lsAssert(blockSubIndex < pool<T>::BlockSize);
          break;
        }
      }
    }
  }

  iteratedItem++;

  return *this;
}
