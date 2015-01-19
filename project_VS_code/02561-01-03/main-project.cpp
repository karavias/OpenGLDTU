// 02561-01-03

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>    // std::max
#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Angel.h"
#include "mat.h"
#include "TextureLoader.h"
#include "Perlin/perlin.h"
#include "ObjLoader.h"
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#define PI 3.14159265
using namespace std;
using namespace Angel;


struct Shader{
	GLuint shaderProgram;
	GLuint projectionUniform,
		modelViewUniform,
		cubemapUniform,
		textureUniform,
		cameraPosUniform;
	GLuint positionAttribute;
	GLuint textCoordAttribute;
	GLuint texture;

};
struct MeshObject {
	Shader shader;
	GLuint vertexArrayObject;
	vector<int> indices;
	string name;
	MeshObject(){}

	MeshObject(Shader shader, GLuint vertexArrayObject, vector<int> indices, const char* name)
		:shader(shader), vertexArrayObject(vertexArrayObject), indices(indices), name(name){
	}

};

struct DirectionalLight
{
	vec3 Color;
	float AmbientIntensity;
	vec3 Direction;
	float DiffuseIntensity;
};



MeshObject skybox;
MeshObject table;
MeshObject sun;

int WINDOW_WIDTH = 800;
int WINDOW_HEIGHT = 600;
GLuint cubemapTexture, cubemapUniform;

struct MapShader {
	GLuint shader;
	GLuint projectionUniform;
	GLuint modelViewUniform;
	GLuint heightUniform;
	GLuint ambientProductUniform;
	GLuint diffuseProductUniform;
	GLuint specularProductUniform;
	GLuint lightPositionUniform;
	GLuint pointUniform;
	GLuint seaLevelUniform;
	GLuint mirrorUniform;
	GLuint textureUniform;

	GLuint colorAttribute;
	GLuint positionAttribute;
	GLuint normalAttribute;

	GLuint shapeVertexArrayBuffer;
	GLuint vertexBuffer;
	GLuint bumpMap;
};

MapShader landShader;
MapShader seaShader;

const GLuint rectangleSize = 4;

struct Vertex {
	vec3 position;
	vec3 color;
	vec3 normal;
	vec2 uv;
};

struct DirectionAttr {
	float theta;
	float fi;
	float r;

	vec3 getDirection(){
		float a = r * cos(fi);
		vec3 cart;
		cart.x = a * cos(theta);
		cart.y = r * sin(fi);
		cart.z = a * sin(theta);
		return cart;

	}
	vec3 getDirectionLeft() {
		float a = r * cos(0);
		vec3 cart;
		cart.x = a * cos(theta - (PI / 2));
		cart.y = r * sin(0);
		cart.z = a * sin(theta - (PI / 2));
		return cart;

	}

	vec3 getDirectionUp() {
		float a = r * cos(fi + (PI / 2));
		vec3 cart;
		cart.x = a * cos(theta);
		cart.y = r * sin(fi + (PI / 2));
		cart.z = a * sin(theta);
		return cart;
	}
};
struct PerlinAttrs {
	int octav;
	int frequency;
	int ampl;
	int seed;
};
const int SURFACE_WIDTH = 500;
const int SURFACE_HEIGHT = 500;
const float SURFACE_POINT_DISTANCE = 1;
const GLuint vSize = SURFACE_WIDTH * SURFACE_HEIGHT;
const GLuint iSize = (SURFACE_WIDTH*SURFACE_HEIGHT) + (SURFACE_WIDTH - 1)*(SURFACE_HEIGHT - 2);



