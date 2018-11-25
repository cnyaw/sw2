
//
//  2D space search module.
//
//  Copyright (c) 2007 Waync Cheng.
//  All Rights Reserved.
//
//  2007/07/21 Waync created.
//

///
/// \file
/// \brief 2D space search module.
///
/// Cells module is used for quick objects search in a specified circle or rectangle
/// in 2D space.
///
/// Example:
///
/// \code
/// #include "swCells.h"
///
/// //
/// // User define object.
/// //
///
/// class MyObj {};
///
/// //
/// // Declare a Cells to manage MyObj*, coordinate data type use float.
/// //
///
/// Cells<MyObj*, float> cells;
///
/// //
/// // Initialize Cells, left-top of map is (-1024, -1024), slice 8 small cells
/// // in verticle and horizontal. Size of each small cell is (256, 256). Therefore
/// // the scope of the map is (-1024, -1024)-(1024, 1024).
/// //
///
/// cells.init(-1024, -1024, 256, 256, 8, 8);
///
/// //
/// // Add a new object to the cells, so it can be searched.
/// //
///
/// MyObj* pobj = getNewMyObj();
///
/// int_t id = cells.alloc(pobj, pobj->getX(), pobj->getY());
/// if (-1 == id)
/// { // Add failed. General cause of fail is out of map boundary.
/// }
/// else
/// { // Add success. Save the cell ID, this ID is used to hangle pobj in the cells.
/// }
///
/// //
/// // When object moves, we need to update the object's coordinate in Cells.
/// //
///
/// if (!cells.move(id, pobj->getX(), pobj->getY()))
/// { // Update may fail because of invalid ID or invalid coordinate.
/// }
///
/// //
/// // Filter to search objects in a specified area.
/// //
///
/// struct Filter
/// {
///    bool operator(const MyObj* pobj) const // User define filter.
///    {
///       //
///       // Return true to select this object, nMax minus 1. User can store pobj
///       // for later use. Return false to skip this object.
///       //
///
///       return true;
///    }
/// }
///
/// //
/// // Search in a circle, set pobj as the center, radius 125, max 100 objects,
/// // with user define filter.
/// //
///
/// cells.search(pobj->getX(), pobj->getY(), 125.0f, 100, Filter());
///
/// //
/// // Search in a rectangle area, set pobj as the center of rectangle, 100 x 100,
/// // max 100 objects, with user define filter.
///
/// cells.search(pobj->getX() - 50.0f, pobj->getY() - 50.0f, pobj->getX() + 50.0f, pobj->getY() + 50.0f, 100, Filter());
///
/// //
/// // Remove the object from Cells if it is no longer need.
/// //
///
/// cells.free(id);
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2007/07/21
///

#pragma once

#include <vector>

#include "swGeometry.h"
#include "swObjectPool.h"

namespace sw2 {

namespace impl {

template<typename ObjT, typename ValueT> struct implCellsRectFunc
{
  RECT_t<ValueT> rc;

  implCellsRectFunc(ValueT x1, ValueT y1, ValueT x2, ValueT y2) : rc(x1, y1, x2, y2)
  {
  }

  bool operator()(ObjT const& obj) const
  {
    return rc.ptInRect(POINT_t<ValueT>(obj.x, obj.y));
  }
};

template<typename ObjT, typename ValueT> struct implCellsSphereFunc
{
  ValueT x, y, radiuss;

  implCellsSphereFunc(ValueT x_, ValueT y_, ValueT radius_) : x(x_), y(y_), radiuss(radius_ * radius_)
  {
  }

  bool operator()(ObjT const& obj) const
  {
    return (x - obj.x) * (x - obj.x) + (y - obj.y) * (y - obj.y) <= radiuss;
  }
};

template<typename ObjT, typename ValueT> struct implCellsItem
{
  ObjT obj;
  int cellxy;                           // Index of cells.
  int id;                               // ID of cells[cellxy].
  ValueT x, y;                          // Current coordinate.
};

} // namespace impl

///
/// \brief 2D space search module.
///

template<typename ObjT, typename ValueT = int, uint INIT_OBJ_POOL_SIZE = 1024, uint INIT_CELL_POOL_SIZE = 1>
class Cells
{
public:

  ValueT refx, refy;                    // Origin(left-top corner) coordinate.
  ValueT cellw, cellh;                  // Cell size(width/height).
  int ncellx, ncelly;                   // Cell count.
  RECT_t<ValueT> rc;                    // Boundary.

  typedef impl::implCellsItem<ObjT, ValueT> ItemT;

  ObjectPool<ItemT, INIT_OBJ_POOL_SIZE, true> cobjs; // Manage all objects in the cells.
  std::vector<ObjectPool<int, INIT_CELL_POOL_SIZE, true> > cells; // All cells, each cell manages its' own objects list.

