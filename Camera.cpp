#include "Camera.h"

Camera::Camera() {
	reset();
}

Camera::~Camera() {
}

void Camera::reset() {
	orientLookAt(glm::vec3(0.0f, 0.0f, DEFAULT_FOCUS_LENGTH),
		glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	setViewAngle(VIEW_ANGLE);
	setNearPlane(NEAR_PLANE);
	setFarPlane(FAR_PLANE);
	screenWidth = screenHeight = 200;
	screenWidthRatio = 1.0f;
	rotU = rotV = rotW = 0;
}

//called by main.cpp as a part of the slider callback for controlling rotation
// the reason for computing the diff is to make sure that we are only incrementally rotating the camera
void Camera::setRotUVW(float u, float v, float w) {
	float diffU = u - rotU;
	float diffV = v - rotV;
	float diffW = w - rotW;
	rotateU(diffU);
	rotateV(diffV);
	rotateW(diffW);
	rotU = u;
	rotV = v;
	rotW = w;
}

void Camera::orientLookAt(glm::vec3 eyePoint, glm::vec3 lookatPoint, glm::vec3 upVec) {
	eyePosition.x = eyePoint.x;
	eyePosition.y = eyePoint.y;
	eyePosition.z = eyePoint.z;
	eyePosition.w = 1.0f;

	glm::vec3 v = lookatPoint - eyePoint;
	glm::vec4 lookV, upV;

	lookV.x = v.x;
	lookV.y = v.y;
	lookV.z = v.z;
	lookV.w = 0.0f;
	lookV = glm::normalize(lookV);

	// Maybe we should constrain upVec to not == lookVec
	upV.x = upVec.x;
	upV.y = upVec.y;
	upV.z = upVec.z;
	upV.w = 0.0f;
	upV = glm::normalize(upV);

	setRotationMatrix(lookV, upV);
}

void Camera::orientLookVec(glm::vec3 eyePoint, glm::vec3 lookVec, glm::vec3 upVec) {
	eyePosition.x = eyePoint.x;
	eyePosition.y = eyePoint.y;
	eyePosition.z = eyePoint.z;
	eyePosition.w = 1.0f;

	glm::vec4 lookV, upV;

	lookV.x = lookVec.x;
	lookV.y = lookVec.y;
	lookV.z = lookVec.z;
	lookV.w = 0.0f;
	lookV = glm::normalize(lookV);

	// Maybe we should constrain upVec to not == lookVec
	upV.x = upVec.x;
	upV.y = upVec.y;
	upV.z = upVec.z;
	upV.w = 0.0f;
	upV = glm::normalize(upV);

	setRotationMatrix(lookV, upV);
}

// Returns the rotation matrix
glm::mat4 Camera::getRotationMatrix() {
	return rotationMatrix;
}

void Camera::setRotationMatrix(glm::vec4 lookV, glm::vec4 upV) {
	glm::vec3 w = glm::normalize(glm::vec3(-1.0f * lookV));
	glm::vec3 u = glm::normalize(glm::cross(glm::vec3(upV), w));
	glm::vec3 v = glm::cross(w, u);

	rotationMatrix = glm::mat4(1.0f);
	rotationMatrix[0][0] = u.x;
	rotationMatrix[1][0] = u.y;
	rotationMatrix[2][0] = u.z;

	rotationMatrix[0][1] = v.x;
	rotationMatrix[1][1] = v.y;
	rotationMatrix[2][1] = v.z;

	rotationMatrix[0][2] = w.x;
	rotationMatrix[1][2] = w.y;
	rotationMatrix[2][2] = w.z;
}

// Returns the scaling matrix
glm::mat4 Camera::getScaleMatrix() {
	glm::mat4 scaleMat4(1.0);
	float theta_h = (PI / 180) * viewAngle;

	float half_w = (tan(theta_h / 2.0f) * farPlane * (screenWidthRatio));

	float theta_w = atan(half_w / farPlane);
	scaleMat4[0][0] = 1 / (tan(theta_w) * farPlane);
	scaleMat4[1][1] = 1 / (tan(theta_h / 2) * farPlane);
	scaleMat4[2][2] = 1 / farPlane;

	return scaleMat4;
}

// Returns the inverse of the scaling matrix
glm::mat4 Camera::getInverseScaleMatrix() {
	glm::mat4 invScaleMat4(1.0f);
	glm::mat4 scale = getScaleMatrix();
	invScaleMat4[0][0] = 1 / scale[0][0];
	invScaleMat4[1][1] = 1 / scale[1][1];
	invScaleMat4[2][2] = 1 / scale[2][2];

	return invScaleMat4;
}

// Returns the translation matrix
glm::mat4 Camera::getTranslationMatrix() {
	glm::mat4 transMatrix(1.0);
	transMatrix[3][0] = -eyePosition.x;
	transMatrix[3][1] = -eyePosition.y;
	transMatrix[3][2] = -eyePosition.z;

	return transMatrix;
}

// Returns the inverse of the translation matrix
glm::mat4 Camera::getInverseTranslationMatrix() {
	glm::mat4 transMatrix(1.0);
	transMatrix[3][0] = eyePosition.x;
	transMatrix[3][1] = eyePosition.y;
	transMatrix[3][2] = eyePosition.z;

	return transMatrix;
}

// Returns the unhinging matrix
glm::mat4 Camera::getUnhingeMatrix() {
	glm::mat4 unhingeMat4(1.0f);
	float c = (-1.0f * nearPlane) / farPlane;
	unhingeMat4[2][2] = -1.0f / (c + 1);
	unhingeMat4[3][2] = c / (c + 1);
	unhingeMat4[2][3] = -1;
	unhingeMat4[3][3] = 0;

	return unhingeMat4;
}

// Creates and returns the projection matrix from the unhinging and scaling matrices
glm::mat4 Camera::getProjectionMatrix() {
	glm::mat4 unhinge = getUnhingeMatrix();
	glm::mat4 scale = getScaleMatrix();

	return unhinge * scale;
}

// Creates and returns the model view matrix from the rotation and scaling matrices
glm::mat4 Camera::getModelViewMatrix() {
	glm::mat4 rot = getRotationMatrix();
	glm::mat4 trans = getTranslationMatrix();

	return rot * trans;
}

// Creates and returns the inverse of the model view matrix from the inverse 
// translation matrix and the inverse rotation matrix
glm::mat4 Camera::getInverseModelViewMatrix() {
	return getInverseTranslationMatrix() * glm::transpose(getRotationMatrix());
}


// Translates the camera and look vector by v
void Camera::translate(glm::vec3 v) {
	eyePosition.x += v.x;
	eyePosition.y += v.y;
	eyePosition.z += v.z;

	rotationMatrix[0][2] += v.x;
	rotationMatrix[1][2] += v.y;
	rotationMatrix[2][2] += v.z;
}


//=====================================================================
//
//								Setters
//
//=====================================================================

// Sets the view angle to _viewAngle
void Camera::setViewAngle(float _viewAngle) {
	viewAngle = _viewAngle;
}

// Sets the near plane of the camera to _nearPlane
void Camera::setNearPlane(float _nearPlane) {
	nearPlane = _nearPlane;
}

// Sets the far plane of the camera to _farPlane
void Camera::setFarPlane(float _farPlane) {
	farPlane = _farPlane;
}

// Sets the screen size and aspect ratio
void Camera::setScreenSize(int _screenWidth, int _screenHeight) {
	screenWidth = _screenWidth;
	screenHeight = _screenHeight;
	screenWidthRatio = float(screenWidth) / screenHeight;
}

//========================================================================
//
//						ROTATION FUNCTIONS
//
//========================================================================

// Rotates the camera about the u axis (pitches up or down)
void Camera::rotateU(float degrees) {
	glm::vec3 u = glm::vec3(rotationMatrix[0][0], rotationMatrix[1][0], rotationMatrix[2][0]);

	rotationMatrix = rotationMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(-degrees), u);
}