vec3 eyePoint(SURFACE_WIDTH*SURFACE_POINT_DISTANCE / 2, 5, SURFACE_WIDTH*SURFACE_POINT_DISTANCE / 2);
void loadShader(MapShader * shaderProgram);
void display();
GLuint loadBufferData(MapShader ms, Vertex* vertices, int vertexCount);
void reshape(int W, int H);
void loadGeometry();
int* getIndices();
void UpdateWindowTitle();
void drawMeshObject(mat4 & projection, mat4 & modelView, MeshObject& meshObject);
void drawMapObject(MapShader & ms, mat4 & projection, mat4 & modelView);
void drawMirror(mat4 projection, mat4 view);
void drawWater(mat4 projection, mat4 view);
bool wPressed = false;
bool aPressed = false;
bool sPressed = false;
bool dPressed = false;
bool qPressed = false;
bool ePressed = false;
bool mouseLocked = false;
float rotateZ;
float rotateY;
float rotateX;
float terrainY = 0;
vec2 mousePos;
float sphericalPolarAngle = 0;
DirectionAttr direction;
vec2 mouseLockPosition(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
int* indices = new int[iSize];
Vertex* vertices = new Vertex[vSize];
PerlinAttrs perlinAttrs;
float whatever = 0.0f;
clock_t frameTime;
bool makeNewTerrain = false;
float seaLevel;
vec4 light_position(SURFACE_WIDTH * SURFACE_POINT_DISTANCE, 300, SURFACE_HEIGHT * SURFACE_POINT_DISTANCE, 0);
vec4 sunMoveDestination(SURFACE_WIDTH * SURFACE_POINT_DISTANCE, 300, SURFACE_HEIGHT * SURFACE_POINT_DISTANCE, 0);

//the display function.
void display() {
	glClearColor(1, 1, 1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


	mat4 projection =
		Perspective(45, WINDOW_WIDTH / float(WINDOW_HEIGHT), 0.5, 10000);
	mat4 modelView = LookAt(eyePoint, eyePoint + direction.getDirection(), vec3(0, 1, 0));

	drawMapObject(landShader, projection, modelView);
	drawWater(projection, modelView);



	mat4 tableView = modelView*Translate(SURFACE_WIDTH * SURFACE_POINT_DISTANCE / 2, -571, SURFACE_HEIGHT * SURFACE_POINT_DISTANCE / 2);

	//drawMeshObject(projection, tableView, table);
	mat4 sunView = modelView * Translate(light_position);// *Scale(30, 30, 30);

	drawMeshObject(projection, sunView, sun);
	mat4 skyBoxView = modelView * Translate(SURFACE_WIDTH * SURFACE_POINT_DISTANCE / 2, 0, SURFACE_HEIGHT * SURFACE_POINT_DISTANCE / 2)*Scale(30, 30, 30);
	drawMeshObject(projection, skyBoxView, skybox);


	glutSwapBuffers();

	Angel::CheckError();

}

//draw water, initializes water as stencil buffer,
//draws the mirrored objects and then it draws the normal water.
void drawWater(mat4 projection, mat4 view) {
	mat4 model;
	glClearStencil(0);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NEVER, 0x0, 0x0);			 //Test always success, value written 1
	glColorMask(false, false, false, false); //Disable writting in color buffer
	glStencilOp(GL_INCR, GL_INCR, GL_INCR); //Stencil & Depth test passes => replace existing value in stencil buffer
	drawMapObject(seaShader, projection, view);
	glColorMask(true, true, true, true);    //Enable writting in color buffer
	glStencilFunc(GL_EQUAL, 0x1, 0x1);          //Draw only to color buffer where stencil buffer is 1
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Stencil buffer read only

	drawMirror(projection, view);

	glDisable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);

	drawMapObject(seaShader, projection, view);
	glDisable(GL_BLEND);

}

