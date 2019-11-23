/*
 * Copyright 2019 Eberty Alves
 */

#include <bits/stdc++.h>

#include <osg/Image>
#include <osg/LineWidth>
#include <osg/MatrixTransform>
#include <osg/Matrixd>
#include <osg/PositionAttitudeTransform>
#include <osg/TexGenNode>
#include <osg/TexMat>
#include <osg/Texture2D>

#include <osgDB/ReadFile>

#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>

#include <osgUtil/Optimizer>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgViewer/GraphicsWindow>

enum CameraIndex { TranslationVector = 0, LensDistortion, ViewportPx, PixelSizeMm, CenterPx, FocalMm, RotationMatrix };
enum PlaneIndex { semantic = 0, fileName };

const int texture_unit = 1;

std::vector<std::vector<float>> camera_description(7);
std::vector<std::string> plane_description(2);

bool readMlpFile(const std::string& file_path) {
  std::size_t found;
  std::string line;
  std::ifstream file(file_path);

  if (file.is_open()) {
    while (getline(file, line)) {
      if (line.find("MLRaster") != std::string::npos) break;
    }

    while (getline(file, line)) {
      if (line.find("</MLRaster>") != std::string::npos) {
        break;
      } else if (line.find("<VCGCamera") != std::string::npos) {
        std::vector<std::string> VCGCamera = {"TranslationVector=", "LensDistortion=", "ViewportPx=",    "PixelSizeMm=",
                                              "CenterPx=",          "FocalMm=",        "RotationMatrix="};
        for (std::size_t i = 0; i < VCGCamera.size(); i++) {
          found = line.find(VCGCamera[i]);
          if (found != std::string::npos) {
            std::size_t j = found + VCGCamera[i].size() + 1, k = 0;
            for (; j < line.size() && line[j] != '\"'; j++, k++) {
            }
            std::istringstream is(line.substr(found + VCGCamera[i].size() + 1, k));
            camera_description[i] = {std::istream_iterator<float>(is), std::istream_iterator<float>()};
          }
        }
      } else if (line.find("<Plane") != std::string::npos) {
        std::vector<std::string> Plane = {"semantic=", "fileName="};
        for (std::size_t i = 0; i < Plane.size(); i++) {
          found = line.find(Plane[i]);
          if (found != std::string::npos) {
            std::size_t j = found + Plane[i].size() + 1, k = 0;
            for (; j < line.size() && line[j] != '\"'; j++, k++) {
            }
            plane_description[i] = line.substr(found + Plane[i].size() + 1, k);
          }
        }
      }
    }

    file.close();
    return true;
  } else {
    std::cout << "Unable to open file: " << file_path << std::endl;
    return false;
  }
}

std::string baseDirectory(const std::string& file_path) {
  std::string file_directory = file_path;

  std::string::size_type ss;
  if (std::string::npos != (ss = file_directory.find_last_of("\\/"))) {
    file_directory.erase(ss, file_directory.length() - ss);
  } else {
    file_directory = "";
  }

  char s;
  if (file_directory.empty()) {
    file_directory = ".";
    file_directory += '/';
  } else if ((s = *(file_directory.end() - 1)) != '\\' && s != '/') {
    file_directory += '/';
  }

  return file_directory;
}

