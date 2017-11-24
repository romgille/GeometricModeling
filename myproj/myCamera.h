#pragma once
#include "mySegment.h"

class myCamera
{
public:
	//Camera positioning
	glm::vec3 camera_eye;
	glm::vec3 camera_up;
	glm::vec3 camera_forward;

	//Projection parameters
	float fovy;
	float zNear;
	float zFar;

	//Window parameters
	int window_width;
	int window_height;

	myCamera();
	~myCamera();
	void reset();
	void setWindowSize(int width, int height);

	void crystalball_rotateView(int dx, int dy);
	void firstperson_rotateView(int dx, int dy);
	void panView(int dx, int dy);

	mySegment constructRay(int x, int y) const;
	
	glm::mat4 projectionMatrix() const;
	glm::mat4 viewMatrix() const;
	
	void moveForward(float size);
	void moveBack(float size);
	void turnLeft(float size);
	void turnRight(float size);

	void print() const;
};
