#ifndef INCLUDE_TRANSFORMATIONS_HPP
#define INCLUDE_TRANSFORMATIONS_HPP

#include <iostream>

#include "al/core.hpp"

using namespace al;
using namespace std;

Vec3f eucl(const Vec4f& src) {
  return Vec3f(src[1], src[2], src[3]);
}

Vec3f s3(const Vec4f& src) {
  return Vec3f(src[1] / (1.f - src[0]),
               src[2] / (1.f - src[0]),
               src[3] / (1.f - src[0]));
}

Vec3f klein(const Vec4f& src) {
  return Vec3f(
    src[1] / src[0],
    src[2] / src[0],
    src[3] / src[0]
    );
}

Vec3f uhs(const Vec3f& src) {
  // return src;
  float a = 2.0 / (1.0 - 2.0*src[2] + src[0]*src[0] + src[1]*src[1] + src[2]*src[2]);
  return Vec3f(
    a * src[0],
    a * src[1],
    a * (1.0 - src[2]) - 1.0
    );
}

Mat4f rotate3s(Mat4f& srcMat, float& theta, float& phi) {
  Mat4f rotate = Mat4f(cos(theta), 0, 0, -sin(theta),
                     0, cos(phi), -sin(phi), 0,
                     0, sin(phi), cos(phi), 0,
                     sin(theta), 0, 0, cos(theta));
  return rotate * srcMat;
}

Mat4f rotateTheta(Mat4f& srcMat, float& angle) {
  Mat4f rotate = Mat4f(
    cosh(angle), sinh(angle), 0.0, 0.0,
    sinh(angle), cosh(angle), 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0);

  return rotate * srcMat;
}

Mat4f rotatePhi(Mat4f& srcMat, float& angle) {
  Mat4f rotate = Mat4f(
    cosh(angle), 0.0, sinh(angle), 0.0,
    0.0, 1.0, 0.0, 0.0,
    sinh(angle), 0.0, cosh(angle), 0.0,
    0.0, 0.0, 0.0, 1.0);

  return rotate * srcMat;
}

Mat4f rotateEpsilon(Mat4f& srcMat, float& angle) {
  Mat4f rotate = Mat4f(
    cosh(angle), 0.0, 0.0, sinh(angle),
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    sinh(angle), 0.0, 0.0, cosh(angle));

  return rotate * srcMat;
}

Mat4f para(Mat4f& srcMat, float& p, float& q) {
  Mat4f par = Mat4f(
    0.5*(2.0 + p*p + q*q), p, -q, -0.5*(p*p + q*q),
    p, 1.0, 0.0, -p,
    -q, 0.0, 1.0, q,
    0.5*(p*p + q*q), p, -q, 0.5*(2.0 - p*p - q*q));

  return par * srcMat;
}

Vec4f loxodromic(const Vec4f& src, const float& s, const float& t, const float& x) {
  Mat4f s_mat(
    cosh(2.0*t), 0, 0, sinh(2.0*t),
    0, cos(2.0*s), sin(2.0*s), 0,
    0, -sin(2.0*s), cos(2.0*s), 0,
    sinh(2.0*t), 0, 0, cosh(2.0*t)
    );

  Mat4f x_mat(
    (1.0+pow(x,2)+pow(x,4))/(2.0*x*x), pow(x,-1), 0.0, (-1.0+pow(x,2)+pow(x,4))/(2.0*x*x),
    x, 1.0, 0.0, x,
    0.0, 0.0, 1.0, 0.0,
    -(1.0+pow(x,2)-pow(x,4))/(2.0*x*x), -pow(x,-1), 0.0, (1.0-pow(x,2)+pow(x,4))/(2.0*x*x)
    );

  Mat4f x_inv(x_mat);
  invert(x_inv);

  Mat4f result;

  Mat4f::multiply(result, s_mat, x_inv);
  Mat4f::multiply(result, x_mat, result);

  return result*src;
}

bool compare(Mat4f& a, Mat4f& b) {
  for(int i = 0; i < a.size(); ++i) {
    if (abs(a[i] - b[i]) > 1e-7) return false;
  }

  return true;
}

#endif