
//
//  Object pool.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/02/21 Waync created.
//

///
/// \file
/// \brief Object pool.
///
/// ObjectPool is an array, but also like a list. It manages objects like a list,
/// and access objects like an array. Advantage of an object pool is to reuse
/// the pool entities and it can auto grow the capacity when pool is out of space.
/// Also because of the list characteristic, iteration is faster than a simple
/// array.
///
/// Example:
///
/// \code
/// #include "swObjectPool.h"
///
/// //
/// // User define object.
/// //
///
/// class MyObj {};
///
/// //
/// // Declare pool capacity 32(fixed) to manager MyObj.
/// //
///
/// ObjectPool<MyObj,32> myPool;
///
/// //
/// // Allocate a free entity.
/// //
///
/// int_t id = myPool.alloc();
/// if (-1 == id)
/// { // Allocate fail.
/// }
/// else
/// { // Allocate success.
/// }
///
/// //
/// // Manipulate/use the object.
/// //
///
/// MyObj& obj = myPool[id]; // Use the ID as an array index to get the object.
/// ... // Use the object.
///
/// //
/// // Iterate the pool, in general way.
/// //
///
/// for (int i = myPool.first(); -1 != i; i = myPool.next(i))
/// { // Do something to object myPool[i].
/// }
///
/// //
/// // Iterate the pool, some entity will be freed during iteration.
/// //
///
/// for (int i = myPool.first(); -1 != i;)
/// {
///   int_t next = myPool.next(i);
///   // Do something to object myPool[i], it may be freed here.
///   i = next;
/// }
///
/// //
/// // Release unused entity.
/// //
///
/// myPool.free(id);
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2006/03/15
///

#pragma once

#include "swinc.h"

namespace sw2 {

///
/// \brief Object pool.
///

template<class PoolT, class T>
class ObjectPoolBase
{
protected:
  int mCapacity;
  T* mEntity;                           // Entities, object pool.

  int mNumUsed;                         // Number used entity.
  int mFree1, mFreeN;                   // Free list head and tail.
  int mUsed1, mUsedN;                   // Free list head and tail.

  int* mLstNext;                        // Next list for free and used list.
  int* mLstPrev;                        // Previous list for free and used list.
  bool* mLstUsed;                       // IsUsed flag.

public:

  ///
  /// \brief Get free entity count.
  /// \return Return free entity count.
  ///

  int available() const
  {
    return capacity() - mNumUsed;
  }

  ///
  /// \brief Get pool capacity.
  /// \return Return pool capacity.
  ///

  int capacity() const
  {
    return mCapacity;
  }

  ///
  /// \brief Get used entity count.
  /// \return Return used entity count.
  ///

  int size() const
  {
    return mNumUsed;
  }

  ///
  /// \brief Check is an entity used.
  /// \param [in] index The ID(index) of entity.
  /// \return Return true if the entity is used else return false.
  ///

  bool isUsed(int index) const
  {
    if (0 > index || mCapacity <= index) {
      return false;
    } else {
      return mLstUsed[index];
    }
  }

  ///
  /// \brief Allocate a free entity.
  /// \return Return a new ID(index) if success else return -1.
  /// \note If the pool is full and is not fixed size(AUTO_GROW=true). Then the
  ///       pool will double its capacity and return a free entity.
  ///

  int alloc()
  {
    int found = mFree1;
    if (-1 == found) {
      return -1;
    }

    mFree1 = mLstNext[found];
    if (-1 == mFree1) {
      mFreeN = -1;
    } else {
      mLstPrev[mFree1] = -1;
    }

    mLstNext[found] = -1;
    if (-1 != mUsedN) {
      mLstNext[mUsedN] = found;
    }

    mLstPrev[found] = mUsedN;
    if (-1 == mUsed1) {
      mUsed1 = found;
    }

    mUsedN = found;

    mLstUsed[found] = true;
    mNumUsed += 1;

    return found;
  }

  ///
  /// \brief Allocate a free entity with specified ID(index).
  /// \param [in] index Specified ID(index).
  /// \return Return a new ID(index) if success else return -1.
  /// \note If the specified ID is used then return faild(-1).
  ///