//draws the mirrored objects that are projected in the stencil buffer.
void drawMirror(mat4 projection, mat4 view) {
	mat4 model = view * Translate(0, 2 * seaLevel, 0)* Scale(1, -1, 1);
	drawMapObject(landShader, projection, model);
	mat4 sunView = view * Translate(SURFACE_WIDTH * SURFACE_POINT_DISTANCE, 300, SURFACE_HEIGHT * SURFACE_POINT_DISTANCE)*Scale(1, -1, 1);
	drawMeshObject(projection, sunView, sun);
	mat4 skyBoxView = view * Translate(SURFACE_WIDTH * SURFACE_POINT_DISTANCE / 2, 0, SURFACE_HEIGHT * SURFACE_POINT_DISTANCE / 2)*Scale(30, -30, 30);
	drawMeshObject(projection, skyBoxView, skybox);

}

//this is used to draw the terrain.
void drawMapObject(MapShader& ms, mat4 & projection, mat4 & modelView){
	glUseProgram(ms.shader);
	if (ms.projectionUniform != GL_INVALID_INDEX) {
		glUniformMatrix4fv(ms.projectionUniform, 1, GL_TRUE, projection);
	}
	else {
		cout << "Invalid uniform??" << endl;
		return;
	}
	glBindVertexArray(ms.shapeVertexArrayBuffer);

	glUniformMatrix4fv(ms.modelViewUniform, 1, GL_TRUE, modelView);


	glUniform1f(ms.heightUniform, terrainY);
	//vec4 light_position(0.0, 0.0, -1.0, 0);

	vec4 light_ambient(0.2, 0.2, 0.2, 1.0);
	vec4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	vec4 light_specular(1.0, 1.0, 1.0, 1.0);

	vec4 material_ambient(1.0, 0.0, 1.0, 1.0);
	vec4 material_diffuse(1.0, 0.8, 0.0, 1.0);
	vec4 material_specular(1.0, 0.8, 0.0, 1.0);


	vec4 ambient_product = light_ambient * material_ambient;
	vec4 diffuse_product = light_diffuse * material_diffuse;
	vec4 specular_product = light_specular * material_specular;
	vec4 centerOfTheWorld = vec4(SURFACE_WIDTH * SURFACE_POINT_DISTANCE, 0, SURFACE_HEIGHT * SURFACE_POINT_DISTANCE, 0);


	glUniform4fv(glGetUniformLocation(ms.shader, "AmbientProduct"),
		1, ambient_product);
	glUniform4fv(glGetUniformLocation(ms.shader, "DiffuseProduct"),
		1, diffuse_product);
	glUniform4fv(glGetUniformLocation(ms.shader, "SpecularProduct"),
		1, specular_product);
	glUniform4fv(glGetUniformLocation(ms.shader, "centerOfTheWorld"),
		1, centerOfTheWorld);
	glUniform4fv(glGetUniformLocation(ms.shader, "LightPosition"),
		1, light_position);
	glUniform1f(glGetUniformLocation(ms.shader, "point"),
		0);
	glUniform1f(glGetUniformLocation(ms.shader, "seaLevel"),
		seaLevel);

	glDrawElements(GL_TRIANGLE_STRIP, iSize, GL_UNSIGNED_INT, indices);

}


//this is used to draw the sun, the table and the skybox.
void drawMeshObject(mat4 & projection, mat4 & modelView, MeshObject& meshObject) {
	glUseProgram(meshObject.shader.shaderProgram);
	if (meshObject.shader.projectionUniform != GL_INVALID_INDEX){
		glUniformMatrix4fv(meshObject.shader.projectionUniform, 1, GL_TRUE, projection);
	}
	if (meshObject.shader.modelViewUniform != GL_INVALID_INDEX){
		glUniformMatrix4fv(meshObject.shader.modelViewUniform, 1, GL_TRUE, modelView);
	}
	if (meshObject.shader.cameraPosUniform != GL_INVALID_INDEX){
		glUniform3fv(meshObject.shader.cameraPosUniform, 1, eyePoint);
	}
	if (meshObject.shader.cubemapUniform != GL_INVALID_INDEX){
		// bind cubemap to texture slot 0 and set the uniform value
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glUniform1i(meshObject.shader.cubemapUniform, 0);
	}
	if (meshObject.shader.textureUniform != GL_INVALID_INDEX){
		// bind texture to texture slot 0 and set the uniform value
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, meshObject.shader.texture);
		glUniform1i(meshObject.shader.textureUniform, 0);
	}

	glBindVertexArray(meshObject.vertexArrayObject);
	glDrawElements(GL_TRIANGLES, meshObject.indices.size(), GL_UNSIGNED_INT, &meshObject.indices[0]);
}


