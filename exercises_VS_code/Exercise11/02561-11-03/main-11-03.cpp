#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Angel.h"
#include "NURBSCurve.h"
#include "NURBSRenderer.h"

using namespace std;
using namespace Angel;

mat4 projection;

NURBSRenderer *lineRenderer = NULL;
NURBSRenderer *bezierRenderer = NULL;
NURBSRenderer *uniformBSplineRenderer = NULL;
NURBSRenderer *nonUniformBSplineRenderer = NULL;
NURBSRenderer *nurbsRenderer = NULL;
NURBSRenderer *current = NULL;
bool renderPoints = true;

vec3 controlPoints [7] = {
	vec3(-10.,0.,0.),
	vec3(-10.,5.,0.),
	vec3(-5.,5.,0.),
	vec3(0.,10.,0.),
	vec3(5.,5.,0.),
	vec3(10.,5.,0.),
	vec3(10.,0.,0.)};

void display() {	
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);	
	
	mat4 modelView = LookAt(vec4(0,5,1,1), vec4(0,5,0,1), vec4(0,1,0,0)); // identity modelView matrix
	
	if (current != NULL){
		current->render(projection, modelView);
	}
	
	if (renderPoints){
		lineRenderer->renderControlPoints(projection, modelView);
	}

	glutSwapBuffers();
	Angel::CheckError();
}

void reshape(int w, int h) {
	if (w <= h)
      projection = Ortho2D (-12.0, 12.0, -12.0*(GLfloat)h/(GLfloat)w, 
               12.0*(GLfloat)h/(GLfloat)w);
   else
      projection = Ortho2D(-12.0*(GLfloat)w/(GLfloat)h, 
               12.0*(GLfloat)w/(GLfloat)h, -12.0, 12.0);
    glViewport(0, 0, w, h);
}

void setupLines(){
	const int numberOfControlPoints = 7;
	NURBSCurve *line = new NURBSCurve(numberOfControlPoints);
	for (int i=0;i<numberOfControlPoints;i++){
		line->setControlPoint(i, controlPoints[i]);
	}
	const int numberOfKnots = 8;
	float knotVector[numberOfKnots] = {0,1,2,3,4,5,6,7};
	line->setKnotVector(numberOfKnots, knotVector);
	cout << "Piecewise line curve: degree "<< line->getDegree() << " Order " << line->getOrder() << endl;
	lineRenderer = new NURBSRenderer(line);
}

void setupBezier(){
	const int numberOfControlPoints = 4;
	NURBSCurve *bezier = new NURBSCurve(numberOfControlPoints);
	for (int i = 0; i<numberOfControlPoints; i++){
		bezier->setControlPoint(i, controlPoints[i]);
	}
	const int numberOfKnots = 8;
	float knotVector[numberOfKnots] = { 0, 0, 0, 0, 1, 1, 1, 1 };
	bezier->setKnotVector(numberOfKnots, knotVector);
	cout << "Bezier curve: degree " << bezier->getDegree() << " Order " << bezier->getOrder() << endl;
	bezierRenderer = new NURBSRenderer(bezier);
}

void setupUniformBSpline(){
	const int numberOfControlPoints = 7;
	NURBSCurve *bSpline = new NURBSCurve(numberOfControlPoints);
	for (int i = 0; i<numberOfControlPoints; i++){
		bSpline->setControlPoint(i, controlPoints[i]);
	}
	const int numberOfKnots = 11;
	float knotVector[numberOfKnots] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	bSpline->setKnotVector(numberOfKnots, knotVector);
	cout << "bSpline curve: degree " << bSpline->getDegree() << " Order " << bSpline->getOrder() << endl;
	uniformBSplineRenderer = new NURBSRenderer(bSpline);
}

void setupNonUniformBSpline(){
	const int numberOfControlPoints = 7;
	NURBSCurve *bSpline = new NURBSCurve(numberOfControlPoints);
	for (int i = 0; i<numberOfControlPoints; i++){
		bSpline->setControlPoint(i, controlPoints[i]);
	}
	const int numberOfKnots = 11;
	float knotVector[numberOfKnots] = { 0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4 };
	bSpline->setKnotVector(numberOfKnots, knotVector);
	cout << "non uniform bSpline curve: degree " << bSpline->getDegree() << " Order " << bSpline->getOrder() << endl;
	nonUniformBSplineRenderer = new NURBSRenderer(bSpline);
}

void setupNURBS(){
	const int numberOfControlPoints = 7;
	NURBSCurve *nurbs = new NURBSCurve(numberOfControlPoints);
	float w[numberOfControlPoints] = { 1, 30, 200, 500, 200, 30, 1 };
	for (int i = 0; i<numberOfControlPoints; i++){
		nurbs->setControlPoint(i, vec4(controlPoints[i], w[i]));
	}
	const int numberOfKnots = 11;
	float knotVector[numberOfKnots] = { 0, 0, 0, 0, 1, 2, 3, 4, 4, 4, 4 };
	nurbs->setKnotVector(numberOfKnots, knotVector);
	cout << "nurbs curve: degree " << nurbs->getDegree() << " Order " << nurbs->getOrder() << endl;
	nurbsRenderer = new NURBSRenderer(nurbs);
}

void keyboard(unsigned char c, int x, int y){
	switch (c){
	case '1':
		current = lineRenderer;
		glutSetWindowTitle("Lines");
		break;
	case '2':
		current = bezierRenderer;
		glutSetWindowTitle("Bezier");
		break;
	case '3':
		current = uniformBSplineRenderer;
		glutSetWindowTitle("UniformBSpline");
		break;
		break;
	case '4':
		current = nonUniformBSplineRenderer;
		glutSetWindowTitle("NonUniformBSpline");
		break;
	case '5':
		current = nurbsRenderer;
		glutSetWindowTitle("NURBS");
		break;
	case 'p':
	case 'P':
		renderPoints = !renderPoints;
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char* argv[]) {
	
    glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    
	glutSetOption(
        GLUT_ACTION_ON_WINDOW_CLOSE,
        GLUT_ACTION_GLUTMAINLOOP_RETURNS
    );

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_3_2_CORE_PROFILE);
	glutCreateWindow("02561-11-03");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutReshapeWindow(500, 500);

	Angel::InitOpenGL();
		
	setupLines();
	setupBezier();
	setupUniformBSpline();
	setupNonUniformBSpline();
	setupNURBS();

	keyboard('1', 0, 0);

	Angel::CheckError();


	glutMainLoop();
}