  int alloc(int index)
  {
    if (mLstUsed[index]) {
      return -1;
    }

    if (mFree1 == index) {
      return alloc();
    }

    int nexti = mLstNext[index], previ = mLstPrev[index];
    if (-1 != previ) {
      mLstNext[previ] = nexti;
    }

    if (-1 != nexti) {
      mLstPrev[nexti] = previ;
    }

    mLstNext[index] = mFree1;
    mLstPrev[mFree1] = index;
    mFree1 = index;

    if (index == mFreeN) {
      mFreeN = mLstPrev[mFreeN];
    }

    mLstPrev[index] = -1;

    return alloc();
  }

  ///
  /// \brief Release a unused entity.
  /// \param [in] index The entity ID(index).
  ///

  void free(int index)
  {
    if (!isUsed(index)) {
      return;
    }

    int unext = mLstNext[index], uprev = mLstPrev[index];
    if (-1 != uprev) {
      mLstNext[uprev] = unext;
    }

    if (-1 != unext) {
      mLstPrev[unext] = uprev;
    }

    if (index == mUsed1) {
      mUsed1 = unext;
    }

    if (index == mUsedN) {
      mUsedN = uprev;
    }

    mLstUsed[index] = false;
    mNumUsed -= 1;

    mLstNext[index] = -1;
    if (-1 != mFreeN) {
      mLstNext[mFreeN] = index;
    }

    mLstPrev[index] = mFreeN;
    if (-1 == mFree1) {
      mFree1 = index;
    }

    mFreeN = index;
  }

  ///
  /// \brief Reset pool to initial state.
  /// \note Reset is similar to clear, except reset also changes to initial state.
  ///

  void reset()
  {
    mNumUsed = 0, mFree1 = 0, mFreeN = capacity() - 1, mUsed1 = -1, mUsedN = -1;
    for (int i = 0; i < capacity(); i++) {
      mLstNext[i] = (capacity() - 1 == i) ? -1 : i + 1;
      mLstPrev[i] = 0 == i ? -1 : i - 1;
      mLstUsed[i] = false;
    }
  }

  ///
  /// \brief Free all used entities.
  /// \note After clear, the order of free entities are undefined.
  ///

  void clear()
  {
    while (-1 != first()) {
      free(first());
    }
  }

  ///
  /// \brief Exchange two entities(only order in the list change, no content change).
  /// \param [in] a The ID(index) of entity a.
  /// \param [in] b The ID(index) of entity b.
  /// \return Return true if success else return false.
  ///

  bool swap(int a, int b)
  {
    if (a == b || !isUsed(a) || !isUsed(b)) {
      return false;
    }

    int an = mLstNext[a], ap = mLstPrev[a];
    int bn = mLstNext[b], bp = mLstPrev[b];

    if (-1 != ap) {
      mLstNext[ap] = ap == b ? -1 : b;
    } else {
      mUsed1 = b;
    }

    if (-1 != an) {
      mLstPrev[an] = an == b ? -1 : b;
    } else {
      mUsedN = b;
    }

    mLstNext[b] = b == an ? a : an;
    mLstPrev[b] = b == ap ? a : ap;

    if (-1 != bp) {
      mLstNext[bp] = bp == a ? -1 : a;
    } else {
      mUsed1 = a;
    }

    if (-1 != bn) {
      mLstPrev[bn] = bn == a ? -1 : a;
    } else {
      mUsedN = a;
    }

    mLstNext[a] = a == bn ? b : bn;
    mLstPrev[a] = a == bp ? b : bp;

    return true;
  }

  ///
  /// \brief Insert an entity before a specified entity.
  /// \param [in] idPos The entity ID(pos) to insert before.
  /// \param [in] id The entity ID(index) to be insert.
  /// \return Return true if success else return false.
  /// \note If idPos = -1 or invalid then id will append to the end of list.
  ///