//calculates the normals for the generated map.
void CalcNormals(int* pIndices, unsigned int IndexCount, Vertex* pVertices, unsigned int VertexCount)
{

	for (unsigned int i = 0; i < IndexCount; i++) {
		if (i + 2 >= IndexCount) {
			break;
		}
		unsigned int Index0 = pIndices[i];
		unsigned int Index1 = pIndices[i + 1];
		unsigned int Index2 = pIndices[i + 2];
		vec3 v1 = pVertices[Index1].position - pVertices[Index0].position;
		vec3 v2 = pVertices[Index2].position - pVertices[Index0].position;
		vec3 Normal = normalize(cross(v1, v2));// v1.Cross(v2);

		pVertices[Index0].normal += Normal;
		pVertices[Index1].normal += Normal;
		pVertices[Index2].normal += Normal;
	}

	for (unsigned int i = 0; i < VertexCount; i++) {
		int x = i % SURFACE_WIDTH;
		int y = i / SURFACE_WIDTH;

		if (y % 2 == 1) {
			pVertices[i].normal = -normalize(pVertices[i].normal);
		}
		else {
			pVertices[i].normal = normalize(pVertices[i].normal);

		}
	}
}


//calculates the indices to generate the triangle strip for the
//map surface correctly.
int* getIndices() {
	int height = SURFACE_HEIGHT;
	int width = SURFACE_WIDTH;

	int i = 0;

	for (int row = 0; row<height - 1; row++) {
		if ((row & 1) == 0) { // even rows
			for (int col = 0; col<width; col++) {
				indices[i++] = col + row * width;
				indices[i++] = col + (row + 1) * width;
			}
		}
		else { // odd rows
			for (int col = width - 1; col>0; col--) {
				indices[i++] = col + (row + 1) * width;
				indices[i++] = col - 1 + +row * width;
			}
		}
	}
	if ((height & 1) && height>2) {
		indices[i++] = (height - 1) * height;
	}


	return indices;
}

//calculates the vertices for the surface.
//the vertices are calculated in fixed x and y positions and a height that
//is calculated using perlin noise.
void GenerateVertices() {
	vec3 blue(0, 0, 1);
	vec3 sand(238.0 / 255.0, 214.0 / 255.0, 175.0 / 255.0);
	vec3 sandStart(173, 173, 153);
	vec3 sandEnd(105, 102, 65);
	vec3 mountainEnd(49, 66, 21);
	vec3 green(0, 1, 0);
	vec3 white(1, 1, 1);
	float ampl = perlinAttrs.ampl;

	Perlin *p = new Perlin(perlinAttrs.octav, perlinAttrs.frequency, ampl, perlinAttrs.seed);
	float sandPoint = ampl / 25.0f;
	seaLevel = -sandPoint;
	float mountainPoint = ampl / 3.0f;
	float maxHeight = 0;
	for (int indexY = 0; indexY< SURFACE_HEIGHT; indexY++) {
		for (int indexX = 0; indexX < SURFACE_WIDTH; indexX++) {
			float height = p->Get((indexX * SURFACE_POINT_DISTANCE) / (SURFACE_WIDTH * SURFACE_POINT_DISTANCE),
				(indexY * SURFACE_POINT_DISTANCE) / (SURFACE_HEIGHT * SURFACE_POINT_DISTANCE));
			vec3 color(0, 0, 0);
			if (height > maxHeight) {
				maxHeight = height;
			}
			if (height < -sandPoint) {
				color.z = 238.0 / 255.0;
				color.y = (200.0 - (max(1.0f, (float)((float)abs(height) / (ampl / 2.0f)) * 160.0f))) / 255.0;
			}
			else if (height > sandPoint && height < (mountainPoint - mountainPoint / 40.0f)) {
				color = min(1.0f, (abs(height - ampl) / mountainPoint)) * sandEnd / 255.0f + min(1.0f, (abs(height - sandPoint) / mountainPoint)) * mountainEnd / 255.0f
					+ vec3(static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.05f)),
					static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.05f)),
					static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 0.05f)));

			}
			else if (height > sandPoint) {
				color = white;
			}
			else {
				color = (abs(height - sandPoint) / (2 * sandPoint)) * sandStart / 255.0f + (abs(height + sandPoint) / (2 * sandPoint)) * sandEnd / 255.0f;

			}
			vertices[indexX + (SURFACE_WIDTH)* indexY].position = vec3(indexX * SURFACE_POINT_DISTANCE, height,
				indexY * SURFACE_POINT_DISTANCE);
			vertices[indexX + (SURFACE_WIDTH)* indexY].color = color;

		}
	}
}

