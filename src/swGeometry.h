
//
//  Geometry utility.
//
//  Copyright (c) 2009 Waync Cheng.
//  All Rights Reserved.
//
//  2009/09/05 Waync created.
//

///
/// \file
/// \brief Geometry utility.
/// \author Waync Cheng
/// \date 2009/09/05
///

#pragma once

#include "swinc.h"

namespace sw2 {

///
/// \brief 2D point.
///

template<typename ValueT>
struct POINT_t
{
  POINT_t(ValueT x_ = ValueT(), ValueT y_ = ValueT()) : x(x_), y(y_)
  {
  }

  ValueT x;                           ///< X coordinate of the point.
  ValueT y;                           ///< Y coordinate of the point.
};

typedef POINT_t<int> IntPoint;
typedef POINT_t<float> FloatPoint;

///
/// \brief 2D rectangle.
///

template<typename ValueT>
struct RECT_t
{
  RECT_t(const RECT_t& rc) :
    left(rc.left),
    top(rc.top),
    right(rc.right),
    bottom(rc.bottom)
  {
  }

  RECT_t(ValueT l = ValueT(), ValueT t = ValueT(), ValueT r = ValueT(), ValueT b = ValueT()) : left(l), top(t), right(r), bottom(b)
  {
  }

  ///
  /// \brief Resize the rectangle.
  /// \param [in] xval Size of width to change; >0 means enlarge, <0 means shrink, =0 no change.
  /// \param [in] yval Size of height to change; >0 means enlarge, <0 means shrink, =0 no change.
  ///

  inline void inflate(ValueT xval, ValueT yval)
  {
    left -= xval;
    top -= yval;
    right += xval;
    bottom += yval;
  }

  ///
  /// \brief Move the rectangle.
  /// \param [in] xval Horizontal offset to move.
  /// \param [in] yval Vertical offset to move.
  ///

  inline void offset(ValueT xval, ValueT yval)
  {
    left += xval;
    top += yval;
    right += xval;
    bottom += yval;
  }

  ///
  /// \brief Check is a point in this rectangle.
  /// \param [in] pt The point.
  /// \return If the point is in the rectangle then return true else return false.
  /// \note A point is exact on the right or bottom edge of the rectangle treats
  ///       outside the rectangle.
  ///

  inline bool ptInRect(const POINT_t<ValueT>& pt) const
  {
    return left <= pt.x && right > pt.x && top <= pt.y && bottom > pt.y;
  }

  ///
  /// \brief Check is a rectangle intersect with this rectangle.
  /// \param [in] rc The rectangle.
  /// \return Return true if two rectangles are intersect else return false.
  ///

  inline bool intersect(const RECT_t& rc) const
  {
    if (0 >= rc.width() || 0 >= rc.height()) {
      return false;
    } else {
      return rc.right > left && rc.bottom > top && rc.left < right && rc.top < bottom;
    }
  }

  ///
  /// \brief Check and get the intersect sub-rectangle with this rectangle.
  /// \param [in] rc The rectangle.
  /// \param [out] rcInt The intersect rectangle. If two rectangles are not intersect
  ///                    then rcInt is not defined.
  /// \return Return true if two rectangles are intersect else return false.
  ///

  inline bool intersect(const RECT_t& rc, RECT_t& rcInt) const
  {
    if (0 >= rc.width() || 0 >= rc.height()) {
      return false;
    }

    if (rc.right > left && rc.bottom > top && rc.left < right && rc.top < bottom) {
      rcInt.left = rc.left > left ? rc.left : left;
      rcInt.top = rc.top > top ? rc.top : top;
      rcInt.right = rc.right > right ? right : rc.right;
      rcInt.bottom = rc.bottom > bottom ? bottom : rc.bottom;
      return true;
    }

    return false;
  }

  //
  // \brief Check if rc is completely inside this rectangle.
  // \param[in] rc The rectangle to be tested.
  // \return Return true if rc is completely inside this rectangle else return false.
  //

  bool contain(const RECT_t &rc) const
  {
    return left <= rc.left && rc.right <= right && top <= rc.top && rc.bottom <= bottom;
  }

  inline RECT_t& operator=(const RECT_t& rc)
  {
    if (this == &rc) {
      return *this;
    }

    left = rc.left;
    top = rc.top;
    right = rc.right;
    bottom = rc.bottom;

    return *this;
  }

  inline bool operator==(const RECT_t& rc) const
  {
    return left == rc.left && top == rc.top && right == rc.right && bottom == rc.bottom;
  }

  ///
  /// \brief Get the rectangle height.
  /// \return Return the rectangle height.
  ///

  inline ValueT height() const
  {
    return bottom - top;
  }

  ///
  /// \brief Get the rectangle width.
  /// \return Return the rectangle width.
  ///

  inline ValueT width() const
  {
    return right - left;
  }

  ///
  /// \brief Check is the rectangle empty.
  /// \return Return true if the rectangle is empty(all 0) else return false.
  ///

  inline bool empty() const
  {
    ValueT z = ValueT();
    return left == z && top == z && right == z && bottom == z;
  }

  ///
  /// \brief Set the rectangle empty.
  ///

  inline void setEmpty()
  {
    ValueT z = ValueT();
    left = z;
    top = z;
    right = z;
    bottom = z;
  }

  ValueT left;                          ///< X coordinate of left-top corner.
  ValueT top;                           ///< Y coordinate of left-top corner.
  ValueT right;                         ///< X coordinate of right-bottom corner.
  ValueT bottom;                        ///< Y coordinate of right-bottom corner.
};

typedef RECT_t<int> IntRect;
typedef RECT_t<float> FloatRect;

} // namespace sw2

// end of swGeometry.h