  ///
  /// \brief Initialize cells.
  /// \param [in] refx X of reference origin point(left-top corner).
  /// \param [in] refy Y of reference origin point(left-top corner).
  /// \param [in] cellw Width of cell.
  /// \param [in] cellh Height of cell.
  /// \param [in] ncellx Number of cells of horizontal direction.
  /// \param [in] ncelly Number of cells of vertical direction.
  ///

  void init(ValueT refx, ValueT refy, ValueT cellw, ValueT cellh, int ncellx, int ncelly)
  {
    this->refx = refx;
    this->refy = refy;
    this->cellw = cellw;
    this->cellh = cellh;
    this->ncellx = ncellx;
    this->ncelly = ncelly;
    rc = RECT_t<ValueT>(refx, refy, refx + ncellx * cellw, refy + ncelly * cellh);
    reset();
  }

  ///
  /// \brief Reset to the initial state.
  ///

  void reset()
  {
    cobjs.clear();
    cells.clear();
    cells.resize(ncellx * ncelly);
  }

  ///
  /// \brief Add an object to the cells.
  /// \param [in] obj The object(user obj)
  /// \param [in] x X coordinate of the object.
  /// \param [in] y Y coordinate of the object.
  /// \return Return -1 if fail else return an ID of the object.
  ///

  int alloc(ObjT const& obj, ValueT x, ValueT y)
  {
    if (!rc.ptInRect(POINT_t<ValueT>(x, y))) {
      return -1;
    }

    int id = cobjs.alloc();
    ItemT& i = cobjs[id];
    i.obj = obj;
    i.cellxy = getCellxy(x, y);
    i.id = cells[i.cellxy].alloc();
    i.x = x;
    i.y = y;
    cells[i.cellxy][i.id] = id;

    return id;
  }

  ///
  /// \brief Remove an object from the cells.
  /// \param [in] id ID of the object, obtained by alloc.
  /// \return Return true if success else return false.
  ///

  bool free(int id)
  {
    if (!cobjs.isUsed(id)) {
      return false;
    }

    ItemT const& i = cobjs[id];
    cells[i.cellxy].free(i.id);

    cobjs.free(id);

    return true;
  }

  ///
  /// \brief Move an object.
  /// \param [in] id ID of the object
  /// \param [in] newx New X coordinate of the object.
  /// \param [in] newy New Y coordinate of the object.
  /// \return Return true if success else return false.
  ///

  bool move(int id, ValueT newx, ValueT newy)
  {
    if (!cobjs.isUsed(id)) {
      return false;
    }

    if (!rc.ptInRect(POINT_t<ValueT>(newx, newy))) {
      return false;
    }

    int nextxy = getCellxy(newx, newy);

    ItemT& i = cobjs[id];
    if (i.cellxy != nextxy) {           // Move to next cell?
      cells[i.cellxy].free(i.id);       // Remove from prev cell.
      i.cellxy = nextxy;
      i.id = cells[i.cellxy].alloc();   // Put to new cell.
      cells[i.cellxy][i.id] = id;
    }

    i.x = newx;                         // Update new location.
    i.y = newy;

    return true;
  }

  ///
  /// \brief Search objects in a specified circle.
  /// \param [in] x X coordinate of circle center.
  /// \param [in] y Y coordinate of circle center.
  /// \param [in] radius Radius of the circle.
  /// \param [in] nMax Max number of objects to search.
  /// \param [in,out] filter An objects filter.
  /// \note A filter is used to decide an object is selected or not, the prototype
  ///       is bool operator()(ObjT const& obj). If it returns true, nMax - 1.
  ///       else false to skip the object.
  /// \note The search is from the center of circle to the outer. But the order
  ///       is not totally according to the distance. If it is need, please sort manually.
  ///

  template<typename FilterT>
  void search(ValueT x, ValueT y, ValueT radius, uint nMax, FilterT& filter) const
  {
    impl::implCellsSphereFunc<ItemT,ValueT> sf(x, y, radius);
    search(x - radius, y - radius, x + radius, y + radius, nMax, filter, sf);
  }

  ///
  /// \brief Search objects in a specified rectangle.
  /// \param [in] x1 X coordinate of left-top corner.
  /// \param [in] y1 Y coordinate of left-top corner.
  /// \param [in] x2 X coordinate of right-bottom corner.
  /// \param [in] y2 Y coordinate of right-bottom corner.
  /// \param [in] nMax Max number of objects to search.
  /// \param [in,out] filter An objects filter.
  /// \note A filter is used to decide an object is selected or not, the prototype
  ///       is bool operator()(ObjT const& obj). If it returns true, nMax - 1.
  ///       else false to skip the object.
  /// \note The search is from the center of rectangle to the outer. But the order
  ///       is not totally according to the distance. If it is need, please sort manually.
  ///