  bool insert(int idPos, int id)
  {
    if (!isUsed(id)) {
      return false;
    }

    if (idPos == id) {
      return true;
    }

    //
    // Insert end.
    //

    if (!isUsed(idPos)) {
      if (last() == id) {               // Already at that position.
        return true;
      }

      free(id);

      int a = alloc(id);                // Add to end.
      assert(a == id);

      return true;
    }

    if (prev(idPos) == id) {            // id already before position.
      return true;
    }

    //
    // Unlink id.
    //

    int first = mUsed1;

    if (-1 != mLstNext[id]) {           // id is last one?
      mLstPrev[mLstNext[id]] = mLstPrev[id]; // Next(id).prev = prev(id).
    } else {
      mUsedN = mLstPrev[id];
    }

    if (first == id) {                  // id is first one?
      mUsed1 = mLstNext[id];
    } else {
      mLstNext[mLstPrev[id]] = mLstNext[id]; // Prev(id).next = next(id).
    }

    //
    // Insert id before idPos.
    //

    if (first == idPos) {
      mUsed1 = id;
    } else {
      mLstNext[mLstPrev[idPos]] = id;   // Prev(idPos).next = id.
    }

    mLstNext[id] = idPos;               // Next(id) = idPos.

    mLstPrev[id] = mLstPrev[idPos];     // Prev(id) = prev(idPos).
    mLstPrev[idPos] = id;               // Prev(idPos) = id.

    return true;
  }

  ///
  /// \brief Get first used entity.
  /// \return Return first used entity.
  ///

  int first() const
  {
    return mUsed1;
  }

  ///
  /// \brief Get next used entity.
  /// \param [in] cursor Current entity ID(index).
  /// \return Return next used entity.
  ///

  int next(int cursor) const
  {
    assert(isUsed(cursor));
    return mLstNext[cursor];
  }

  ///
  /// \brief Get last used entity.
  /// \return Return last used entity.
  ///

  int last() const
  {
    return mUsedN;
  }

  ///
  /// \brief Get previous used entity.
  /// \param [in] cursor Current entity ID(index).
  /// \return Return previous used entity.
  ///

  int prev(int cursor) const
  {
    assert(isUsed(cursor));
    return mLstPrev[cursor];
  }

  ///
  /// \brief Get object of the specified entity.
  /// \param [in] index The entity ID(index) of object.
  /// \return Return the reference to the specified entity.
  ///

  T& operator[](int index)
  {
    //
    // See Effective C++ 3rd, Ch1-Item3.
    //

    return const_cast<T&>(static_cast<PoolT const&>(*this)[index]);
  }

  const T& operator[](int index) const
  {
    assert(isUsed(index));
    return mEntity[index];
  }
};

template<typename T, size_t INIT_SIZE = 16, bool AUTO_GROW = false>
class ObjectPool : public ObjectPoolBase<ObjectPool<T, INIT_SIZE, AUTO_GROW>, T>
{
  typedef ObjectPoolBase<ObjectPool<T, INIT_SIZE, AUTO_GROW>, T> Base;

  void copy_(ObjectPool const& pool)
  {
    grow_(pool.mCapacity);

    for (int i = 0; i < pool.mCapacity; i++) {
      Base::mEntity[i] = pool.mEntity[i];
    }

    ::memcpy(Base::mLstNext, pool.mLstNext, Base::mCapacity * sizeof(int));
    ::memcpy(Base::mLstPrev, pool.mLstPrev, Base::mCapacity * sizeof(int));
    ::memcpy(Base::mLstUsed, pool.mLstUsed, Base::mCapacity * sizeof(bool));

    Base::mNumUsed = pool.mNumUsed;
    Base::mFree1 = pool.mFree1;
    Base::mFreeN = pool.mFreeN;
    Base::mUsed1 = pool.mUsed1;
    Base::mUsedN = pool.mUsedN;
  }

