#pragma once

#ifndef MYGLCANVAS_H
#define MYGLCANVAS_H

#include <FL/gl.h>
#include <FL/glut.h>
#include <FL/glu.h>
#include <glm/glm.hpp>
#include <time.h>
#include <iostream>

#include "SceneObject.h"
#include "Camera.h"

#define SPLINE_SIZE 100
#define COASTER_SPEED 0.0001

class MyGLCanvas : public Fl_Gl_Window {
public:

	// Length of our spline (i.e how many points do we randomly generate)


	glm::vec3 eyePosition;
	glm::vec3 rotVec;
	glm::vec3 lookatPoint;

	int wireframe;
	int  viewAngle;
	float clipNear;
	float clipFar;

	MyGLCanvas(int x, int y, int w, int h, const char *l = 0);
	~MyGLCanvas();

private:
	glm::vec3 generateRay(int pixelX, int pixelY);
	glm::vec3 getEyePoint();
	glm::vec3 getIsectPointWorldCoord(glm::vec3 eye, glm::vec3 ray, float t);
	double intersect(glm::vec3 eyePointP, glm::vec3 rayV, glm::mat4 transformMatrix);

	void draw();
	void drawScene();

	void drawAxis();
	void drawGrid();

	int handle(int);
	void resize(int x, int y, int w, int h);
	void updateCamera(int width, int height);

	SceneObject* myObject;
	Camera camera;
	bool castRay;
	bool drag;
	glm::vec3 oldCenter;
	glm::vec3 oldIsectPoint;
	float oldT;

	//// Used for intersection
	glm::vec3 spherePosition;

	int mouseX = 0;
	int mouseY = 0;


};

#endif // !MYGLCANVAS_H