//loads the map geometry.
void loadGeometry() {
	terrainY = 100;
	GenerateVertices();
	getIndices();
	CalcNormals(indices, iSize, vertices, vSize);
	landShader.shapeVertexArrayBuffer = loadBufferData(landShader, vertices, vSize);
	seaShader.shapeVertexArrayBuffer = loadBufferData(seaShader, vertices, vSize);

}

//loads the buffer data for the map elements.
GLuint loadBufferData(MapShader ms, Vertex* vertices, int vertexCount) {
	GLuint vertexArrayObject;
	glEnable(GL_DEPTH_TEST);
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
	glGenBuffers(1, &ms.vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, ms.vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(ms.positionAttribute);
	glEnableVertexAttribArray(ms.colorAttribute);
	glEnableVertexAttribArray(ms.normalAttribute);


	glVertexAttribPointer(ms.positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);
	glVertexAttribPointer(ms.colorAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)sizeof(vec3));
	glVertexAttribPointer(ms.normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)(2 * sizeof(vec3)));

	return vertexArrayObject;
}

//initializes the shaders for the map elements. The land and the sea.
void LoadMapShader() {
	landShader.shader = InitShader("landShader.vert", "landShader.frag", "fragColor");
	seaShader.shader = InitShader("waterShader.vert", "waterShader.frag", "fragColor");
	loadShader(&landShader);
	loadShader(&seaShader);
}

//loads a shader for a map object.
void loadShader(MapShader * sp){

	sp->projectionUniform = glGetUniformLocation(sp->shader, "projection");
	if (sp->projectionUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain the 'projection' uniform." << endl;
	}
	sp->modelViewUniform = glGetUniformLocation(sp->shader, "modelView");
	if (sp->modelViewUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain the 'modelView' uniform." << endl;
	}
	sp->heightUniform = glGetUniformLocation(sp->shader, "height");
	if (sp->heightUniform == GL_INVALID_INDEX) {
		cerr << "Shader did not contain the 'height' attribute." << endl;
	}
	sp->colorAttribute = glGetAttribLocation(sp->shader, "color");
	if (sp->colorAttribute == GL_INVALID_INDEX) {
		cerr << "Shader did not contain the 'color' attribute." << endl;
	}
	sp->positionAttribute = glGetAttribLocation(sp->shader, "position");
	if (sp->positionAttribute == GL_INVALID_INDEX) {
		cerr << "Shader did not contain the 'position' attribute." << endl;
	}
	sp->normalAttribute = glGetAttribLocation(sp->shader, "normal");
	if (sp->normalAttribute == GL_INVALID_INDEX) {
		cerr << "Shader did not contain the 'normal' attribute." << endl;
	}

}

