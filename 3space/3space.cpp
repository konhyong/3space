#include <iostream>

#include "al/core.hpp"
#include "al/core/app/al_DistributedApp.hpp"
#include "generator.hpp"

using namespace al;
using namespace std;

const std::string shader_vert =
#include "shader.vert"
;
const std::string shader_frag =
#include "shader.frag"
;

struct State {
  Pose pose;
  Mat4f camera;
  int activeGroup;
  float scale;
  int depth;
  bool showStatic;
  bool showOrigin;
};

class SpaceApp : public DistributedApp<State> {
public:
  VAOMesh tetra;
  BufferObject buffer;
  Group groups;

  ShaderProgram main_shader;

  float theta, phi;
  int changeTheta, changePhi, changeScale;
  int currentGroup;

  int joystickModifier;

  void onCreate() {
    nav().pos(Vec3f{0, 0, 20}).faceToward({0, 0, 0}, {0, 1, 0});
    lens().near(0.01).far(1000).eyeSep(0.02);

    theta = phi = 0;
    changeTheta = changePhi = changeScale = 0;
    joystickModifier = 0;

    state().pose = nav();
    state().camera.setIdentity();
    state().activeGroup = 0;
    currentGroup = -1;
    state().scale = 1.f;
    state().showStatic = true;
    state().showOrigin = true;
    state().depth = 2;

    int Nv;
    Nv = addTetrahedron(tetra, 0.5);
    for(int i=0; i<Nv; ++i){
      float f = float(i)/Nv;
      tetra.color(HSV(f,1,1));
    }

    // tetra.decompress();
    // tetra.generateNormals();

    tetra.update();

    buffer.bufferType(GL_ARRAY_BUFFER);
    buffer.usage(GL_DYNAMIC_DRAW);

    buffer.create();

    main_shader.compile(shader_vert, shader_frag);

    groups.init();

    buffer.bind();
    for (int i = 0; i < 4; ++i) {
      glVertexAttribPointer(3+i, 4, GL_FLOAT, GL_FALSE, sizeof(Mat4f), (void *)(sizeof(Vec4f)*i));
      glEnableVertexAttribArray(3+i);
      glVertexAttribDivisor(3+i, 1);
    }

    buffer.data(groups[state().activeGroup].transforms.size() * sizeof(Mat4f), groups[state().activeGroup].transforms.data());
    buffer.unbind();
  }

  void updateCamera() {
    Mat4f& cam = state().camera;

    cam.setIdentity();
    if(groups[state().activeGroup].type == GroupType::SPHERICAL) {
      cam = rotate3s(cam, theta, phi);
    }
    else if (groups[state().activeGroup].type == GroupType::HYPERBOLIC) {
      cam = para(cam, theta, phi);
    }
  }

  void simulate(double dt) {
    if(changeTheta > 0) theta += 0.01;
    else if(changeTheta < 0) theta -= 0.01;

    if(changePhi > 0) phi += 0.01;
    else if(changePhi < 0) phi -= 0.01;

    updateCamera();

    if(changeScale > 0) state().scale += 0.01;
    else if(changeScale < 0) state().scale -= 0.01;

    state().pose = nav();
  }

  void onAnimate(double dt){
    if(role() & ROLE_RENDERER) {
      nav().set(state().pose);
    }

    if(currentGroup != state().activeGroup) {
      buffer.bind();
      buffer.data(groups[state().activeGroup].transforms.size() * sizeof(Mat4f), groups[state().activeGroup].transforms.data());
      buffer.unbind();

      currentGroup = state().activeGroup;
    }
  }

  void onDraw(Graphics& g){
    g.clear(0, 0, 0);

    g.depthTesting(true);
    // g.lighting(true);

    g.pushMatrix();
      g.shader(main_shader);
      
      if(render_stereo) {
        float e = pp_render.current_eye? 1 : -1;
        g.shader().uniform("is_omni", 1.f);
        g.shader().uniform("eye_sep", lens().eyeSep() * e / 2.0f);
        g.shader().uniform("foc_len", lens().focalLength());
      }

      g.shader().uniform("camera", state().camera);
      g.shader().uniform("scale", state().scale);
      g.shader().uniform("groupType", groups[state().activeGroup].type);
      g.shader().uniform("showOrigin", state().showOrigin);

      g.update();

      g.polygonMode(Graphics::FILL);

      tetra.vao().bind();
      std::vector<Mat4f>& transforms = groups[state().activeGroup].transforms;

      if(tetra.indices().size()) {
        tetra.indexBuffer().bind();
        glDrawElementsInstanced(GL_TRIANGLES, tetra.indices().size(),
                                GL_UNSIGNED_INT, 0, transforms.size());
      }
      else {
        glDrawArraysInstanced(GL_TRIANGLES, 0, tetra.vertices().size(), transforms.size());
      }

      if(state().showStatic) {
        g.polygonMode(Graphics::LINE);
        g.shader().uniform("camera", Mat4f::identity());
        if(tetra.indices().size()) {
          tetra.indexBuffer().bind();
          glDrawElementsInstanced(GL_TRIANGLES, tetra.indices().size(),
                                  GL_UNSIGNED_INT, 0, transforms.size());
        }
        else {
          glDrawArraysInstanced(GL_TRIANGLES, 0, tetra.vertices().size(), transforms.size());
        }
      }

      g.polygonMode(Graphics::FILL);
    g.popMatrix();
  }