osg::StateSet* createProjectorState(const std::string& file) {
  if (readMlpFile(file) && camera_description[CameraIndex::RotationMatrix].size() >= 16 &&
      plane_description[PlaneIndex::fileName].size()) {
    double mat_rotation[4][4];
    for (size_t i = 0; i < 4; i++) {
      for (size_t j = 0; j < 4; j++) {
        mat_rotation[i][j] = camera_description[CameraIndex::RotationMatrix][(4 * i) + j];
      }
    }

    osg::ref_ptr<osg::Image> bottom_image = osgDB::readImageFile(plane_description[PlaneIndex::fileName]);

    if (!bottom_image) {
      bottom_image = osgDB::readImageFile(baseDirectory(file) + plane_description[PlaneIndex::fileName]);
    }

    if (bottom_image) {
      osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();

      /* 1. Load the texture that will be projected */
      osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
      texture->setInternalFormat(GL_RGBA);
      texture->setResizeNonPowerOfTwoHint(false);
      texture->setBorderColor(osg::Vec4(0, 0, 0, 0));
      texture->setImage(bottom_image.get());

      texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_BORDER);
      texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_BORDER);
      texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_BORDER);
      texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
      texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
      stateset->setTextureAttributeAndModes(texture_unit, texture.get(), osg::StateAttribute::ON);

      // Set up tex gens
      stateset->setTextureMode(texture_unit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
      stateset->setTextureMode(texture_unit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
      stateset->setTextureMode(texture_unit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
      stateset->setTextureMode(texture_unit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

      /* 2. Load the Shaders */
      osg::ref_ptr<osg::Program> program(new osg::Program());

      osg::ref_ptr<osg::Shader> vertex_shader(
          osg::Shader::readShaderFile(osg::Shader::VERTEX, "shaders/VertexShader.glsl"));
      osg::ref_ptr<osg::Shader> frag_shader(
          osg::Shader::readShaderFile(osg::Shader::FRAGMENT, "shaders/FragmentShader.glsl"));

      program->addShader(vertex_shader.get());
      program->addShader(frag_shader.get());
      stateset->setAttribute(program.get());

      /* 3. Handover the texture to the fragment shader via uniform */
      osg::ref_ptr<osg::Uniform> tex_uniform = new osg::Uniform(osg::Uniform::SAMPLER_2D, "projectionMap");
      tex_uniform->set(texture_unit);
      stateset->addUniform(tex_uniform.get());
      stateset->setRenderingHint(osg::StateSet::RenderingHint::TRANSPARENT_BIN);

      /* 4. set Texture matrix*/
      osg::ref_ptr<osg::TexMat> tex_mat = new osg::TexMat();
      osg::Matrix mat;

      // TODO(Eberty)
      osg::Vec3d position(-camera_description[CameraIndex::TranslationVector][0],
                          camera_description[CameraIndex::TranslationVector][1],
                          -camera_description[CameraIndex::TranslationVector][2]);

      osg::Quat orientation;
      orientation.set(osg::Matrixd::rotate(osg::PI, osg::Y_AXIS) * osg::Matrixd(*mat_rotation));

      osg::Matrixd matrix;
      matrix.setTrans(position);
      matrix.setRotate(orientation);
      matrix.invert(matrix);
      const osg::Matrixd WORLD2OSGCAM = osg::Matrixd::rotate(osg::PI_4/32, osg::Y_AXIS) * osg::Matrixd::rotate(osg::PI_4/128, osg::Y_AXIS) * osg::Matrixd::rotate(osg::PI_2, osg::Y_AXIS) *
                                        osg::Matrixd::rotate(-osg::PI_4/32, osg::X_AXIS) * osg::Matrixd::rotate(osg::PI_4/2, osg::X_AXIS);
      osg::Matrixd m = matrix * WORLD2OSGCAM;
      osg::Vec3 eye, center, up;
      m.getLookAt(eye, center, up);

      osg::Matrixd mtr =  osg::Matrixd::rotate(-osg::PI_4, osg::X_AXIS) * osg::Matrixd::rotate(osg::PI_4/2, osg::X_AXIS) * osg::Matrixd(*mat_rotation);
      float roll = atan2(mtr(2, 1), mtr(2, 2));
      float pitch = asin(mtr(2, 0));
      float yaw = -atan2(mtr(1, 0), mtr(0, 0));
      osg::ref_ptr<osg::Uniform> normal_uniform = new osg::Uniform("projectionNormal", osg::Vec3(roll, pitch, yaw));
      stateset->addUniform(normal_uniform.get());

      float projector_angle = camera_description[CameraIndex::FocalMm][0] * 3.87;
      float asp_ratio = camera_description[CameraIndex::ViewportPx][0] / camera_description[CameraIndex::ViewportPx][1];

      osg::Matrixd look_at = osg::Matrixd::lookAt(eye, center, up);
      osg::Matrixd perspective = osg::Matrixd::perspective(projector_angle, asp_ratio, 0.1, 100);
      mat = look_at * perspective;

      tex_mat->setMatrix(mat);
      stateset->setTextureAttributeAndModes(texture_unit, tex_mat.get(), osg::StateAttribute::ON);
      return stateset.release();
    } else {
      std::cout << "Error on open image: " << plane_description[PlaneIndex::fileName] << std::endl;
    }
  } else {
    std::cout << "Error on read file: " << file << std::endl;
  }

  return NULL;
}

osg::Group* createModel(const std::string& mesh, const std::string& mlp) {
  /* Load the model which will be the receiver of out projection */
  osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(mesh);

  if (model) {
    osg::ref_ptr<osg::Group> root = new osg::Group();

    /* Add the model to graph */
    root->addChild(model);

    /* Enable projective texturing for all objects of this node */
    osg::ref_ptr<osg::StateSet> stateset = createProjectorState(mlp);
    if (stateset) {
      root->setStateSet(stateset);
      return root.release();
    }
  } else {
    std::cout << "Error on load mesh: " << mesh << std::endl;
  }

  return NULL;
}

osgViewer::Viewer* createViewer() {
  osg::ref_ptr<osgGA::TrackballManipulator> manipulator = new osgGA::TrackballManipulator();
  manipulator->setAllowThrow(false);

  osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer();
  viewer->setUpViewInWindow(0, 0, 640, 480);
  viewer->getCamera()->setClearColor(osg::Vec4(48.0 / 255.0, 48.0 / 255.0, 48.0 / 255.0, 1.0));
  viewer->setCameraManipulator(manipulator);
  viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
  viewer->addEventHandler(new osgViewer::ThreadingHandler());
  viewer->addEventHandler(new osgViewer::StatsHandler());

  osgViewer::Viewer::Windows windows;
  viewer->getWindows(windows);
  if (!windows.empty()) {
    windows.front()->setWindowName("Model viewer - Eberty Alves 2019");
  }

  return viewer.release();
}

bool endsWith(const std::string& str, const std::string& suffix) {
  return ((str.size() >= suffix.size()) && (str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0));
}

int main(int argc, char** argv) {
  if (argc > 2 && (endsWith(argv[1], ".obj") || endsWith(argv[1], ".ply")) && endsWith(argv[2], ".mlp")) {
    osg::ref_ptr<osg::Group> model = createModel(argv[1], argv[2]);
    if (model) {
      osgUtil::Optimizer optimizer;
      optimizer.optimize(model);

      osg::ref_ptr<osgViewer::Viewer> viewer = createViewer();
      viewer->setSceneData(model);
      return viewer->run();
    }
  } else {
    std::cout << "Usage: " << argv[0] << " <mesh_file.obj> <meshlab_project.mlp>" << std::endl << std::endl;
    std::cout << "Please, make sure that there is one MLRaster (camera) on mlp file" << std::endl;
  }

  return -1;
}