//resize callback function
void reshape(int W, int H) {
	WINDOW_WIDTH = W;
	WINDOW_HEIGHT = H;
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
}

//handle keyboard pressed events.
void keyboard(unsigned char c, int x, int y){
	switch (c){
	case 'a':
	case 'A':
		aPressed = true;
		break;
	case 'w':
	case 'W':
		wPressed = true;
		break;
	case 'd':
	case 'D':
		dPressed = true;
		break;
	case 's':
	case 'S':
		sPressed = true;
		break;
	case 'q':
	case 'Q':
		qPressed = true;
		break;
	case 'e':
	case 'E':
		ePressed = true;
		break;
	case '+':
		perlinAttrs.seed++;
		makeNewTerrain = true;
		break;
	case '-':
		perlinAttrs.seed--;
		makeNewTerrain = true;
		break;
	case '1':
		perlinAttrs.ampl--;
		makeNewTerrain = true;
		break;
	case '4':
		perlinAttrs.ampl++;
		makeNewTerrain = true;
		break;
	case '2':
		perlinAttrs.frequency--;
		makeNewTerrain = true;
		break;
	case '5':
		perlinAttrs.frequency++;
		makeNewTerrain = true;
		break;
	case '3':
		perlinAttrs.octav--;
		makeNewTerrain = true;
		break;
	case '6':
		perlinAttrs.octav++;
		makeNewTerrain = true;
		break;
	case 'z':
	case 'Z':
		whatever--;
		cout << "whatever? " << whatever << endl;
		break;
	case 'x':
	case 'X':
		whatever++;
		cout << "whatever? " << whatever << endl;
		break;
	}
	UpdateWindowTitle();
}

//handle keyboard released events.
void keyboardUp(unsigned char c, int x, int y){

	switch (c){
	case 'a':
	case 'A':
		aPressed = false;
		break;
	case 'w':
	case 'W':
		wPressed = false;
		break;
	case 'd':
	case 'D':
		dPressed = false;
		break;
	case 's':
	case 'S':
		sPressed = false;
		break;
	case 'q':
	case 'Q':
		qPressed = false;
		break;
	case 'e':
	case 'E':
		ePressed = false;
		break;
	case 'c':
	case 'C':
		if (sunMoveDestination.x > 0) {
			sunMoveDestination.x = 0;
			sunMoveDestination.z = 0;
		} else {
			sunMoveDestination.x = SURFACE_WIDTH * SURFACE_POINT_DISTANCE;
			sunMoveDestination.z = SURFACE_HEIGHT * SURFACE_POINT_DISTANCE;
		}
	}
	UpdateWindowTitle();
}

void UpdateWindowTitle() {
	std::stringstream title;
	title << "Project - s141278 : [Perlin: " << 
		"[- ampl: " << perlinAttrs.ampl<<
		"[- freq: " << perlinAttrs.frequency <<
		"[- octav: " << perlinAttrs.octav <<
		"[- seed: " << perlinAttrs.seed << "]]";
	string result = title.str();
	glutSetWindowTitle(&result[0]);
}


//this function is the main loop of the application
//that handles the input and renders the new display.
void timer(int v) {
	float diff = (((float)clock() - (float)frameTime) / 1000000.0F) * 1000;
	frameTime = clock();
	float speed = 50.0f;
	if (wPressed) {
		eyePoint += direction.getDirection() * speed * diff;
	}
	if (sPressed) {
		eyePoint -= direction.getDirection() * speed * diff;
	}
	if (aPressed) {
		eyePoint += direction.getDirectionLeft() * speed * diff;
	}
	if (dPressed) {
		eyePoint -= direction.getDirectionLeft() * speed * diff;
	}
	if (qPressed) {
		eyePoint += direction.getDirectionUp() * speed * diff;
	}
	if (ePressed) {
		eyePoint -= direction.getDirectionUp() * speed * diff;
	}
	if (makeNewTerrain) {
		if (terrainY < -2) {
			terrainY = -2;
		}
		terrainY += 2.0f;
		if (terrainY > 100) {
			makeNewTerrain = false;
			loadGeometry();
		}
	}
	else {
		terrainY -= 2.0f;
	}
	if (length(sunMoveDestination - light_position) > 1) {
		vec4 moveDir = normalize(sunMoveDestination - light_position);
		light_position += moveDir * diff * speed;
	}

	glutPostRedisplay();
	
	glutTimerFunc(10, timer, 0);

}