// Rotates the camera about the v axis (yaws side to side)
void Camera::rotateV(float degrees) {
	glm::vec3 v = glm::vec3(rotationMatrix[0][1], rotationMatrix[1][1], rotationMatrix[2][1]);

	rotationMatrix = rotationMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(-degrees), v);
}

// Rotates the camera about the w axis (rolls clockwise or counterclockwise)
void Camera::rotateW(float degrees) {
	glm::vec3 w = glm::vec3(rotationMatrix[0][2], rotationMatrix[1][2], rotationMatrix[2][2]);

	rotationMatrix = rotationMatrix * glm::rotate(glm::mat4(1.0f), glm::radians(degrees), w);
}

// Rotates 'point' about 'a' by degrees
void Camera::rotate(glm::vec3 point, glm::vec3 a, float degrees) {
	glm::mat4 M1, M2, M3, M2_inv, M1_inv;

	glm::mat4 I = glm::mat4(1.0f);
	glm::vec3 x_axis = glm::vec3(1, 0, 0);
	glm::vec3 y_axis = glm::vec3(0, 1, 0);
	glm::vec3 z_axis = glm::vec3(0, 0, 1);

	float M1_angle = atan(a.z / a.x);
	float M2_angle = asin(a.y / glm::length(a));
	float lambda = (PI / 180) * degrees;

	M1 = glm::rotate(I, M1_angle, y_axis);
	M2 = glm::rotate(I, M2_angle, z_axis);
	M3 = glm::rotate(I, lambda, x_axis);
	M2_inv = glm::inverse(M2);
	M1_inv = glm::inverse(M1);

	glm::mat4 rotation_matrix = M2_inv * M3 * M2;

	// If the x-component of the axis is 0 we should not rotate it into the x-y plane
	// NOTE: This fails if the axis of rotation is the z-axis 
	if (fabs(a.x) >= .0001f) {
		rotation_matrix = M1_inv * rotation_matrix * M1;
	}

	glm::vec4 new_pt = rotation_matrix * glm::vec4(point, 1.0f);
}

//=====================================================================
//
//								Getters
//
//=====================================================================

glm::vec3 Camera::getEyePoint() {
	glm::vec3 v;
	v.x = eyePosition.x;
	v.y = eyePosition.y;
	v.z = eyePosition.z;
	return v;
}

glm::vec3 Camera::getLookVector() {
	return glm::vec3(-rotationMatrix[0][2], -rotationMatrix[1][2], -rotationMatrix[2][2]);
}

glm::vec3 Camera::getUpVector() {
	return glm::vec3(rotationMatrix[0][1], rotationMatrix[1][1], rotationMatrix[2][1]);
}

float Camera::getViewAngle() {
	return viewAngle;
}

float Camera::getNearPlane() {
	return nearPlane;
}

float Camera::getFarPlane() {
	return farPlane;
}

int Camera::getScreenWidth() {
	return screenWidth;
}

int Camera::getScreenHeight() {
	return screenHeight;
}

float Camera::getScreenWidthRatio() {
	return screenWidthRatio;
}
