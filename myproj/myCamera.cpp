#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>    

#include "myCamera.h"
#include <iostream>
#include "default_constants.h"
#include "helper_functions.h"

using namespace std;


myCamera::myCamera()
{
	reset();
}

myCamera::~myCamera()
{
}

void myCamera::reset()
{
	camera_eye = DEFAULT_CAMERA_EYE;
	camera_up = DEFAULT_CAMERA_UP;
	camera_forward = DEFAULT_CAMERA_FORWARD;

	fovy = DEFAULT_FOVY;
	zNear = DEFAULT_zNEAR;
	zFar = DEFAULT_zFAR;
}

void myCamera::setWindowSize(int width, int height)
{
	window_width = width;
	window_height = height;
}

void myCamera::crystalball_rotateView(int dx, int dy)
{
	float vx = static_cast<float>(dx) / static_cast<float>(window_width);
	float vy = -static_cast<float>(dy) / static_cast<float>(window_height);

	float theta = 4.0f * (fabs(vx) + fabs(vy));

	glm::vec3 camera_right = glm::normalize(glm::cross(camera_forward, camera_up));
	glm::vec3 tomovein_direction = -camera_right * vx + -camera_up * vy;

	glm::vec3 rotation_axis = glm::normalize(glm::cross(tomovein_direction, camera_forward));

	rotate(camera_forward, rotation_axis, theta, true);
	rotate(camera_up, rotation_axis, theta, true);
	rotate(camera_eye, rotation_axis, theta, false);
}

void myCamera::firstperson_rotateView(int dx, int dy)
{
	float vx = static_cast<float>(dx) / static_cast<float>(window_width);
	float vy = -static_cast<float>(dy) / static_cast<float>(window_height);

	float theta = 4.0f * (fabs(vx) + fabs(vy));

	glm::vec3 camera_right = glm::normalize(glm::cross(camera_forward, camera_up));
	glm::vec3 tomovein_direction = -camera_right * vx + -camera_up * vy;

	glm::vec3 rotation_axis = glm::normalize(glm::cross(tomovein_direction, camera_forward));

	rotate(camera_forward, rotation_axis, theta, true);
}

void myCamera::panView(int dx, int dy)
{
	float vx = static_cast<float>(dx) / static_cast<float>(window_width);
	float vy = -static_cast<float>(dy) / static_cast<float>(window_height);

	glm::vec3 camera_right = glm::normalize(glm::cross(camera_forward, camera_up));
	glm::vec3 tomovein_direction = -camera_right * vx + -camera_up * vy;

	camera_eye += 1.6f * tomovein_direction;
}

mySegment myCamera::constructRay(int x, int y) const
{
	glm::mat4 projection_matrix = projectionMatrix( );
	glm::mat4 view_matrix = viewMatrix();

	y = window_height - y;

	float x_c = (2.0f*x) / static_cast<float>(window_width) - 1.0f;
	float y_c = (2.0f*y) / static_cast<float>(window_height) - 1.0f;

	glm::vec4 tmp = glm::vec4(x_c, y_c, -1.0f, 1.0f);
	tmp = glm::inverse(projection_matrix) * tmp;
	tmp.z = -1.0f; tmp.w = 0.0f;

	mySegment ray;
	ray.p1 = camera_eye;
	ray.p2 = camera_eye + glm::normalize(glm::vec3(glm::inverse(view_matrix) * tmp));
	return ray;
}

glm::mat4 myCamera::projectionMatrix() const
{
	return glm::perspective(glm::radians(fovy), static_cast<float>(window_width) / static_cast<float>(window_height), zNear, zFar);
}

glm::mat4 myCamera::viewMatrix() const
{
	return glm::lookAt(camera_eye, camera_eye + camera_forward, camera_up);
}

void myCamera::moveForward(float size)
{
	camera_eye += size * camera_forward;
}

void myCamera::moveBack(float size)
{
	camera_eye -= size * camera_forward;
}

void myCamera::turnLeft(float size)
{
	rotate(camera_forward, camera_up, size, true);
}

void myCamera::turnRight(float size)
{
	rotate(camera_forward, camera_up, -size, true);
}

void myCamera::print() const
{
	cout << "Eye: (" << camera_eye.x << ", " << camera_eye.y << ", " << camera_eye.z << ")" << endl;
	cout << "Forward: (" << camera_forward.x << ", " << camera_forward.y << ", " << camera_forward.z << ")" << endl;
	cout << "Up: (" << camera_up.x << ", " << camera_up.y << ", " << camera_up.z << ")" << endl;
}
