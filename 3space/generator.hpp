#ifndef INCLUDE_GENERATOR_HPP
#define INCLUDE_GENERATOR_HPP

#include <iostream>

#include "al/core.hpp"
#include "transformations.hpp"

using namespace al;
using namespace std;

enum GroupType {
  EUCLEADIAN = 0,
  SPHERICAL = -1,
  HYPERBOLIC = 1
};

bool isSameMat(Mat4f& a, Mat4f& b) {
  for(int i = 0; i < a.size(); ++i) {
    if (abs(a[i] - b[i]) > 1e-4) return false;
  }

  return true;
}

struct Generator {
  std::vector<Mat4f> gen;
  std::vector<Mat4f> transforms;
  std::vector<int> depths;
  GroupType type;
  int maxDepth;

  Generator(Mat4f& a, int depth = 4, GroupType t=GroupType::EUCLEADIAN) {
    type = t;
    maxDepth = depth;

    Mat4f a_inv = a;
    invert(a_inv);

    gen.resize(2);
    gen[0] = a;
    gen[1] = a_inv;

    genTransforms(maxDepth);
  }

  Generator(Mat4f& a, Mat4f& b, int depth = 4, GroupType t=GroupType::EUCLEADIAN) {
    type = t;
    maxDepth = depth;

    Mat4f a_inv = a;
    Mat4f b_inv = b;
    invert(a_inv);
    invert(b_inv);

    gen.resize(4);
    gen[0] = a;
    gen[1] = b;
    gen[2] = a_inv;
    gen[3] = b_inv;

    genTransforms(maxDepth);
  }

  Generator(Mat4f& a, Mat4f& b, Mat4f& c, int depth = 4, GroupType t=GroupType::EUCLEADIAN) {
    type = t;
    maxDepth = depth;

    Mat4f a_inv = a;
    Mat4f b_inv = b;
    Mat4f c_inv = c;
    invert(a_inv);
    invert(b_inv);
    invert(c_inv);

    gen.resize(6);
    gen[0] = a;
    gen[1] = b;
    gen[2] = c;
    gen[3] = a_inv;
    gen[4] = b_inv;
    gen[5] = c_inv;

    genTransforms(maxDepth);
  }

  void genTransforms(int& maxDepth) {
    int count = 0;
    transforms.push_back(Mat4f::identity());
    count += 1;
    depths.push_back(count);

    int idx = 0;
    for(int d = 0; d < maxDepth; ++d) {
      int oldIdx = idx;
      idx = transforms.size();

      bool depthChange = false;

      for(int i = oldIdx; i < idx; ++i) {
        for(int j = 0; j < gen.size(); ++j) {
          Mat4f& old = transforms[i];
          Mat4f newTrans = gen[j] * old;
          
          bool unique = true;

          for(int k = 0; k < transforms.size(); ++k) {
            if(isSameMat(newTrans, transforms[k])) {
              unique = false;
              break;
            }
          }
          if(unique) {
            transforms.push_back(newTrans);
            count += 1;
            depthChange = true;
          }
        }
      }

      if(depthChange) {
        depths.push_back(count);
      }
    }
    cout << transforms.size() << endl;
  }

  unsigned size() { return transforms.size(); }

  Mat4f& get(int n) { return transforms[n]; }

  int getDepth(int n) { return depths[n]; }
};

struct Group {
  std::vector<Generator> generators;