  template<typename FilterT>
  void search(ValueT x1, ValueT y1, ValueT x2, ValueT y2, uint nMax, FilterT& filter) const
  {
    impl::implCellsRectFunc<ItemT,ValueT> rf(x1, y1, x2, y2);
    search(x1, y1, x2, y2, nMax, filter, rf);
  }

private:

  int getCellxy(ValueT x, ValueT y) const
  {
    return (int)((x - refx) / cellw) + ncellx * (int)(((y - refy) / cellh));
  }

  //
  // With specified rectangle (x1,y1)-(x2,y2), search objects from the center of
  // the rectangle in down-left-up-right order circular until all cells contained
  // in the rectangle are searched or nMax objects are found.
  //

  template<typename FilterT, typename FuncT>
  void search(ValueT x1, ValueT y1, ValueT x2, ValueT y2, uint nMax, FilterT& filter, FuncT& func) const
  {
    int const BOTTOM = 0, LEFT = 1, TOP = 2, RIGHT = 3;
    const int dirx[] = {0, -1, 0, 1}, diry[] = {1, 0, -1, 0}; // Down, left, up, right.
    int lpath;                          // Total/max cell to search.
    int cdir, cc, cx, cy;               // Current direction, current cell, current cell x, current cell y.
    int bd[4];                          // Boundary.
    int cb[4];                          // Current boundary.
    RECT_t<ValueT> rcb(x1, y1, x2, y2); // Rect boundary.

    assert(x1 < x2 && y1 < y2);

    if (!rc.intersect(rcb)) {
      return;
    }

    bd[LEFT] = (int)((x1 - refx) / cellw); // Find boundary cell.
    if (0 > bd[LEFT]) {
      bd[LEFT] = 0;
    }

    bd[TOP] = (int)((y1 - refy) / cellh);
    if (0 > bd[TOP]) {
      bd[TOP] = 0;
    }

    bd[RIGHT] = (int)((x2 - refx) / cellw);
    if (ncellx <= bd[RIGHT]) {
      bd[RIGHT] = ncellx - 1;
    }

    bd[BOTTOM] = (int)((y2 - refy) / cellh);
    if (ncelly <= bd[BOTTOM]) {
      bd[BOTTOM] = ncelly - 1;
    }

    lpath = (bd[RIGHT] - bd[LEFT] + 1) * // Total/max cell to search.
            (bd[BOTTOM] - bd[TOP] + 1) - 1;
    cc = (bd[LEFT] +                    // Center cell.
         (bd[RIGHT] - bd[LEFT]) / 2) +
          ncellx * (bd[TOP] + (bd[BOTTOM] - bd[TOP]) / 2);

    cdir = 0;
    cx = cc % ncellx, cy = cc / ncellx;
    cb[BOTTOM] = cb[TOP] = cy;
    cb[LEFT] = cb[RIGHT] = cx;

    while (true) {

      //
      // Process current cell.
      //

      if (0 <= cc && ncellx * ncelly > cc) {
        const ObjectPool<int, INIT_CELL_POOL_SIZE, true> &p = cells[cc];
        for (int i = p.first(); -1 != i && 0 < nMax; i = p.next(i)) {
          ItemT const& obj = cobjs[p[i]];
          if (!func(obj)) {
            continue;
          }
          if (filter(obj.obj)) {
            nMax -= 1;
          }
        }
      }

      //
      // End of search?
      //

      if (0 >= lpath || 0 >= (int)nMax) {
        break;
      }

      //
      // Get next cell for search.
      //

      while (true) {

        int ax = cx + dirx[cdir], ay = cy + diry[cdir]; // Get next cell's coordinate.
        int nc = ax + ncellx * ay;

        switch (cdir)
        {
        case BOTTOM:            // Down.
          if (ay > cb[BOTTOM]) {
            cb[BOTTOM] = ay;
            cdir = (cdir + 1) % 4;
          }
          break;

        case LEFT:              // Left.
          if (ax < cb[LEFT]) {
            cb[LEFT] = ax;
            cdir = (cdir + 1) % 4;
          }
          break;

        case TOP:               // Up.
          if (ay < cb[TOP]) {
            cb[TOP] = ay;
            cdir = (cdir + 1) % 4;
          }
          break;

        case RIGHT:             // Right.
          if (ax > cb[RIGHT]) {
            cb[RIGHT] = ax;
            cdir = (cdir + 1) % 4;
          }
          break;
        }

        cc = nc;
        cx = ax, cy = ay;

        if (0 > ax || 0 > ay) {
          continue;
        }

        if (ax < bd[LEFT] || ax > bd[RIGHT] || ay > bd[BOTTOM] || ay < bd[TOP]) {
          continue;
        }

        lpath -= 1;
        break;
      }
    }
  }
};

} // namespace sw2

// end of swCells.h
