#include "MyGLCanvas.h"
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <iostream>

using namespace std;

std::ostream& operator<< (std::ostream& out, const glm::vec3& vec) {
	out << "{"
		<< vec.x << " " << vec.y << " " << vec.z
		<< "}";

	return out;
}


std::ostream& operator<< (std::ostream& out, const glm::mat4& mat) {
	out << "\n{"
		<< "[" << mat[0][0] << " " << mat[1][0] << " " << mat[2][0] << " " << mat[3][0] << "]\n"
		<< "[" << mat[0][1] << " " << mat[1][1] << " " << mat[2][1] << " " << mat[3][1] << "]\n"
		<< "[" << mat[0][2] << " " << mat[1][2] << " " << mat[2][2] << " " << mat[3][2] << "]\n"
		<< "[" << mat[0][3] << " " << mat[1][3] << " " << mat[2][3] << " " << mat[3][3] << "]"
		<< "}";

	return out;
}


MyGLCanvas::MyGLCanvas(int x, int y, int w, int h, const char *l) : Fl_Gl_Window(x, y, w, h, l) {
	mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE);
	
	eyePosition = glm::vec3(0.0f, 0.0f, 3.0f);
	lookatPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	rotVec = glm::vec3(0.0f, 0.0f, 0.0f);

	wireframe = 0;
	viewAngle = 60;
	clipNear = 0.01f;
	clipFar = 10.0f;

	castRay = false;
	drag = false;
	mouseX = 0;
	mouseY = 0;
	spherePosition = glm::vec3(0, 0, 0);

	myObject = new SceneObject(175);
	camera.setViewAngle(viewAngle);
	camera.setNearPlane(clipNear);
	camera.setFarPlane(clipFar);
	// Set the mode so we are modifying our objects.
	camera.orientLookVec(eyePosition, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
}

MyGLCanvas::~MyGLCanvas() {
}

/* The generateRay function accepts the mouse click coordinates
	(in x and y, which will be integers between 0 and screen width and 0 and screen height respectively).
   The function returns the ray
*/
glm::vec3 MyGLCanvas::generateRay(int pixelX, int pixelY) {
	glm::vec4 lookAtPoint(-1.0f + 2.0f * (float)pixelX / w(), 1.0f - 2.0f * (float)pixelY / h(), -1.0f, 1.0f);

	glm::vec4 eyePoint(getEyePoint(), 1.0f);

	glm::mat4 invMat = camera.getInverseModelViewMatrix() * camera.getInverseScaleMatrix();

	// invMat used to convert the points to object space in my(Danielle's) A4 implementation 
	lookAtPoint = invMat * lookAtPoint;
	eyePoint = invMat * eyePoint;

	glm::vec3 ray = lookAtPoint - eyePoint;

	ray = glm::normalize(ray);
	return ray;
}

// world
glm::vec3 MyGLCanvas::getEyePoint() {
	glm::vec3 eye(0, 0, 0);
	return eye;
}

/* The getIsectPointWorldCoord function accepts three input parameters:
	(1) the eye point (in world coordinate)
	(2) the ray vector (in world coordinate)
	(3) the "t" value

	The function should return the intersection point on the sphere
*/
glm::vec3 MyGLCanvas::getIsectPointWorldCoord(glm::vec3 eye, glm::vec3 ray, float t) {
	glm::vec3 p = glm::vec3(0, 0, 3) + t * ray;
	return p; 
}

/* The intersect function accepts three input parameters:
	(1) the eye point (in world coordinate)
	(2) the ray vector (in world coordinate)
	(3) the transform matrix that would be applied to there sphere to transform it from object coordinate to world coordinate

	The function should return:
	(1) a -1 if no intersection is found
	(2) OR, the "t" value which is the distance from the origin of the ray to the (nearest) intersection point on the sphere
*/
double MyGLCanvas::intersect(glm::vec3 eyePointP, glm::vec3 rayV, glm::mat4 transformMatrix) {
	cerr << transformMatrix << endl;

	transformMatrix = glm::inverse(transformMatrix);

	glm::vec3 eyeObject = transformMatrix * glm::vec4(eyePointP, 1.0);
	glm::vec3 rayObject = transformMatrix * glm::vec4(rayV, 0.0);
	//glm::vec3 eyeObject = eyePointP;
	//glm::vec3 rayObject = rayV;

	double A = glm::dot(rayObject, rayObject);
	double B = 2.0 * glm::dot(eyeObject, rayObject);
	double C = glm::dot(eyeObject, eyeObject) - 0.25;

	double determinant = (B * B) - (4.0 * A * C);

	if (determinant < 0.0)
		return -1.0;
	if (determinant == 0.0)
		return (-1.0 * B) / (2.0 * A);

	double tminus = ((-1.0 * B) - sqrt(determinant)) / (2.0 * A);
	double tplus = ((-1.0 * B) + sqrt(determinant)) / (2.0 * A);
	return std::min(tminus, tplus);
}


