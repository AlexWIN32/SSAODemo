/*******************************************************************************
    Author: Alexey Frolov (alexwin32@mail.ru)

    This software is distributed freely under the terms of the MIT License.
    See "LICENSE" or "http://copyfree.org/content/standard/licenses/mit/license.txt".
*******************************************************************************/

#pragma once

template<class T>
struct BaseVector2;

template<class T>
struct BasePoint2;

template<typename T>
struct Size;

template<class T>
struct Range;

template<class T>
struct BaseVector3;

template<class T>
struct BasePoint3;

template<class T>
struct BasePoint4;

template<class T> 
using BaseVector4 = BasePoint4<T>; 

template<class T>
struct Size3;

template<class T>
struct BaseColor;

typedef BaseVector4<float> Vector4;
typedef BasePoint4<int> Point4;
typedef BasePoint4<float> Point4F;
typedef BaseVector3<float> Vector3;
typedef BasePoint3<int> Point3;
typedef BasePoint3<float> Point3F;
typedef BasePoint3<unsigned short> Point3US;
typedef BaseVector2<float> Vector2;
typedef BasePoint2<float> Point2F;
typedef BasePoint2<int> Point2;
typedef BasePoint2<unsigned short> Point2US;
typedef Size<unsigned short> SizeUS;
typedef Size<float> SizeF;
typedef Size3<float> Size3F;
typedef Size3<unsigned short> Size3US;
typedef Range<float> RangeF;
typedef Range<unsigned short> RangeUS;
typedef Range<SizeUS> RangeSizeUS;
typedef Range<Vector2> RangeVector2;
typedef BaseColor<float> ColorF;
typedef BaseColor<unsigned char> ColorUC;