  void grow_(int size)
  {
    if (0 == size) {
      return;
    }

    int newSize = size;

    //
    // Re-allocate new pool.
    //

    T *pT = 0;
    int *pLstNext = 0, *pLstPrev = 0;
    bool *pLstUsed = 0;

    pT = new T [newSize];
    if (0 == pT) {
      goto failed;
    }

    pLstNext = new int [newSize];
    if (0 == pLstNext) {
      goto failed;
    }

    pLstPrev = new int [newSize];
    if (0 == pLstPrev) {
      goto failed;
    }

    pLstUsed = new bool [newSize];
    if (0 == pLstUsed) {
      goto failed;
    }

    //
    // Duplicate old pool settings to new pool.
    //

    if (0 < Base::mCapacity) {
      for (int i = 0; i < Base::mCapacity; i++) {
        pT[i] = Base::mEntity[i];
      }
      ::memcpy(pLstNext, Base::mLstNext, Base::mCapacity * sizeof(int));
      ::memcpy(pLstPrev, Base::mLstPrev, Base::mCapacity * sizeof(int));
      ::memcpy(pLstUsed, Base::mLstUsed, Base::mCapacity * sizeof(bool));
    }

    //
    // Initialize new pool.
    //

    for (int i = Base::mCapacity; i < newSize; i++) {
      pLstNext[i] = (newSize - 1 == i) ? -1 : i + 1;
      pLstPrev[i] = 0 == i ? -1 : i - 1;
      pLstUsed[i] = false;
    }

    if (-1 != Base::mFreeN) {
      pLstNext[Base::mFreeN] = Base::mCapacity;
      pLstPrev[Base::mCapacity] = Base::mFreeN;
    }

    if (-1 == Base::mFree1) {
      Base::mFree1 = Base::mCapacity;
    }

    Base::mFreeN = newSize - 1;

    //
    // Done.
    //

    Base::mCapacity = newSize;

    delete [] Base::mEntity;
    delete [] Base::mLstNext;
    delete [] Base::mLstPrev;
    delete [] Base::mLstUsed;

    Base::mEntity = pT;
    Base::mLstNext = pLstNext;
    Base::mLstPrev = pLstPrev;
    Base::mLstUsed = pLstUsed;

    return;

failed:
    delete [] pT;
    delete [] pLstNext;
    delete [] pLstPrev;
    delete [] pLstUsed;
  }

public:

  ObjectPool()
  {
    Base::mEntity = 0;
    Base::mCapacity = Base::mNumUsed = 0;
    Base::mFree1 = Base::mFreeN = Base::mUsed1 = Base::mUsedN = -1;
    Base::mLstNext = Base::mLstPrev = 0;
    Base::mLstUsed = 0;
    grow_(INIT_SIZE);
  }

  ObjectPool(ObjectPool const& pool)
  {
    Base::mEntity = 0;
    Base::mCapacity = Base::mNumUsed = 0;
    Base::mFree1 = Base::mFreeN = Base::mUsed1 = Base::mUsedN = -1;
    Base::mLstNext = Base::mLstPrev = 0;
    Base::mLstUsed = 0;
    copy_(pool);
  }

  ~ObjectPool()
  {
    delete [] Base::mEntity, Base::mEntity = 0;
    delete [] Base::mLstNext, Base::mLstNext = 0;
    delete [] Base::mLstPrev, Base::mLstPrev = 0;
    delete [] Base::mLstUsed, Base::mLstUsed = 0;

    Base::mCapacity = Base::mNumUsed = 0;
    Base::mFree1 = Base::mUsed1 = Base::mFreeN = Base::mUsedN = -1;
  }

  ObjectPool& operator=(ObjectPool const& pool)
  {
    if (&pool == this) {
      return *this;
    }

    this->~ObjectPool();
    copy_(pool);
    return *this;
  }

  int alloc()
  {
    if (-1 == Base::mFree1) {
      grow_(2 * Base::mCapacity);
    }

    return Base::alloc();
  }

  int alloc(int index)
  {
    if (0 > index) {
      return -1;
    }

    while (Base::mCapacity <= index) {
      grow_(2 * Base::mCapacity);
    }

    return Base::alloc(index);
  }
};

template<typename T, size_t POOL_SIZE>
class ObjectPool<T, POOL_SIZE, false> : public ObjectPoolBase<ObjectPool<T, POOL_SIZE, false>, T>
{
  T mEntityInst[POOL_SIZE];             // Entities, object pool.

  int mLstNextInst[POOL_SIZE];          // Next list for free and used list.
  int mLstPrevInst[POOL_SIZE];          // Previous list for free and used list.
  bool mLstUsedInst[POOL_SIZE];         // IsUsed flag.

  typedef ObjectPoolBase<ObjectPool, T> Base;

public:

  ObjectPool()
  {
    Base::mEntity = mEntityInst;
    Base::mLstNext = mLstNextInst;
    Base::mLstPrev = mLstPrevInst;
    Base::mLstUsed = mLstUsedInst;
    Base::mCapacity = POOL_SIZE;
    Base::reset();
  }

  int alloc()
  {
    if (-1 == Base::mFree1) {
      return -1;
    }

    return Base::alloc();
  }

  int alloc(int index)
  {
    if (0 > index || (int)POOL_SIZE <= index) {
      return -1;
    }

    if (Base::isUsed(index)) {
      return -1;
    }

    return Base::alloc(index);
  }
};

} // namespace sw2

// end of swObjectPool.h
