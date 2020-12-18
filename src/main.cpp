#include <iostream>

// for devel branch
#include "al/app/al_App.hpp"
#include "freenect2Wrapper.hpp"

using namespace al;
using namespace std;

struct MyApp : App {
  Freenect2Wrapper fn;

  Texture tex_rgb, tex_ir, tex_depth, tex_registered;
  bool pip = false;
  bool takeSnapshot = false;
  int pattern_idx = 0;

  Mesh mesh;
  Texture tex_pattern;
  vector<Colori> pattern;

  Texture tex_black, tex_white, tex_response;
  vector<unsigned char> black, white, response;

  int width;
  int height;

  int line_idx = 0;

  void onCreate() override {
    width = 800;
    height = 600;

    tex_pattern.create2D(width, height, Texture::RGBA8);
    pattern.resize(width * height);

    for (int j = 0; j < height; ++j) {
      for (int i = 0; i < width; ++i) {
        if (i < 2 || j < 2 || i > 797 || j > 597) {
          pattern[j * width + i] = Colori(155, 255, 155, 255);
        } else if (i > 399 && i < 402) {
          pattern[j * width + i] = Colori(0, 255, 255, 255);
        } else if (j > 299 && j < 302) {
          pattern[j * width + i] = Colori(0, 155, 255, 255);
        } else {
          pattern[j * width + i] = Colori(255, 155, 155, 255);
        }
      }
    }

    // setLine(240);

    tex_pattern.submit(pattern.data());

    black.resize(1920 * 1080 * 4);
    white.resize(1920 * 1080 * 4);
    response.resize(1920 * 1080 * 4);

    mesh.primitive(Mesh::TRIANGLE_STRIP);
    mesh.texCoord(1, 1);
    mesh.vertex(-1, -1, 0);
    mesh.texCoord(0, 1);
    mesh.vertex(1, -1, 0);
    mesh.texCoord(1, 0);
    mesh.vertex(-1, 1, 0);
    mesh.texCoord(0, 0);
    mesh.vertex(1, 1, 0);

    fn.init();

    tex_rgb.create2D(1920, 1080, Texture::RGBA, Texture::BGRA, Texture::UBYTE);

    tex_rgb.filterMag(Texture::NEAREST);
    tex_rgb.filterMin(Texture::NEAREST);
    tex_rgb.wrap(Texture::CLAMP_TO_BORDER, Texture::CLAMP_TO_BORDER);

    tex_black.create2D(1920, 1080, Texture::RGBA, Texture::BGRA,
                       Texture::UBYTE);

    tex_black.filterMag(Texture::NEAREST);
    tex_black.filterMin(Texture::NEAREST);
    tex_black.wrap(Texture::CLAMP_TO_BORDER, Texture::CLAMP_TO_BORDER);

    tex_white.create2D(1920, 1080, Texture::RGBA, Texture::BGRA,
                       Texture::UBYTE);

    tex_white.filterMag(Texture::NEAREST);
    tex_white.filterMin(Texture::NEAREST);
    tex_white.wrap(Texture::CLAMP_TO_BORDER, Texture::CLAMP_TO_BORDER);

    tex_response.create2D(1920, 1080, Texture::RGBA, Texture::BGRA,
                          Texture::UBYTE);

    tex_response.filterMag(Texture::NEAREST);
    tex_response.filterMin(Texture::NEAREST);
    tex_response.wrap(Texture::CLAMP_TO_BORDER, Texture::CLAMP_TO_BORDER);

    tex_registered.create2D(512, 424, Texture::RGBA, Texture::BGRA,
                            Texture::UBYTE);

    tex_registered.filterMag(Texture::NEAREST);
    tex_registered.filterMin(Texture::NEAREST);
    tex_registered.wrap(Texture::CLAMP_TO_BORDER, Texture::CLAMP_TO_BORDER);

    tex_ir.create2D(512, 424, Texture::R32F, Texture::RED, Texture::FLOAT);

    tex_ir.filterMag(Texture::NEAREST);
    tex_ir.filterMin(Texture::NEAREST);
    tex_ir.wrap(Texture::CLAMP_TO_BORDER, Texture::CLAMP_TO_BORDER);

    tex_depth.create2D(512, 424, Texture::R32F, Texture::RED, Texture::FLOAT);

    tex_depth.filterMag(Texture::NEAREST);
    tex_depth.filterMin(Texture::NEAREST);
    tex_depth.wrap(Texture::CLAMP_TO_BORDER, Texture::CLAMP_TO_BORDER);
  }