GLuint createVertexBuffer(Vertex* vertices, int vertexCount) {
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	return vertexBuffer;
}

//loads a mesh .obj file.
GLuint loadMesh(char *filename, vector<int> &indices, float scale = 1.0){
	vector<vec3> position;
	vector<vec3> normal; // unused
	vector<vec2> uv; // unused
	loadObject(filename, position, indices, normal, uv, scale);
	Vertex* vertexData = new Vertex[position.size()];

	for (int i = 0; i < position.size(); i++) {
		vertexData[i].position = position[i];
		vertexData[i].uv = uv[i];
	}
	GLuint vertexBuffer = createVertexBuffer(vertexData, position.size());
	delete[] vertexData;
	return vertexBuffer;
}

//creates array object with the data for the vertices.
GLuint createVertexArrayObject(GLuint vertexBuffer, const Shader & shader, bool includeUV = false){
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	GLuint vertexArrayObject;

	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	glEnableVertexAttribArray(shader.positionAttribute);
	glVertexAttribPointer(shader.positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)0);
	if (includeUV) {
		glEnableVertexAttribArray(shader.textCoordAttribute);
		glVertexAttribPointer(shader.textCoordAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid *)3);

	}

	return vertexArrayObject;
}

//function that loads the shader vertex and fragment with some default attributes/uniforms.
Shader loadShader(const char* vertShader, const char* fragShader){
	Shader shader;
	shader.shaderProgram = InitShader(vertShader, fragShader, "fragColor");
	// get uniform locations
	shader.projectionUniform = glGetUniformLocation(shader.shaderProgram, "projection");
	shader.modelViewUniform = glGetUniformLocation(shader.shaderProgram, "modelView");
	shader.cubemapUniform = glGetUniformLocation(shader.shaderProgram, "cubemap");
	shader.textureUniform = glGetUniformLocation(shader.shaderProgram, "textureMap");
	shader.cameraPosUniform = glGetUniformLocation(shader.shaderProgram, "cameraPos");
	
	// get attribute locations
	shader.positionAttribute = glGetAttribLocation(shader.shaderProgram, "position");
	shader.textCoordAttribute = glGetAttribLocation(shader.shaderProgram, "textureCoord");
	return shader;
}

//initialize the skybox mesh
void initSkyboxMeshAndShader(){
	Shader skyboxShader = loadShader("skybox.vert", "skybox.frag");
	vector<int> skyboxIndices;

	GLuint cubeVertexBuffer = loadMesh("meshes/cube.obj", skyboxIndices, 50.0f);
	GLuint skyboxVertexArrayObject = createVertexArrayObject(cubeVertexBuffer, skyboxShader);
	skybox = MeshObject(
		skyboxShader,
		skyboxVertexArrayObject,
		skyboxIndices,
		"Skybox"
		);
}

//initialize the sun mesh
void InitSunMeshAndShader() {
	Shader tableShader = loadShader("sunShader.vert", "sunShader.frag");
	vector<int> tableIndices;

	GLuint tableVertexBuffer = loadMesh("meshes/sphere.obj", tableIndices, 20.0f);
	GLuint tableVertexArrayObject = createVertexArrayObject(tableVertexBuffer, tableShader);
	sun = MeshObject(
		tableShader,
		tableVertexArrayObject,
		tableIndices,
		"Sun"
		);
}

