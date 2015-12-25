
#ifndef includeguard_vec_mat_hpp_includeguard
#define includeguard_vec_mat_hpp_includeguard

#include <limits>
#include <cmath>
#include <algorithm>
#include <type_traits>

// --------------------------------------------------------------------------

template <typename T> struct vec2
{
  enum { ELEMENT_COUNT = 2 };

  union
  {
    struct
    {
      T x, y;
    };
    T xy[ELEMENT_COUNT];
  };

  vec2 (void) = default;

  template <typename XY>
  vec2 (XY xy) : x (xy), y (xy) { }

  template <typename X, typename Y>
  vec2 (X xx, Y yy) : x (xx), y (yy) { }

  template <typename S> explicit vec2 (const vec2<S>& rhs)
  : x (rhs.x), y (rhs.y) { }

  template <typename S> explicit operator vec2<S> () const
  {
    return vec2<S> ((S)x, (S)y);
  }

  friend vec2 operator + (const vec2& a, const vec2& b)
  {
    return vec2 (a.x + b.x, a.y + b.y);
  }
  template <typename S> friend vec2 operator + (const vec2& a, S b)
  {
    return vec2 (a.x + b, a.y + b);
  }
  template <typename S> friend vec2 operator + (S a, const vec2& b)
  {
    return b + a;
  }
  vec2& operator += (const vec2& b)
  {
    *this = *this + b;
    return *this;
  }
  template <typename S> vec2& operator += (S b)
  {
    *this = *this + b;
    return *this;
  }

  friend vec2 operator - (const vec2& a, const vec2& b)
  {
    return vec2 (a.x - b.x, a.y - b.y);
  }
  template <typename S> friend vec2 operator - (const vec2& a, S b)
  {
    return vec2 (a.x - b, a.y - b);
  }
  template <typename S> friend vec2 operator - (S a, const vec2& b)
  {
    return vec2 (a - b.x, a - b.y);
  }
  vec2& operator -= (const vec2& b)
  {
    *this = *this - b;
    return *this;
  }
  template <typename S> vec2& operator -= (S b)
  {
    *this = *this - b;
    return *this;
  }

  friend vec2 operator * (const vec2& a, const vec2& b)
  {
    return vec2 (a.x * b.x, a.y * b.y);
  }
  template <typename S> friend vec2 operator * (const vec2& a, S b)
  {
    return vec2 (a.x * b, a.y * b);
  }
  template <typename S> friend vec2 operator * (S a, const vec2& b)
  {
    return b * a;
  }
  vec2& operator *= (const vec2& b)
  {
    *this = *this * b;
    return *this;
  }
  template <typename S> vec2& operator *= (S b)
  {
    *this = *this * b;
    return *this;
  }

  friend vec2 operator / (const vec2& a, const vec2& b)
  {
    return vec2 (a.x / b.x, a.y / b.y);
  }
  template <typename S> friend vec2 operator / (const vec2& a, S b)
  {
    return vec2 (a.x / b, a.y / b);
  }
  template <typename S> friend vec2 operator / (S a, const vec2& b)
  {
    return vec2 (a / b.x, a / b.y);
  }
  vec2& operator /= (const vec2& b)
  {
    *this = *this / b;
    return *this;
  }
  template <typename S> vec2& operator /= (S b)
  {
    *this = *this / b;
    return *this;
  }

  vec2 operator - (void) const
  {
    return vec2 (-x, -y);
  }

  vec2 operator + (void) const
  {
    return *this;
  }

  friend vec2 operator << (const vec2& a, unsigned int b)
  {
    return { a.x << b, a.y << b };
  }
  friend vec2 operator << (unsigned int a, const vec2& b)
  {
    return { a << b.x, a << b.y };
  }
  vec2& operator <<= (unsigned int b)
  {
    return *this = *this << b;
  }

  friend vec2 operator >> (const vec2& a, unsigned int b)
  {
    return { a.x >> b, a.y >> b };
  }
  friend vec2 operator >> (unsigned int a, const vec2& b)
  {
    return { a >> b.x, a >> b.y };
  }
  vec2& operator >>= (unsigned int b)
  {
    return *this = *this >> b;
  }
};

template <typename T> T inline dot (const vec2<T>& a, const vec2<T>& b)
{
  return a.x * b.x + a.y * b.y;
}

