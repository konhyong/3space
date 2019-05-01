R"(
#version 330
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;
uniform float is_omni;
uniform float eye_sep;
uniform float foc_len;
uniform mat4 camera;
uniform float scale;
uniform int groupType;
uniform bool showOrigin;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 aColor;
layout (location = 3) in mat4 xfm;
out vec4 fColor;

const float PI_2 = 1.57079632679489661923;

vec4 stereo_displace(vec4 v, float e, float r) {
  vec3 OE = vec3(-v.z, 0.0, v.x); // eye direction, orthogonal to vertex vector
  OE = normalize(OE);             // but preserving +y up-vector
  OE *= e;               // set mag to eye separation
  vec3 EV = v.xyz - OE;  // eye to vertex
  float ev = length(EV); // save length
  EV /= ev;              // normalize
  // coefs for polynomial t^2 + 2bt + c = 0
  // derived from cosine law r^2 = t^2 + e^2 + 2tecos(theta)
  // where theta is angle between OE and EV
  // t is distance to sphere surface from eye
  float b = -dot(OE, EV);         // multiply -1 to dot product because
                                  // OE needs to be flipped in direction
  float c = e * e - r * r;
  float t = -b + sqrt(b * b - c); // quadratic formula
  v.xyz = OE + t * EV;            // direction from origin to sphere surface
  v.xyz = ev * normalize(v.xyz);  // normalize and set mag to eye-to-v distance
  return v;
}

vec3 sphere3(vec4 src) {
  return vec3(src.y / (1.0 - src.x),
              src.z / (1.0 - src.x),
              src.w / (1.0 - src.x));
}

vec3 klein(vec4 src) {
  return src.yzw / src.x;
}

vec3 uhs(vec3 src) {
  float a = 2.0 / (1.0 - 2.0*src.z + src.x*src.x + src.y*src.y + src.z*src.z);
  return vec3(a*src.x, a*src.y, a*(1.0 - src.z) - 1.0);
}

void main() {
  fColor = aColor;

  vec3 scaled_pos = scale * pos;
  if(!showOrigin && gl_InstanceID == 0) scaled_pos = vec3(0, 0, 0);

  vec4 p = vec4(sqrt(1.0 + groupType * (scaled_pos.x*scaled_pos.x + scaled_pos.y*scaled_pos.y + scaled_pos.z*scaled_pos.z)), scaled_pos);
  p = camera * xfm * p;

  switch(groupType) {
    case -1: p = vec4(sphere3(p), 1); break;
    case 1: p = vec4(klein(p), 1); break; // uhs(klien(p)) for uhs
    default: p = vec4(p.yzw, 1); break;
  }

  if (is_omni > 0.5) {
    gl_Position = al_ProjectionMatrix * stereo_displace(al_ModelViewMatrix * p, eye_sep, foc_len);
  }
  else {
    gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * p;
  }
}
)"