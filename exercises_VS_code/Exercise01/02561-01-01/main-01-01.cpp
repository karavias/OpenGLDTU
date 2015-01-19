// 02561-01-01 Hello triangle world

#include <iostream>
#include <string>


#include "Angel.h"

using namespace std;
using namespace Angel;


//define size of window in pixels
int WINDOW_WIDTH = 600;
int WINDOW_HEIGHT = 600;


GLuint shaderProgram;
GLuint colorAttribute, positionAttribute;
GLuint vertexArrayObject;
GLuint vertexArrayObject2;
//define vertex as 2 dimensional points and RGB color values
struct Vertex{
	vec2 position;
	vec3 color;
};

const int NUMBER_OF_VERTICES = 3;  
Vertex vertexData[NUMBER_OF_VERTICES] = {  //define a single shape with 6 vertices, each point having RGB color values
	//original triangle
	{ vec2(-0.5, -0.5 ), vec3( 1.0, 0.0, 0.0) },  //use red
	{ vec2( 0.5, -0.5 ), vec3( 0.0, 1.0, 0.0 ) }, //use green
	{ vec2( 0.5,  0.5 ), vec3( 0.0, 0.0, 1.0 ) }, //use blue

};

Vertex vertexData2[NUMBER_OF_VERTICES] = {  //define a single shape with 6 vertices, each point having RGB color values
	//new triangle
	{ vec2(-0.5, -0.5 ), vec3( 1.0, 1.0, 1.0) }, //use white
	{ vec2( -0.5, 0.5 ), vec3( 1.0, 1.0, 1.0 ) },
	{ vec2( 0.5,  0.5 ), vec3( 1.0, 1.0, 1.0 ) }
};


	
// forward declaration
void loadShader();
void display();
void loadBufferData();
void reshape(int W, int H);

// Display buffers on the screen
void display() {	
	//Clears the previous image
	glClearColor(0.0, 0.0, 0.0, 1.0); //initialize display
	glClear(GL_COLOR_BUFFER_BIT);
	//Select a shader for the resulting image
	glUseProgram(shaderProgram);
	//Make one object from the vertices.
	glBindVertexArray(vertexArrayObject);
    //Draw the vertices(triangles) on the buffer.
	glDrawArrays(GL_TRIANGLES, 0, NUMBER_OF_VERTICES);

	//Make one object from the vertices.
	glBindVertexArray(vertexArrayObject2);
    //Draw the vertices(triangles) on the buffer.
	glDrawArrays(GL_TRIANGLES, 0, NUMBER_OF_VERTICES);

	//change the buffer to display the result
	glutSwapBuffers();
	//error checking
	Angel::CheckError();
}

//Fixes the aspect ratio.
void reshape(int windowWidth, int windowHeight) {
	glViewport(0, 0, windowWidth, windowHeight);
}

//Write to the active buffer
void loadBufferData() {
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
	GLuint  vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, NUMBER_OF_VERTICES * sizeof(Vertex), vertexData, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(positionAttribute);
	glEnableVertexAttribArray(colorAttribute);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);
	glVertexAttribPointer(colorAttribute  , 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)sizeof(vec2));

	glGenVertexArrays(1, &vertexArrayObject2);
	glBindVertexArray(vertexArrayObject2);
	GLuint  vertexBuffer2;
	glGenBuffers(1, &vertexBuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer2);
	glBufferData(GL_ARRAY_BUFFER, NUMBER_OF_VERTICES * sizeof(Vertex), vertexData2, GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(positionAttribute);
	glEnableVertexAttribArray(colorAttribute);
	glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);
	glVertexAttribPointer(colorAttribute  , 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)sizeof(vec2));
}

//Define the shaders and relevant parameters 
void loadShader() {
	shaderProgram = InitShader("color-shader.vert",  "color-shader.frag", "fragColor");
	colorAttribute = glGetAttribLocation(shaderProgram, "color");
	positionAttribute = glGetAttribLocation(shaderProgram, "position");
}

//Initial to  execute the program
//Initializes the Glut window.
//Continues displaying the output window.
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
	glutCreateWindow("02561-01-01");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	Angel::InitOpenGL();
		
	loadShader();
	loadBufferData();

	Angel::CheckError();
	glutMainLoop();
}