  void init() {
    Mat4f a, b, c;

    // s1 * r2
    a = Mat4f(
      1, 0, 0, 0,
      1, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1);
    generators.emplace_back(a, 6, GroupType::EUCLEADIAN);

    // t2 * r
    a = Mat4f(
      1, 0, 0, 0,
      1, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1);
    b = Mat4f(
      1, 0, 0, 0,
      0, 1, 0, 0,
      1, 0, 1, 0,
      0, 0, 0, 1);
    generators.emplace_back(a, b, 6, GroupType::EUCLEADIAN);

    // 3-Torus;
    a = Mat4f(
      1, 0, 0, 0,
      1, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1);
    b = Mat4f(
      1, 0, 0, 0,
      0, 1, 0, 0,
      1, 0, 1, 0,
      0, 0, 0, 1);
    c = Mat4f(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      1, 0, 0, 1);
    generators.emplace_back(a, b, c, 6, GroupType::EUCLEADIAN);

    // k2 * r
    a = Mat4f(
      1, 0, 0, 0,
      1, -1, 0, 0,
      0, 0, 1, 0,
      1, 0, 0, 1);
    b = Mat4f(
      1, 0, 0, 0,
      1, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1);
    generators.emplace_back(a, b, 6, GroupType::EUCLEADIAN);

    //k2 * s1
    a = Mat4f(
      1, 0, 0, 0,
      0, 1, 0, 0,
      1, 0, 1, 0,
      0, 0, 0, -1);
    b = Mat4f(
      1, 0, 0, 0,
      1, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1);
    c = Mat4f(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      1, 0, 0, 1);
    generators.emplace_back(a, b, c, 6, GroupType::EUCLEADIAN);

    // Half-Twist Cube
    a = Mat4f(
      1, 0, 0, 0,
      0, 0, 0, -1,
      1, 0, 1, 0,
      0, 1, 0, 0);
    b = Mat4f(
      1, 0, 0, 0,
      1, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1);
    c = Mat4f(
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 1, 0,
      1, 0, 0, 1);
    generators.emplace_back(a, b, c, 6, GroupType::EUCLEADIAN);

    //Half-Twist Chimney
    a = Mat4f(
      1, 0, 0, 0,
      0, 0, 0, -1,
      1, 0, 1, 0,
      0, 1, 0, 0);
    b = Mat4f(
      1, 0, 0, 0,
      1, 1, 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1);
    generators.emplace_back(a, b, 6, GroupType::EUCLEADIAN);

    // Binary Tetrahedral
    a = 0.5f * Mat4f(
      1, 1, -1, 1,
      -1, 1, -1, -1,
      1, 1, 1, -1,
      -1, 1, 1, 1);
    b = 0.5f * Mat4f(
      1, -1, -1, 1,
      1, 1, -1, -1,
      1, 1, 1, 1,
      -1, 1, -1, 1);
    generators.emplace_back(a, b, 6, GroupType::SPHERICAL);

    // Binary Octahedral
    a = 0.5f * Mat4f(
      1, 1, -1, 1,
      -1, 1, -1, -1,
      1, 1, 1, -1,
      -1, 1, 1, 1);
    b = (1.f / sqrt(2.f)) * Mat4f(
      1, 0, 0, 1,
      0, 1, -1, 0,
      0, 1, 1, 0,
      -1, 0, 0, 1);
    generators.emplace_back(a, b, 6, GroupType::SPHERICAL);

    // Binary Icosahedral
    double gold = (1.0 + sqrt(5)) / 2.0;
    double gold_inv = 1.0 / gold;
    a = 0.5f * Mat4f(
      1, 1, -1, 1,
      -1, 1, -1, -1,
      1, 1, 1, -1,
      -1, 1, 1, 1);
    b = 0.5f * Mat4f(
      gold, 0, -1, gold_inv,
      0, gold, -gold_inv, -1,
      1, gold_inv, gold, 0,
      -gold_inv, 1, 0, gold);
    generators.emplace_back(a, b, 6, GroupType::SPHERICAL);

    // Aplonian Gasket
    a = Mat4f(
      3.0, 0.0, -2.0, 2.0,
      0.0, 1.0, 0.0, 0.0,
      -2.0, 0.0, 1.0, -2.0,
      -2.0, 0.0, 2.0, -1.0);

    b = Mat4f(
      3.0, 2.0, -2.0, 0.0,
      2.0, 1.0, -2.0, 0.0,
      2.0, 2.0, -1.0, 0.0,
      0.0, 0.0, 0.0, 1.0);
    generators.emplace_back(a, b, 8, GroupType::HYPERBOLIC);

    // Figure 8 knot complement
    a = Mat4f(
      1.5, 1.0, 0.0, -0.5,
      1.0, 1.0, 0.0, -1.0,
      0.0, 0.0, 1.0, 0.0,
      0.5, 1.0, 0.0, 0.5);
    b = Mat4f(
      1.5, 0.5, -sqrt(3)/2.0, 0.5,
      0.5, 1.0, 0.0, 0.5,
      -sqrt(3)/2.0, 0.0, 1.0, -sqrt(3)/2.0,
      -0.5, -0.5, sqrt(3)/2.0, 0.5);

    generators.emplace_back(a, b, 8, GroupType::HYPERBOLIC);

    cout << "** Generated " << generators.size() << " groups." << endl;
  }

  unsigned size() { return generators.size(); }

  Generator& operator[](int i) { return generators[i]; }
  const Generator& operator[](int i) const { return generators[i]; }
};

#endif