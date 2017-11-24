#pragma once
#include "glm/glm.hpp"

#define varname(name) #name

const int MAX_VERTEX_DEGREE = 10000;
const int MAX_FACE_DEGREE = 10000;

const int TOPPANELGUI_HEIGHT = 220;
const float OUTPUTWINDOW_HEIGHT_PROPORTION = 0.2f;
const int LEFTPANELGUI_WIDTH = 400;

const int NUM_BUFFERS = 10;
const glm::vec3 DEFAULT_CAMERA_EYE = glm::vec3(-0.063502, -0.123408, 0.768526);
const glm::vec3 DEFAULT_CAMERA_UP = glm::vec3(-0.000638937, 0.97079, 0.239931);
const glm::vec3 DEFAULT_CAMERA_FORWARD = glm::vec3(0.0105081, 0.230462, -0.973025);

const float DEFAULT_FOVY = 45.0f;
const float DEFAULT_zNEAR = 0.01f;
const float DEFAULT_zFAR = 2000.0f;

const int DEFAULT_WINDOW_HEIGHT = 1424;
const int DEFAULT_WINDOW_WIDTH = 2024;

const glm::vec4 DEFAULT_KD = glm::vec4(0, 1, 0, 0);
const glm::vec4 DEFAULT_KS = glm::vec4(0, 0, 0, 0);
const glm::vec4 DEFAULT_KA = glm::vec4(0.1, 0.1, 0.1, 0);
const float DEFAULT_SPECULAR_COEFFICIENT = 20.0f;

const glm::vec4 INITIAL_BACKGROUNDCOLOR = glm::vec4(0.8f, 0.9f, 1.0f, 0.0f);

const float DEFAULT_KEY_MOVEMENT_STEPSIZE = 1.0f;
const float DEFAULT_MOUSEWHEEL_MOVEMENT_STEPSIZE = 0.5f;
const float DEFAULT_LEFTRIGHTTURN_MOVEMENT_STEPSIZE = 0.01f;

const glm::vec3 DEFAULT_LIGHT_POSITION = glm::vec3(0, 0, 0);
const glm::vec3 DEFAULT_LIGHT_INTENSITY = glm::vec3(1, 1, 1);
const glm::vec3 DEFAULT_LIGHT_DIRECTION = glm::vec3(0, 0, -1);

const glm::vec4 MYCOLOR_RED = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
const glm::vec4 MYCOLOR_BLUE = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
const glm::vec4 MYCOLOR_WHITE = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
const glm::vec4 MYCOLOR_GREEN = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
const glm::vec4 MYCOLOR_BLACK = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
const glm::vec4 MYCOLOR_MESHCOLOR = glm::vec4(0.4f, 0.8f, 0.4f, 1.0f);
const glm::vec4 MYCOLOR_LIGHTGRAY = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
const glm::vec4 MYCOLOR_YELLOW = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

