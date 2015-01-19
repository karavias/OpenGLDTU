// 02561-13-01
#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "Angel.h"
#include "SimpleResourceLoader.h"

using namespace std;
using namespace Angel;

int WINDOW_WIDTH = 500;
int WINDOW_HEIGHT = 500;

GLuint shaderProgram;
GLuint projectionUniform,
	modelViewUniform,
	cameraPositionUniform,
	textureVolumeUniform;
GLuint positionAttribute;
GLuint unitCubeVertexArrayObject,
	vertexBuffer;

GLuint textureVolume;

struct Vertex {
    vec4 position;
};

void loadShader();
void display();
GLuint loadBufferData(Vertex* vertices, int vertexCount);
vec3 sphericalToCartesian(float polarAngle, float elevationAngle, float radius);

// spherical coordinates of camera position (angles in radian)
float sphericalPolarAngle = 0;
float sphericalElevationAngle = 0;
float sphericalRadius = 3;

vec2 mousePos;

void buildUnitCube() {
	const int cubeSize = 8;
	Vertex cubeData[cubeSize] = {
        { vec4( 0.5,  0.5,  0.5, 1.0 ) },
        { vec4( 0.5, -0.5,  0.5, 1.0 ) },
        { vec4( 0.5,  0.5, -0.5, 1.0 ) },
        { vec4( 0.5, -0.5, -0.5, 1.0 ) },
		{ vec4(-0.5,  0.5, -0.5, 1.0 ) },
		{ vec4(-0.5, -0.5, -0.5, 1.0 ) },
		{ vec4(-0.5,  0.5,  0.5, 1.0 ) },
		{ vec4(-0.5, -0.5,  0.5, 1.0 ) }
    };
	unitCubeVertexArrayObject = loadBufferData(cubeData, cubeSize);
}

GLuint loadBufferData(Vertex* vertices, int vertexCount) {
	GLuint vertexArrayObject;

    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);
    
	GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(positionAttribute);
    glVertexAttribPointer(positionAttribute, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);

	return vertexArrayObject;
}

void loadShader(){
	shaderProgram = InitShader("raytrace.vert",  "raytrace.frag", "fragColor");
	projectionUniform = glGetUniformLocation(shaderProgram, "projection");
	if (projectionUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'projection' uniform."<<endl;
	}
	modelViewUniform = glGetUniformLocation(shaderProgram, "modelView");
	if (modelViewUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'modelView' uniform."<<endl;
	}
	cameraPositionUniform = glGetUniformLocation(shaderProgram, "cameraPos");
	if (cameraPositionUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'cameraPos' uniform."<<endl;
	}
	textureVolumeUniform = glGetUniformLocation(shaderProgram, "textureVolume");
	if (textureVolumeUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'textureVolume' uniform."<<endl;
	} 
	positionAttribute = glGetAttribLocation(shaderProgram, "position");
	if (positionAttribute == GL_INVALID_INDEX) {
		cerr << "Shader did not contain/use the 'position' attribute." << endl;
	}

}

void drawWireUnitCube() {
	GLuint indices[36] = {
		0,1,3,
		0,3,2,
		2,3,5,
		2,5,4,
		4,5,7,
		4,7,6,
		6,7,1,
		6,1,0,
		0,2,4,
		0,4,6,
		1,7,5,
		1,5,3
	};
	glBindVertexArray(unitCubeVertexArrayObject);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, indices);
}

void display() {	
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	
    glUseProgram(shaderProgram);

	mat4 projection = Perspective(45,1,0.1,10);
	glUniformMatrix4fv(projectionUniform, 1, GL_TRUE, projection);

	vec3 eyePos = sphericalToCartesian(sphericalPolarAngle, sphericalElevationAngle, sphericalRadius);
	vec3 at(0,0,0);
	vec3 up(0,1,0);
	mat4 modelView = LookAt(eyePos, at, up);

	glUniformMatrix4fv(modelViewUniform, 1, GL_TRUE, modelView);

	// pass camera position uniform
	glUniform3fv(cameraPositionUniform, 1, eyePos);
	// bind textureVolume to slot 0 and set uniform
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_3D, textureVolume); // assign texture to slot 0
	glUniform1i(glGetUniformLocation(shaderProgram, "textureVolume"), 0); // assign slot 0 to uniform
	//textureVolumeUniform = 0;
	drawWireUnitCube();
	
	glutSwapBuffers();

	Angel::CheckError();

}

void reshape(int W, int H) {
    WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}


void mouse(int button, int state, int x, int y) {
	if (state==GLUT_DOWN) {
		mousePos = vec2(x,y);
	}
}

void mouseMovement(int x, int y){
	float rotationSpeed = 0.01;
	vec2 newMousePos = vec2(x,y);
	vec2 mousePosDelta = mousePos - newMousePos;
	sphericalPolarAngle += mousePosDelta.x*rotationSpeed;
	float rotate89Degrees = 89*DegreesToRadians;
	sphericalElevationAngle = min(rotate89Degrees,max(-rotate89Degrees,sphericalElevationAngle + mousePosDelta.y*rotationSpeed));
	mousePos = vec2(x,y);
	glutPostRedisplay();
}

void initRaycasting(char *filename) {
	glClearColor(0, 0, 0, 0);
		
	// Enable and load 3D texture
	SimpleResourceLoader resourceLoader(filename);
	std::string filenameStr = resourceLoader.Get("file_name");
	const char *name = filenameStr.c_str();
	int xsize = resourceLoader.GetInt("slice_width");
	int ysize = resourceLoader.GetInt("slice_height");
	int slices = resourceLoader.GetInt("slices");
	
	// the raw 3d texture with the size xsize X ysize X slices (each value is a single byte)
	unsigned char *texture = load3DTexture(name, xsize, ysize, slices);

	// Setup texture
	glGenTextures(1, &textureVolume);
	glBindTexture(GL_TEXTURE_3D, textureVolume);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, xsize, ysize, slices, 0, GL_RED, GL_UNSIGNED_BYTE, texture);

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

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_3_2_CORE_PROFILE);
	glutCreateWindow("02561-13-01");
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMovement);
	glutReshapeFunc(reshape);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	Angel::InitOpenGL();


	glEnable(GL_DEPTH_TEST);
	// Enable back-face culling 
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	//glEnable(GL_TEXTURE_3D);
	loadShader();
    buildUnitCube();
	initRaycasting("Teddybear.xml");

	Angel::CheckError();
	
	glutMainLoop();
}


// Convert from spherical coordinates to cartesian coordinates
vec3 sphericalToCartesian(float polarAngle, float elevationAngle, float radius){
	float a = radius * cos(elevationAngle);
	vec3 cart;
	cart.x = a * cos(polarAngle);
	cart.y = radius * sin(elevationAngle);
	cart.z = a * sin(polarAngle);
	return cart;
}