template <typename T> T inline length (const vec2<T>& a)
{
  return std::sqrt (dot (a, a));
}

template <typename T> vec2<T> inline normalize (const vec2<T>& a)
{
  return a * (T (1) / length (a));
}

template <typename T> vec2<T> inline orthogonal (const vec2<T>& a)
{
  return vec2<T> (a.y, -a.x);
}

namespace std
{
template <typename T> vec2<T> inline min (const vec2<T>& a, const vec2<T>& b)
{
  return vec2<T> (std::min (a.x, b.x), std::min (a.y, b.y));
}

template <typename T> vec2<T> inline max (const vec2<T>& a, const vec2<T>& b)
{
  return vec2<T> (std::max (a.x, b.x), std::max (a.y, b.y));
}

} // namespace std

// --------------------------------------------------------------------------

template <typename T> struct vec3
{
  enum { ELEMENT_COUNT = 3 };

  union
  {
    struct
    {
      T x, y, z;
    };
    T xyz[ELEMENT_COUNT];
  };

  vec3 (void) = default;

  template <typename XYZ>
  vec3 (XYZ xyz) : x (xyz), y (xyz), z (xyz) { }

  template <typename X, typename Y, typename Z>
  vec3 (X xx, Y yy, Z zz)
  : x (xx), y (yy), z (zz) { }

  template <typename XY, typename Z> vec3 (const vec2<XY>& xy, Z zz)
  : x (xy.x), y (xy.y), z (zz) { }

  template <typename S> explicit vec3 (const vec3<S>& rhs)
  : x (rhs.x), y (rhs.y), z (rhs.z) { }

  template <typename S> explicit operator vec3<S> () const
  {
    return vec3<S> ((S)x, (S)y, (S)z);
  }

  vec2<T> xy (void) const
  {
    return vec2<T> (x, y);
  }


  friend vec3 operator + (const vec3& a, const vec3& b)
  {
    return vec3 (a.x + b.x, a.y + b.y, a.z + b.z);
  }
  template <typename S> friend vec3 operator + (const vec3& a, S b)
  {
    return vec3 (a.x + b, a.y + b, a.z + b);
  }
  template <typename S> friend vec3 operator + (S a, const vec3& b)
  {
    return b + a;
  }
  vec3& operator += (const vec3& b)
  {
    *this = *this + b;
    return *this;
  }
  template <typename S> vec3& operator += (S b)
  {
    *this = *this + b;
    return *this;
  }

  friend vec3 operator - (const vec3& a, const vec3& b)
  {
    return vec3 (a.x - b.x, a.y - b.y, a.z - b.z);
  }
  template <typename S> friend vec3 operator - (const vec3& a, S b)
  {
    return vec3 (a.x - b, a.y - b, a.z - b);
  }
  template <typename S> friend vec3 operator - (S a, const vec3& b)
  {
    return vec3 (a - b.x, a - b.y, a - b.z);
  }
  vec3& operator -= (const vec3& b)
  {
    *this = *this - b;
    return *this;
  }
  template <typename S> vec3& operator -= (S b)
  {
    *this = *this - b;
    return *this;
  }

  friend vec3 operator * (const vec3& a, const vec3& b)
  {
    return vec3 (a.x * b.x, a.y * b.y, a.z * b.z);
  }
  template <typename S> friend vec3 operator * (const vec3& a, S b)
  {
    return vec3 (a.x * b, a.y * b, a.z * b);
  }
  template <typename S> friend vec3 operator * (S a, const vec3& b)
  {
    return b * a;
  }
  vec3& operator *= (const vec3& b)
  {
    *this = *this * b;
    return *this;
  }
  template <typename S> vec3& operator *= (S b)
  {
    *this = *this * b;
    return *this;
  }

  friend vec3 operator / (const vec3& a, const vec3& b)
  {
    return vec3 (a.x / b.x, a.y / b.y, a.z / b.z);
  }
  template <typename S> friend vec3 operator / (const vec3& a, S b)
  {
    return vec3 (a.x / b, a.y / b, a.z / b);
  }
  template <typename S> friend vec3 operator / (S a, const vec3& b)
  {
    return vec3 (a / b.x, a / b.y, a / b.z);
  }
  vec3& operator /= (const vec3& b)
  {
    *this = *this / b;
    return *this;
  }
  template <typename S> vec3& operator /= (S b)
  {
    *this = *this / b;
    return *this;
  }

  vec3 operator - (void) const
  {
    return vec3 (-x, -y, -z);
  }

  vec3 operator + (void) const
  {
    return *this;
  }

  friend vec3 operator << (const vec3& a, unsigned int b)
  {
    return { a.x << b, a.y << b, a.z << b };
  }
  friend vec3 operator << (unsigned int a, const vec3& b)
  {
    return { a << b.x, a << b.y, a << b.z };
  }
  vec3& operator <<= (unsigned int b)
  {
    return *this = *this << b;
  }

  friend vec3 operator >> (const vec3& a, unsigned int b)
  {
    return { a.x >> b, a.y >> b, a.z >> b };
  }
  friend vec3 operator >> (unsigned int a, const vec3& b)
  {
    return { a >> b.x, a >> b.y, a >> b.z };
  }
  vec3& operator >>= (unsigned int b)
  {
    return *this = *this >> b;
  }

};