  void onExit() override { fn.release(); }

  void onAnimate(double dt) override {
    if (pattern_idx == 0) {
      setLine(line_idx++);
      if (line_idx >= width)
        line_idx = 0;
    } else if (pattern_idx == 1) {
      setPattern(0, 0, 0);
    } else if (pattern_idx == 2) {
      setPattern(255, 255, 255);
    }

    tex_pattern.submit(pattern.data()); // subimage3d might be the problem

    fn.update();

    libfreenect2::Frame *rgb = fn.frames[libfreenect2::Frame::Color];
    tex_rgb.submit(rgb->data);

    if (takeSnapshot) {
      if (pattern_idx == 1) {
        black = vector<unsigned char>(rgb->data,
                                      rgb->data + rgb->width * rgb->height *
                                                      rgb->bytes_per_pixel);

        tex_black.submit(black.data());
      } else if (pattern_idx == 2) {
        white = vector<unsigned char>(rgb->data,
                                      rgb->data + rgb->width * rgb->height *
                                                      rgb->bytes_per_pixel);
        tex_white.submit(white.data());
      } else if (pattern_idx == 3) {
        for (int i = 0; i < response.size(); ++i) {
          response[i] = white[i] - black[i];
        }
        tex_response.submit(response.data());
      }
      takeSnapshot = false;
    }

    libfreenect2::Frame *depth = fn.frames[libfreenect2::Frame::Depth];
    tex_depth.submit(depth->data);

    fn.listener.release(fn.frames);
  }

  void onDraw(Graphics &g) override {
    g.clear(0, 0, 0);

    g.camera(Viewpoint::IDENTITY);

    if (pattern_idx == 3) {
      tex_response.bind(0);
      g.texture();
      g.draw(mesh);
      tex_response.unbind(0);
    } else {
      tex_pattern.bind(0);
      g.texture();
      g.draw(mesh);
      tex_pattern.unbind(0);
    }

    if (pip) {
      g.viewport(0, 0, fbWidth() * 0.4, fbHeight() * 0.4);
      tex_rgb.bind(0);
      g.texture();
      g.draw(mesh);
      tex_rgb.unbind(0);

      g.viewport(fbWidth() * 0.4, 0, fbWidth() * 0.4, fbHeight() * 0.4);
      tex_depth.bind(0);
      g.texture();
      g.draw(mesh);
      tex_depth.unbind(0);

      g.viewport(0, fbHeight() * 0.4, fbWidth() * 0.4, fbHeight() * 0.4);
      tex_black.bind(0);
      g.texture();
      g.draw(mesh);
      tex_black.unbind(0);

      g.viewport(fbWidth() * 0.4, fbHeight() * 0.4, fbWidth() * 0.4,
                 fbHeight() * 0.4);
      tex_white.bind(0);
      g.texture();
      g.draw(mesh);
      tex_white.unbind(0);
    }
  }

  void setPattern(int r, int g, int b, int a = 255) {
    for (int j = 0; j < height; ++j) {
      for (int i = 0; i < width; ++i) {
        pattern[j * width + i] = Colori(r, g, b, a);
      }
    }
  }

  void setLine(int idx, bool vertical = true) {
    for (int j = 0; j < height; ++j) {
      for (int i = 0; i < width; ++i) {
        if (vertical && (idx == i))
          pattern[j * width + i] = Colori(255, 255, 255, 255);
        else if (!vertical && (idx == j))
          pattern[j * width + i] = Colori(255, 255, 255, 255);
        else
          pattern[j * width + i] = Colori(0, 0, 0, 255);
      }
    }
  }

  bool onKeyDown(const Keyboard &k) override {
    switch (k.key()) {
    case '`':
      pip = !pip;
      break;
    case '0':
      pattern_idx = 0;
      break;
    case '1':
      pattern_idx = 1;
      break;
    case '2':
      pattern_idx = 2;
      break;
    case '3':
      pattern_idx = 3;
      break;
    case ' ':
      takeSnapshot = true;
      break;
    case 'q':
      quit();
      break;
    }
    return true;
  }
};

int main() {
  MyApp app;
  app.dimensions(800, 600);
  app.start();
}