//initialize the table mesh
void InitTableMeshAndShader() {
	Shader tableShader = loadShader("tableShader.vert", "tableShader.frag");
	vector<int> tableIndices;

	GLuint tableVertexBuffer = loadMesh("meshes/table.obj", tableIndices, 20.0f);
	GLuint tableVertexArrayObject = createVertexArrayObject(tableVertexBuffer, tableShader, true);
	table = MeshObject(
		tableShader,
		tableVertexArrayObject,
		tableIndices,
		"Table"
		);
}


//initialize the cube map for the skybox
void initCubemapTexture() {

	const char* cube[] = { "textures/sky_left.bmp",
		"textures/sky_right.bmp",
		"textures/sky_top.bmp",
		"textures/sky_bottom.bmp",
		"textures/sky_back.bmp",
		"textures/sky_front.bmp" };

	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	for (int i = 0; i<6; ++i)
	{
		unsigned int width, height;
		void* data = loadBMPRaw(cube[i], width, height);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + i,
			0, //level
			GL_RGB8, //internal format
			width, //width
			height, //height
			0, //border
			GL_BGR, //format or GL_BGR for BMP
			GL_UNSIGNED_BYTE, //type
			data); //pixel data
		delete[] data;
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

}


//handle mouse click event.
void mouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		mouseLocked = !mouseLocked;
		mousePos.x = x;
		mousePos.y = y;
	}
}

//handle mouse motion event.
void mouseMovement(int x, int y){

	vec2 mouseDirection = mousePos - vec2(x, y);
	mousePos.x = x;
	mousePos.y = y;
	float speed = 0.002f;
	float rotate89Degrees = 89 * DegreesToRadians;

	direction.fi = min(rotate89Degrees, max(-rotate89Degrees, direction.fi + mouseDirection.y*speed));
	direction.theta -= mouseDirection.x*speed;

}

//initialize basic attributes for camera and map
void InitAttrs() {
	direction.theta = 0.012f;
	direction.fi = -0.528f;
	direction.r = 1;
	eyePoint.x = -263.409f;
	eyePoint.y = 284.321f;
	eyePoint.z = 239.632f;
	perlinAttrs.ampl = 60;
	perlinAttrs.octav = 8;
	perlinAttrs.frequency = 6;
	perlinAttrs.seed = 94;
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

void printHelp() {
	cout << "********************************************" << endl;
	cout << "*******Project For Computer Graphics********" << endl;
	cout << "***Dimitrios Emmanouil Karavias - s141278***" << endl;
	cout << "********************************************" << endl;
	cout << "_____________________Help___________________" << endl;
	cout << "Input:" << endl;
	cout << "Keys w,a,s,d : Move camera forward,left,backward and right on the direction of the view" << endl;
	cout << "Keys q,e : Move camera up and down on the direction of the view" << endl;
	cout << "Keys 1,4 : Decrease/increase perlin amplitude. A new map will be generated" << endl;
	cout << "Keys 2,5 : Decrease/increase perlin frequency. A new map will be generated" << endl;
	cout << "Keys 3,6 : Decrease/increase perlin octaves. A new map will be generated" << endl;
	cout << "Keys -,+ : Decrease/increase perlin seed. A new map will be generated" << endl;
	cout << "--------------------------------------------" << endl;


}

//the main function.
int main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutSetOption(
		GLUT_ACTION_ON_WINDOW_CLOSE,
		GLUT_ACTION_GLUTMAINLOOP_RETURNS
		);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_3_2_CORE_PROFILE);
	glutCreateWindow("Project - s141278");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutReshapeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutTimerFunc(100, timer, 0);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMovement);
	printHelp();
	InitAttrs();
	UpdateWindowTitle();
	initCubemapTexture();
	
	Angel::InitOpenGL();
	LoadMapShader();

	initSkyboxMeshAndShader();
	InitTableMeshAndShader();
	InitSunMeshAndShader();
	loadGeometry();


	Angel::CheckError();

	glutMainLoop();
}