void MyGLCanvas::draw() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!valid()) {  //this is called when the GL canvas is set up for the first time or when it is resized...
		printf("establishing GL context");

		// Set the base texture of our object. Note that loading gl texture can 
		//  only happen after the gl context has been established
		if (myObject->baseTexture == NULL) {
			myObject->setTexture(0, "./data/pink.ppm");
		}
		// Set a second texture layer to our object
		if (myObject->blendTexture == NULL) {
			myObject->setTexture(1, "./data/smile.ppm");
		}

		glViewport(0, 0, w(), h());
		updateCamera(w(), h());

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		//glShadeModel(GL_SMOOTH);
		glShadeModel(GL_FLAT);

		GLfloat light_pos0[] = { eyePosition.x, eyePosition.y, eyePosition.z, 0.0f };
		GLfloat ambient[] = { 0.7f, 0.7f, 0.7f, 1.0f };
		GLfloat diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };

		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_POSITION, light_pos0);

		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);

		/****************************************/
		/*          Enable z-buferring          */
		/****************************************/

		glEnable(GL_DEPTH_TEST);
		glPolygonOffset(1, 1);
	}

	// Clear the buffer of colors in each bit plane.
	// bit plane - A set of bits that are on or off (Think of a black and white image)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawScene();
}

void MyGLCanvas::drawScene() {
	glMatrixMode(GL_MODELVIEW);
	// Set the mode so we are modifying our objects.
	camera.orientLookVec(eyePosition, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	glLoadMatrixf(glm::value_ptr(camera.getModelViewMatrix()));

	if (castRay == true) {
		
		// And here lies the problem: getEyePoint returns in camera space, but we want world space now
		//glm::vec3 eyePointP = getEyePoint();
		glm::vec3 eyePointP(0, 0, 3);


		glm::vec3 rayV = generateRay(mouseX, mouseY);
		glm::vec3 sphereTransV(spherePosition[0], spherePosition[1], spherePosition[2]);

		float t = intersect(eyePointP, rayV, glm::translate(glm::mat4(1.0), sphereTransV));
		glm::vec3 isectPointWorldCoord = getIsectPointWorldCoord(eyePointP, rayV, t);

		if (t > 0) {
			glColor3f(1, 0, 0);
			glPushMatrix();
				glTranslated(spherePosition[0], spherePosition[1], spherePosition[2]);
				glutWireCube(1.0f);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(isectPointWorldCoord[0], isectPointWorldCoord[1], isectPointWorldCoord[2]);
				glutSolidSphere(0.05f, 10, 10);
			glPopMatrix();
			printf("hit!\n");
		}
		else {
			printf("miss!\n");
		}
	}

	glPushMatrix();

	//move the sphere to the designated position
	glTranslated(spherePosition[0], spherePosition[1], spherePosition[2]);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glColor3f(1.0, 1.0, 1.0);
	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	glPushMatrix();
		//glRotatef(90, 0, 1, 0);
		glRotatef(90, 0, 1, 0);
		myObject->drawTexturedSphere();
	glPopMatrix();

	glPopMatrix();
}


void MyGLCanvas::updateCamera(int width, int height) {
	float xy_aspect;
	xy_aspect = (float)width / (float)height;

	camera.setScreenSize(width, height);

	// Determine if we are modifying the camera(GL_PROJECITON) matrix(which is our viewing volume)
	// Otherwise we could modify the object transormations in our world with GL_MODELVIEW
	glMatrixMode(GL_PROJECTION);
	// Reset the Projection matrix to an identity matrix
	glLoadIdentity();
	glm::mat4 projection = camera.getProjectionMatrix();
	glLoadMatrixf(glm::value_ptr(projection));
}


int MyGLCanvas::handle(int e) {
	//printf("Event was %s (%d)\n", fl_eventnames[e], e);
	switch (e) {
	case FL_DRAG:
		mouseX = (int)Fl::event_x();
		mouseY = (int)Fl::event_y();
		if (drag == true) {
			printf("drag and move\n");
			//TODO: compute the new spherePosition as you drag your mouse. spherePosition represents the coordinate for the center of the sphere
			//HINT: use the old t value (computed from when you first intersect the sphere (before dragging starts)) to determine the new spherePosition
			
			glm::vec3 eyePointP(0, 0, 3);
			glm::vec3 rayV = generateRay(mouseX, mouseY);
			
			glm::vec3 isectPointWorldCoord = getIsectPointWorldCoord(eyePointP, rayV, oldT);

			glm::vec3 transWorld = isectPointWorldCoord - oldIsectPoint;

			oldIsectPoint = isectPointWorldCoord;
			
			//cerr << transWorld << endl;
			
			spherePosition = transWorld + spherePosition;
		}
		return (1);
	case FL_MOVE:
		Fl::belowmouse(this);
		//printf("mouse move event (%d, %d)\n", (int)Fl::event_x(), (int)Fl::event_y());
		mouseX = (int)Fl::event_x();
		mouseY = (int)Fl::event_y();

		break;
	case FL_PUSH:
		printf("mouse push\n");
		if ((Fl::event_button() == FL_LEFT_MOUSE) && (castRay == false)) { //left mouse click -- casting Ray
			castRay = true;
		}
		else if ((Fl::event_button() == FL_RIGHT_MOUSE) && (drag == false)) { //right mouse click -- dragging
			//this code is run when the dragging first starts (i.e. the first frame). 
			//it stores a bunch of values about the sphere's "original" position and information
			//glm::vec3 eyePointP = getEyePoint();
			glm::vec3 eyePointP(0, 0, 3);
			glm::vec3 rayV = generateRay(mouseX, mouseY);
			glm::vec3 sphereTransV(spherePosition[0], spherePosition[1], spherePosition[2]);
			float t = intersect(eyePointP, rayV, glm::translate(glm::mat4(1.0), sphereTransV));
			glm::vec3 isectPointWorldCoord = getIsectPointWorldCoord(eyePointP, rayV, t);

			if (t > 0) {
				drag = true;
				printf("drag is true\n");
				oldCenter = spherePosition;
				oldIsectPoint = isectPointWorldCoord;
				oldT = t;
			}
		}
		return (1);
	case FL_RELEASE:
		printf("mouse release\n");
		if (Fl::event_button() == FL_LEFT_MOUSE) {
			castRay = false;
		}
		else if (Fl::event_button() == FL_RIGHT_MOUSE) {
			drag = false;
		}
		return (1);
	case FL_KEYUP:
		printf("keyboard event: key pressed: %c\n", Fl::event_key());
		switch (Fl::event_key()) {
		case 'w': eyePosition.y += 0.05f;  break;
		case 'a': eyePosition.x += 0.05f; break;
		case 's': eyePosition.y -= 0.05f;  break;
		case 'd': eyePosition.x -= 0.05f; break;
		}
		updateCamera(w(), h());
		break;
	case FL_MOUSEWHEEL:
		printf("mousewheel: dx: %d, dy: %d\n", Fl::event_dx(), Fl::event_dy());
		eyePosition.z += Fl::event_dy() * -0.05f;
		updateCamera(w(), h());
		break;
	}

	return Fl_Gl_Window::handle(e);
}

void MyGLCanvas::resize(int x, int y, int w, int h) {
	Fl_Gl_Window::resize(x, y, w, h);
	puts("resize called");
}

void MyGLCanvas::drawAxis() {
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0, 0, 0); glVertex3f(1.0, 0, 0);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(0, 0, 0); glVertex3f(0.0, 1.0, 0);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(0, 0, 0); glVertex3f(0, 0, 1.0);
	glEnd();
	glEnable(GL_LIGHTING);
}