  // KEYBOARD commands local
  void onKeyDown(const Keyboard& k){
    switch (k.key()) {
      case 'r': nav().home(); theta = 0; phi = 0; break;
      case 'f': theta = 0; phi = 0; break;
      case 'g': changeTheta = -1; break;
      case 't': changeTheta = 1; break;
      case 'j': changePhi = -1; break;
      case 'u': changePhi = 1; break;
      case '[': state().activeGroup -= 1; if(state().activeGroup < 0) state().activeGroup = groups.size() - 1; break;
      case ']': state().activeGroup += 1; if(state().activeGroup >= groups.size()) state().activeGroup = 0; break;
      case '-': changeScale = -1; break;
      case '=': changeScale = 1; break;
      case '0': state().scale = 1.f; break;
      case '\\': state().showStatic = !state().showStatic; break;
      case 'p': state().showOrigin = !state().showOrigin; break;
      default: break;
    }
  } // onKeyDown

  void onKeyUp(const Keyboard& k){
    switch (k.key()) {
      case 'g': changeTheta = 0; break;
      case 't': changeTheta = 0; break;
      case 'j': changePhi = 0; break;
      case 'u': changePhi = 0; break;
      case '-': changeScale = 0; break;
      case '=': changeScale = 0; break;
      default: break;
    }

    updateCamera();
  } // onKeyUp

  void onMessage(osc::Message& m) {
    float x;
    float mNavSpeed = 0.125f;
    float mNavTurnSpeed = 2.f;
    // m.print();
    if (m.addressPattern() == "/mx") {
      m >> x;
      nav().moveR(x * mNavSpeed);
    } else if (m.addressPattern() == "/my") {
      m >> x;
      nav().moveU(x * mNavSpeed);
    } else if (m.addressPattern() == "/mz") {
      m >> x;
      nav().moveF(x * mNavSpeed);
    } else if (m.addressPattern() == "/tx") {
      m >> x;
      nav().spinR(x * mNavTurnSpeed);
    } else if (m.addressPattern() == "/ty") {
      m >> x;
      nav().spinU(x * mNavTurnSpeed);
    } else if (m.addressPattern() == "/tz") {
      m >> x;
      nav().spinF(x * mNavTurnSpeed);
    } else if (m.addressPattern() == "/b1") {
      m >> x;
      if (x == 1) {
        changeTheta = 1;
      } else {
        changeTheta = 0;
      }
    } else if (m.addressPattern() == "/b2") {
      m >> x;
      if (x == 1) {
        changeTheta = -1;
      } else {
        changeTheta = 0;
      }
    } else if (m.addressPattern() == "/b3") {
      m >> x;
      if (x == 1) {
        changePhi = -1;
      } else {
        changePhi = 0;
      }
    } else if (m.addressPattern() == "/b4") {
      m >> x;
      if (x == 1) {
        changePhi = 1;
      } else {
        changePhi = 0;
      }
    } else if (m.addressPattern() == "/b5") {
      m >> x;
      if (x == 1) {
        joystickModifier |= 1;
      } else {
        joystickModifier ^= 1;
      }
    } else if (m.addressPattern() == "/b6") {
      m >> x;
      if (x == 1) {
        if (joystickModifier == 0) {
          // ++state->depth;
        } else if (joystickModifier == 1) {
          cout << "changing activeGroup" << endl;
          state().activeGroup += 1; if (state().activeGroup >= groups.size()) state().activeGroup = 0;
        } else if (joystickModifier == 2) {
          changeScale = 1;
        }
      }
      else {
        changeScale = 0;
      }
    } else if (m.addressPattern() == "/b7") {
      m >> x;
      if (x == 1) {
        joystickModifier |= 2;
      } else {
        joystickModifier ^= 2;
      }
    } else if (m.addressPattern() == "/b8") {
      m >> x;
      if (x == 1) {
        if (joystickModifier == 0) {
          // --state->depth;
        } else if (joystickModifier == 1) {
          cout << "changing activeGroup" << endl;
          state().activeGroup -= 1; if (state().activeGroup < 0) state().activeGroup = groups.size() - 1;
        } else if (joystickModifier == 2) {
          changeScale = -1;
        }
      }
      else {
        changeScale = 0;
      }
    } else if (m.addressPattern() == "/b9") {
      m >> x;
      if (x == 1) {
        nav().home();
        theta = 0;
        phi = 0;
      }
    } else if (m.addressPattern() == "/b10") {
      m >> x;
      if (x == 1) {
        if (joystickModifier == 0)
          state().showOrigin = !state().showOrigin;
        else if (joystickModifier == 1) {
          state().showStatic = !state().showStatic;
        }
      }
    }// else {
      m.print();
      cout << joystickModifier << endl;
    //}
  }
};

int main() {
  SpaceApp app;
  app.stereo(true);
  if (!sphere::is_simulator())
    app.displayMode(app.displayMode() | Window::STEREO_BUF);
  app.print();
  app.start();
}