template <typename T> T inline dot (const vec3<T>& a, const vec3<T>& b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

// left-hand cross product
template <typename T> vec3<T> inline cross (const vec3<T>& a, const vec3<T>& b)
{
  return vec3<T> (a.y * b.z - a.z * b.y,
                  a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x);
}

template <typename T> T inline length (const vec3<T>& a)
{
  return std::sqrt (dot (a, a));
}

template <typename T> vec3<T> inline normalize (const vec3<T>& a)
{
  return a * (T (1) / length (a));
}

namespace std
{
template <typename T> vec3<T> inline min (const vec3<T>& a, const vec3<T>& b)
{
  return vec3<T> (std::min (a.x, b.x), std::min (a.y, b.y), std::min (a.z, b.z));
}

template <typename T> vec3<T> inline max (const vec3<T>& a, const vec3<T>& b)
{
  return vec3<T> (std::max (a.x, b.x), std::max (a.y, b.y), std::max (a.z, b.z));
}

} // namespace std

// --------------------------------------------------------------------------

template <typename T> struct vec4
{
  enum { ELEMENT_COUNT = 4 };

  union
  {
    struct
    {
      T x, y, z, w;
    };
    T xyzw[ELEMENT_COUNT];
  };

  vec4 (void) = default;

  template <typename XYZW>
  vec4 (XYZW xyzw) : x (xyzw), y (xyzw), z (xyzw), w (xyzw) { }

  template <typename X, typename Y, typename Z, typename W>
  vec4 (X xx, Y yy, Z zz, W ww)
  : x (xx), y (yy), z (zz), w (ww) { }

  template <typename XYZ, typename W> vec4 (const vec3<XYZ>& xyz, W ww)
  : x (xyz.x), y (xyz.y), z (xyz.z), w (ww) { }

  template <typename XY, typename ZW> vec4 (const vec2<XY>& xy, const vec2<ZW>& zw)
  : x (xy.x), y (xy.y), z (zw.x), w (zw.y) { }

  template <typename S> explicit vec4 (const vec4<S>& rhs)
  : x (rhs.x), y (rhs.y), z (rhs.z), w (rhs.w) { }

  template <typename S> explicit operator vec4<S> () const
  {
    return vec4<S> ((S)x, (S)y, (S)z, (S)w);
  }

  vec2<T> xy (void) const { return vec2<T> (x, y); }
  vec3<T> xyz (void) const { return vec3<T> (x, y, z); }

  friend vec4 operator + (const vec4& a, const vec4& b)
  {
    return vec4 (a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
  }
  template <typename S> friend vec4 operator + (const vec4& a, S b)
  {
    return vec4 (a.x + b, a.y + b, a.z + b, a.w + b);
  }
  template <typename S> friend vec4 operator + (S a, const vec4& b)
  {
    return b + a;
  }
  vec4& operator += (const vec4& b)
  {
    *this = *this + b;
    return *this;
  }
  template <typename S> vec4& operator += (S b)
  {
    *this = *this + b;
    return *this;
  }

  friend vec4 operator - (const vec4& a, const vec4& b)
  {
    return vec4 (a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
  }
  template <typename S> friend vec4 operator - (const vec4& a, S b)
  {
    return vec4 (a.x - b, a.y - b, a.z - b, a.w - b);
  }
  template <typename S> friend vec4 operator - (S a, const vec4& b)
  {
    return vec4 (a - b.x, a - b.y, a - b.z, a - b.w);
  }
  vec4& operator -= (const vec4& b)
  {
    *this = *this - b;
    return *this;
  }
  template <typename S> vec4& operator -= (S b)
  {
    *this = *this - b;
    return *this;
  }

  friend vec4 operator * (const vec4& a, const vec4& b)
  {
    return vec4 (a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
  }
  template <typename S> friend vec4 operator * (const vec4& a, S b)
  {
    return vec4 (a.x * b, a.y * b, a.z * b, a.w * b);
  }
  template <typename S> friend vec4 operator * (S a, const vec4& b)
  {
    return b * a;
  }
  vec4& operator *= (const vec4& b)
  {
    *this = *this * b;
    return *this;
  }
  template <typename S> vec4& operator *= (S b)
  {
    *this = *this * b;
    return *this;
  }

  friend vec4 operator / (const vec4& a, const vec4& b)
  {
    return vec4 (a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
  }
  template <typename S> friend vec4 operator / (const vec4& a, S b)
  {
    return vec4 (a.x / b, a.y / b, a.z / b, a.w / b);
  }
  template <typename S> friend vec4 operator / (S a, const vec4& b)
  {
    return vec4 (a / b.x, a / b.y, a / b.z, a / b.w);
  }
  vec4& operator /= (const vec4& b)
  {
    *this = *this / b;
    return *this;
  }
  template <typename S> vec4& operator /= (S b)
  {
    *this = *this / b;
    return *this;
  }

  vec4 operator - (void) const
  {
    return vec4 (-x, -y, -z, -w);
  }

  vec4 operator + (void) const
  {
    return *this;
  }

  friend vec4 operator << (const vec4& a, unsigned int b)
  {
    return { a.x << b, a.y << b, a.z << b, a.w << b };
  }
  friend vec4 operator << (unsigned int a, const vec4& b)
  {
    return { a << b.x, a << b.y, a << b.z, a << b.w };
  }
  vec4& operator <<= (unsigned int b)
  {
    return *this = *this << b;
  }

  friend vec4 operator >> (const vec4& a, unsigned int b)
  {
    return { a.x >> b, a.y >> b, a.z >> b, a.w >> b };
  }
  friend vec4 operator >> (unsigned int a, const vec4& b)
  {
    return { a >> b.x, a >> b.y, a >> b.z, a >> b.w };
  }
  vec4& operator >>= (unsigned int b)
  {
    return *this = *this >> b;
  }

};

template <typename T> T inline dot (const vec4<T>& a, const vec4<T>& b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template <typename T> T inline length (const vec4<T>& a)
{
  return std::sqrt (dot (a, a));
}

template <typename T> vec4<T> inline normalize (const vec4<T>& a)
{
  return a * (T (1) / length (a));
}

namespace std
{
template <typename T> vec4<T> inline min (const vec4<T>& a, const vec4<T>& b)
{
  return vec4<T> (std::min (a.x, b.x), std::min (a.y, b.y),
                  std::min (a.z, b.z), std::min (a.w, b.w));
}

template <typename T> vec4<T> inline max (const vec4<T>& a, const vec4<T>& b)
{
  return vec4<T> (std::max (a.x, b.x), std::max (a.y, b.y),
                  std::max (a.z, b.z), std::max (a.w, b.w));
}

} // namespace std

// --------------------------------------------------------------------------

template <typename T> struct mat4
{
  enum { ELEMENT_COUNT = 4 * 4 };

  union
  {
    struct
    {
      T m00, m10, m20, m30;
      T m01, m11, m21, m31;
      T m02, m12, m22, m32;
      T m03, m13, m23, m33;
    };
    T m[ELEMENT_COUNT];
  };

  friend mat4 operator * (const mat4& a, const mat4& b)
  {
    mat4 result;
    for (int r = 0; r < 4; r++)
    {
      T tmp[4] = { 0, 0, 0, 0 };

      for (int c = 0; c < 4; c++)
        for (int i = 0; i < 4; i++)
          tmp[c] += a.m[4 * i + r] * b.m[4 * c + i];

      for (int c = 0; c < 4; c++)
        result.m[4 * c + r] = tmp[c];
    }
    return result;
  }

  friend vec4<T> operator * (const mat4& m, const vec4<T>& v)
  {
    return vec4<T> (m.m00 * v.x + m.m01 * v.y + m.m02 * v.z + m.m03 * v.w,
                    m.m10 * v.x + m.m11 * v.y + m.m12 * v.z + m.m13 * v.w,
                    m.m20 * v.x + m.m21 * v.y + m.m22 * v.z + m.m23 * v.w,
                    m.m30 * v.x + m.m31 * v.y + m.m32 * v.z + m.m33 * v.w);

  }

  template <typename X, typename Y, typename Z, typename W> static mat4
  scale (X x, Y y, Z z, W w)
  {
    mat4 m;
    m.m00 = x; m.m10 = 0; m.m20 = 0; m.m30 = 0;
    m.m01 = 0; m.m11 = y; m.m21 = 0; m.m31 = 0;
    m.m02 = 0; m.m12 = 0; m.m22 = z; m.m32 = 0;
    m.m03 = 0; m.m13 = 0; m.m23 = 0; m.m33 = w;
    return m;
  }

  template <typename X, typename Y, typename Z> static mat4
  scale (X x, Y y, Z z)
  {
    return scale (x, y, z, 1);
  }

  template <typename XYZ> static mat4 scale (const vec3<XYZ>& xyz)
  {
    return scale (xyz.x, xyz.y, xyz.z, 1);
  }

  static mat4 zero (void) { return scale (0, 0, 0, 0); }
  static mat4 identity (void) { return scale (1, 1, 1, 1); }

  template <typename X, typename Y, typename Z> static mat4
  translate (X x, Y y, Z z)
  {
    mat4 m;
    m.m00 = 1; m.m10 = 0; m.m20 = 0; m.m30 = 0;
    m.m01 = 0; m.m11 = 1; m.m21 = 0; m.m31 = 0;
    m.m02 = 0; m.m12 = 0; m.m22 = 1; m.m32 = 0;
    m.m03 = x; m.m13 = y; m.m23 = z; m.m33 = 1;
    return m;
  }

  template <typename XYZ> static mat4 translate (const vec3<XYZ>& xyz)
  {
    return translate (xyz.x, xyz.y, xyz.z);
  }

  template <typename A> static mat4 rotate_x (A a)
  {
    T s = (T)std::sin (a);
    T c = (T)std::cos (a);

    mat4 m;
    m.m00 = 1; m.m10 = 0; m.m20 = 0;  m.m30 = 0;
    m.m01 = 0; m.m11 = c; m.m21 = -s; m.m31 = 0;
    m.m02 = 0; m.m12 = s; m.m22 = c;  m.m32 = 0;
    m.m03 = 0; m.m13 = 0; m.m23 = 0;  m.m33 = 1;
    return m;
  }

  template <typename A> static mat4 rotate_y (A a)
  {
    T s = (T)std::sin (a);
    T c = (T)std::cos (a);

    mat4 m;
    m.m00 = c;  m.m10 = 0; m.m20 = s;  m.m30 = 0;
    m.m01 = 0;  m.m11 = 1; m.m21 = 0;  m.m31 = 0;
    m.m02 = -s; m.m12 = 0; m.m22 = c;  m.m32 = 0;
    m.m03 = 0;  m.m13 = 0; m.m23 = 0;  m.m33 = 1;
    return m;
  }

  template <typename A> static mat4 rotate_z (A a)
  {
    T s = (T)std::sin (a);
    T c = (T)std::cos (a);

    mat4 m;
    m.m00 = c;  m.m10 = -s; m.m20 = 0;  m.m30 = 0;
    m.m01 = s;  m.m11 = c;  m.m21 = 0;  m.m31 = 0;
    m.m02 = 0;  m.m12 = 0;  m.m22 = 1;  m.m32 = 0;
    m.m03 = 0;  m.m13 = 0;  m.m23 = 0;  m.m33 = 1;
    return m;
  }

  static mat4 proj_orthogonal (T l, T r, T b, T t, T n, T f)
  {
    T rightMinusLeftInv = 1 / (r - l);
    T topMinusBottomInv = 1 / (t - b);
    T farMinusNearInv   = 1 / (f - n);

    mat4 m;

    m.m00 = 2 * rightMinusLeftInv;
    m.m10 = 0;
    m.m20 = 0;
    m.m30 = 0;

    m.m01 = 0;
    m.m11 = 2 * topMinusBottomInv;
    m.m21 = 0;
    m.m31 = 0;

    m.m02 = 0;
    m.m12 = 0;
    m.m22 = -2 * farMinusNearInv;
    m.m32 = 0;

    m.m03 = -(r + l) * rightMinusLeftInv;
    m.m13 = -(t + b) * topMinusBottomInv;
    m.m23 = -(f + n) * farMinusNearInv;
    m.m33 =  1;

    return m;
  }

  static mat4 proj_frustum (T l, T r, T b, T t, T n, T f)
  {
    T rightMinusLeftInv = 1 / (r - l);
    T topMinusBottomInv = 1 / (t - b);
    T farMinusNearInv = 1 / (f - n);
    T twoNear = 2 * n;

    mat4 m;
    m.m00 = twoNear * rightMinusLeftInv;
    m.m10 = 0;
    m.m20 = 0;
    m.m30 = 0;

    m.m01 = 0;
    m.m11 = twoNear * topMinusBottomInv;
    m.m21 = 0;
    m.m31 = 0;

    m.m02 = (r + l) * rightMinusLeftInv;
    m.m12 = (t + b) * topMinusBottomInv;
    m.m22 = -(f + n) * farMinusNearInv;
    m.m32 = -1;

    m.m03 = 0;
    m.m13 = 0;
    m.m23 = -(twoNear * f) * farMinusNearInv;
    m.m33 = 0;

    return m;
  }

  // symmetric
  static mat4 proj_perspective (T fHovFieldOfViewRad, T fAspect,
                                T fNearPlane, T fFarPlane)
  {
    T left = (T)(std::tan(fHovFieldOfViewRad*0.5)*fNearPlane);
    T top  = (T)(std::tan(fHovFieldOfViewRad/fAspect*0.5)*fNearPlane);
    return proj_frustum (-left, left, -top, top, fNearPlane, fFarPlane);
  }

  // asymmetric
  static mat4 proj_perspective (T fHovFieldOfViewRad, T fAspect,
                                T fNearPlane, T fFarPlane,
                                T fHorizontalCenterOffset,
                                T fVerticalCenterOffset )
  {
    T right = (T)(std::tan(fHovFieldOfViewRad*0.5)*fNearPlane);
    T top  = (T)(std::tan(fHovFieldOfViewRad/fAspect*0.5)*fNearPlane);
    T left = -right;
    T bottom = -top;

    T hshift = right*fHorizontalCenterOffset;
    T vshift = top*fVerticalCenterOffset;

    left+=hshift;
    right+=hshift;
    top+=vshift;
    bottom+=vshift;

    return proj_frustum (left, right, bottom, top, fNearPlane, fFarPlane);
  }

  template <typename E>
  static mat4 look_at (const vec3<E>& eye, const vec3<E>& at, const vec3<E>& up)
  {
    mat4<T> m;

    vec3<E> vDir = normalize (at - eye);

    vec3<E> vTemp = normalize (cross (vDir, up));
    vec3<E> vUp   = cross (vTemp, vDir);

    m.m[0] = vTemp.x;
    m.m[1] = vUp.x;
    m.m[2] = -vDir.x;
    m.m[3] = 0;
    m.m[4] = vTemp.y;
    m.m[5] = vUp.y;
    m.m[6] = -vDir.y;
    m.m[7] = 0;
    m.m[8] = vTemp.z;
    m.m[9] = vUp.z;
    m.m[10] = -vDir.z;
    m.m[11] = 0;
    m.m[12] = 0;
    m.m[13] = 0;
    m.m[14] = 0;
    m.m[15] = 1;

    return m * translate (-eye);
  }
};



static_assert (std::is_trivial<vec2<float>>::value, "");
static_assert (std::is_standard_layout<vec2<float>>::value, "");
static_assert (std::is_pod<vec2<float>>::value, "");

static_assert (std::is_trivial<vec3<int>>::value, "");
static_assert (std::is_standard_layout<vec3<int>>::value, "");
static_assert (std::is_pod<vec3<int>>::value, "");

static_assert (std::is_trivial<vec4<int>>::value, "");
static_assert (std::is_standard_layout<vec4<int>>::value, "");
static_assert (std::is_pod<vec4<int>>::value, "");

#endif // includeguard_vec_mat_hpp_includeguard
