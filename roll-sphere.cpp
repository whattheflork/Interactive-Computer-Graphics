/*	Matt Florkiewicz
	Interactive Computer Graphics
	Professor Yi-Jen Chiang
	December 22, 2015
	Fall 2015 
	Assignment # 4		*/


#include "Angel-yjc.h"
//#include "texmap.c"
#include <iostream>
#include <fstream>
#include <string>

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

#define ImageWidth  32
#define ImageHeight 32
GLubyte Image[ImageHeight][ImageWidth][4];

#define	stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];

GLuint program;       /* shader program object id */
GLuint regularProgram; /* shader program for unlit scene */
GLuint fireworkProgram;
GLuint cube_buffer;   /* vertex buffer object id for cube */
GLuint floor_buffer;
GLuint shadow_buffer;
GLuint axes_buffer;
GLuint firework_buffer;

//mat4 rotation = identity();

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 25.0;

vec4 original_eye(7.0, 3.0, -10.0, 1.0);
vec4 eye = original_eye;
vec4 at(0.0, 0.0, 0.0, 1.0);
vec4 up(0.0, 1.0, 0.0, 0.0);

int beginFlag = 0;
int animationFlag = 1; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int wireFlag = 1;
GLfloat lightingFlag = 1.0;
GLfloat smoothFlag = 0.0;
int shadowFlag = 1;
GLfloat spotFlag = 0.0;
int groundFlag = 1;
int blendFlag = 1;

GLfloat fogFlag = 0.0;
GLfloat linearFlag = 0.0;
GLfloat expFlag = 0.0;
GLfloat squareFlag = 0.0;

GLfloat verticalFlag = 1.0;
GLfloat slantedFlag = 0.0;

GLfloat objFrame = 1.0;
GLfloat eyeFrame = 0.0;

GLfloat sphereTexFlag = 1.0;
GLfloat conFlag = 0.0;
GLfloat checkFlag = 1.0;

GLfloat latticeFlag = 0.0;
GLfloat upFlag = 0.0;
GLfloat tiltFlag = 1.0;

int fireworkFlag = 1.0;
int timeMax = 3500;
int time = 0;
int timeInitial = 0;

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
point4 points[NumVertices];
vec3   normals[NumVertices];
point4 sphere_points[10000];
vec3 sphere_normals[10000];
color4 sphere_colors[10000];
int sphere_NumVertices;
vec2 sphere_texCoords[10000];

color4 shadow_colors[10000];

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec3 floor_normals[floor_NumVertices];
vec2 floor_texCoords[floor_NumVertices];

const int axes_NumVertices = 6;
point4 axes[axes_NumVertices];
color4 axesColors[axes_NumVertices];
vec3 axesNormals[axes_NumVertices];

color4 fireworkColors[300];
point4 fireworkVelocities[300];


// Array of rotation angles (in degrees) for each coordinate axis
float angle = 0;
enum { Xaxis = 8, Yaxis = 0, Zaxis = 3, NumAxes = 3 };
int      Axis = Xaxis;
GLfloat  Theta[NumAxes] = { angle, 0, angle };
bool AB = 1;
bool BC = 0;
bool CA = 0;
float magnitude = length(point4(-4, 1, 4, 1) - point4(-1, 1, -4, 1));


// Model-view and projection matrices uniform location
GLuint  ModelView, Projection, Lighting, Smooth, Spot;
GLuint Fog, Linear, Exp, Square;

GLuint floorTex;
GLuint stripeTex;

/*----- Shader Lighting Parameters -----*/
color4 global_ambient = { 1.0, 1.0, 1.0, 1.0 };

color4 light_ambient(0.0, 0.0, 0.0, 1.0);
color4 light_diffuse(0.8, 0.8, 0.8, 0.8);
color4 light_specular(0.2, 0.2, 0.2, 1.0);
float const_att = 1.0;
float linear_att = 0.00;
float quad_att = 0.00;

struct positional_light {
	point4 position = { -14.0, 12.0, -3.0, 1.0 };
	color4 diffuse = { 1.0, 1.0, 1.0, 1.0 };
	color4 specular = { 1.0, 1.0, 1.0, 1.0 };
	color4 ambient = { 0.0, 0.0, 0.0, 1.0 };
	float const_att = 2.0;
	float linear_att = 0.01;
	float quad_att = 0.001;
};

positional_light light;


point4 distant_light(1.0, 0.0, -1.0, 0.0);

color4 ambient_product;
color4 diffuse_product;
color4 specular_product;
// In World frame.
// Needs to transform it to Eye Frame
// before sending it to the shader(s).

color4 sphere_ambient(0.2, 0.2, 0.2, 1.0);
color4 sphere_diffuse(1.0, 0.84, 0.0, 1.0);
color4 sphere_specular(1.0, 0.84, 0.0, 1.0);
float  sphere_shininess = 125.0;

color4 floor_ambient(0.2, 0.2, 0.2, 1.0);
color4 floor_diffuse(0.0, 1.0, 0.0, 1.0);
color4 floor_specular(0.0, 0.0, 0.0, 1.0);
float  floor_shininess = 1.0;



void SetUp_Lighting_Uniform_Vars(mat4 mv);

int Index = 0;

// RGBA colors
color4 vertex_colors[8] = {
	color4(0.0, 0.0, 0.0, 1.0),  // black
	color4(1.0, 0.0, 0.0, 1.0),  // red
	color4(1.0, 1.0, 0.0, 1.0),  // yellow
	color4(0.0, 1.0, 0.0, 1.0),  // green
	color4(0.0, 0.0, 1.0, 1.0),  // blue
	color4(1.0, 0.0, 1.0, 1.0),  // magenta
	color4(1.0, 1.0, 1.0, 1.0),  // white
	color4(0.0, 1.0, 1.0, 1.0)   // cyan
};

void image_set_up(void)
{
	int i, j, c;

	/* --- Generate checkerboard image to the image array ---*/
	for (i = 0; i < ImageHeight; i++)
		for (j = 0; j < ImageWidth; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

			if (c == 1) /* white */
			{
				c = 255;
				Image[i][j][0] = (GLubyte)c;
				Image[i][j][1] = (GLubyte)c;
				Image[i][j][2] = (GLubyte)c;
			}
			else  /* green */
			{
				Image[i][j][0] = (GLubyte)0;
				Image[i][j][1] = (GLubyte)150;
				Image[i][j][2] = (GLubyte)0;
			}

			Image[i][j][3] = (GLubyte)255;
		}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*--- Generate 1D stripe image to array stripeImage[] ---*/
	for (j = 0; j < stripeImageWidth; j++) {
		/* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
		When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
		*/
		stripeImage[4 * j] = (GLubyte)255;
		stripeImage[4 * j + 1] = (GLubyte)((j>4) ? 255 : 0);
		stripeImage[4 * j + 2] = (GLubyte)0;
		stripeImage[4 * j + 3] = (GLubyte)255;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/*----------- End 1D stripe image ----------------*/

	/*--- texture mapping set-up is to be done in
	init() (set up texture objects),
	display() (activate the texture object to be used, etc.)
	and in shaders.
	---*/

} /* end function */

void floor()
{
	floor_colors[0] = vertex_colors[3]; floor_points[0] = point4(5, 0, 8, 1); floor_texCoords[0] = vec2(6.0, 5.0);
	floor_colors[1] = vertex_colors[3]; floor_points[1] = point4(5, 0, -4, 1); floor_texCoords[1] = vec2(0.0, 5.0);
	floor_colors[2] = vertex_colors[3]; floor_points[2] = point4(-5, 0, -4, 1); floor_texCoords[2] = vec2(0.0, 0.0);

	floor_colors[3] = vertex_colors[3]; floor_points[3] = point4(-5, 0, -4, 1); floor_texCoords[3] = vec2(0.0, 0.0);
	floor_colors[4] = vertex_colors[3]; floor_points[4] = point4(5, 0, 8, 1); floor_texCoords[4] = vec2(6.0, 5.0);
	floor_colors[5] = vertex_colors[3]; floor_points[5] = point4(-5, 0, 8, 1); floor_texCoords[5] = vec2(6.0, 0.0);

	vec4 u = floor_points[1] - floor_points[0];
	vec4 v = floor_points[3] - floor_points[0];
	vec3 normal = normalize(cross(u, v));
	for (int i = 0; i < 6; i++) {
		floor_normals[i] = normalize(vec3(floor_points[i].x, floor_points[i].y, floor_points[i].z));
	}

}

void fillAxes() {
	axes[0] = point4(0, 0, 0, 1);	axesColors[0] = vertex_colors[1]; axesNormals[0] = normalize(vec3(axes[1].x, axes[1].y, axes[1].z));
	axes[1] = point4(10, 0, 0, 1);	axesColors[1] = vertex_colors[1]; axesNormals[1] = normalize(vec3(axes[1].x, axes[1].y, axes[1].z));

	axes[2] = point4(0, 0, 0, 1);	axesColors[2] = vertex_colors[5]; axesNormals[2] = normalize(vec3(axes[3].x, axes[3].y, axes[3].z));
	axes[3] = point4(0, 10, 0, 1);	axesColors[3] = vertex_colors[5]; axesNormals[3] = normalize(vec3(axes[3].x, axes[3].y, axes[3].z));

	axes[4] = point4(0, 0, 0, 1);	axesColors[4] = vertex_colors[4]; axesNormals[4] = normalize(vec3(axes[5].x, axes[5].y, axes[5].z));
	axes[5] = point4(0, 0, 10, 1);	axesColors[5] = vertex_colors[4]; axesNormals[5] = normalize(vec3(axes[5].x, axes[5].y, axes[5].z));

}

void readFile(const std::string& fileName) {
	std::ifstream ifs(fileName);
	int size = 0;
	int index = 0;
	int normalIndex = 0;
	point4 u;
	point4 v;
	float x;
	float y;
	float z;
	float junk;
	ifs >> size;
	sphere_NumVertices = size * 3;
	while (ifs) {
		ifs >> junk >> x >> y >> z;
		sphere_points[index++] = point4(x, y, z, 1);
		ifs >> x >> y >> z;
		sphere_points[index++] = point4(x, y, z, 1);
		ifs >> x >> y >> z;
		sphere_points[index++] = point4(x, y, z, 1);
	}
	vec3 normal;
	for (size_t i = 2; i < sphere_NumVertices; i += 3) {
		u = sphere_points[i] - sphere_points[i - 1];
		v = sphere_points[i - 2] - sphere_points[i - 1];
		normal = normalize(cross(u, v));
		sphere_normals[i - 2] = normal;
		sphere_normals[i - 1] = normal;
		sphere_normals[i] = normal;
		
	}

	for (size_t i = 0; i < sphere_NumVertices; i++) {
		sphere_colors[i] = color4(1.0, 0.84, 0.0, 1.0);
	}



}

void setUpShadowColor() {
	for (size_t i = 0; i < sphere_NumVertices; i++) {
		shadow_colors[i] = color4(0.25, 0.25, 0.25, 0.65);
	}
}

void setUpShadowMatrix(mat4& shadowMatrix) {
	for (size_t i = 0; i < 4; i++) {
		for (size_t i = 0; i < 4; i++) {
			shadowMatrix[i][i] = 0.0;
		}
	}
	shadowMatrix[0][0] = 1.0; shadowMatrix[1][1] = 1.0; shadowMatrix[2][2] = 1.0;
	shadowMatrix[3][1] = -(1 / light.position.y);
}

void setFireWorkParticles() {
	for (size_t i = 0; i < 300; i++) {
		fireworkColors[i] = color4((rand() % 256) / 256.0, (rand() % 256) / 256.0, (rand() % 256) / 256.0,
			1);
		fireworkVelocities[i] = point4(2.0 * ((rand() % 256) / 256.0 - 0.5), 1.2 * 2.0 * ((rand() / 256) / 256.0),
			2.0 * ((rand() % 256) / 256.0 - 0.5), 1);
	}
}

//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
	image_set_up();
	setUpShadowColor();
	fillAxes();
	setFireWorkParticles();

	/*--- Create and Initialize a texture object ---*/
	glGenTextures(1, &stripeTex);      // Generate texture obj name(s)

	glActiveTexture(GL_TEXTURE1);  // Set the active texture unit to be 0 
	glBindTexture(GL_TEXTURE_1D, stripeTex); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,
		//0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);

	// Create and initialize a vertex buffer object
	glGenBuffers(1, &cube_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, cube_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(vec3) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices,
		NULL, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices, sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices, sizeof(color4) * sphere_NumVertices, sphere_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices, sizeof(vec3) * sphere_NumVertices, sphere_normals);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*--- Create and Initialize a texture object ---*/
	glGenTextures(1, &floorTex);      // Generate texture obj name(s)

	glActiveTexture(GL_TEXTURE0);  // Set the active texture unit to be 0 
	glBindTexture(GL_TEXTURE_2D, floorTex); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, Image);





	floor();
	// Create and initialize a vertex buffer object for floor, to be used in display()
	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_normals) + sizeof(floor_colors)
		+sizeof(floor_texCoords), floor_points, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
		floor_colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors), sizeof(floor_normals), floor_normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors) + sizeof(floor_normals),
		sizeof(floor_texCoords), floor_texCoords);



	glGenBuffers(1, &axes_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axes_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes) + sizeof(axesColors),
		axes, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axes), axes);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof(axes) + sizeof(axesColors), sizeof(axesNormals), axesNormals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axes), sizeof(axesColors), axesColors);

	glGenBuffers(1, &shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices + sizeof(color4) * sphere_NumVertices,
		NULL, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices, sizeof(vec3) * sphere_NumVertices, sphere_normals);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_NumVertices, sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_NumVertices, sizeof(color4) * sphere_NumVertices, shadow_colors);

	glGenBuffers(1, &firework_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fireworkColors) + sizeof(fireworkVelocities), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fireworkColors), fireworkColors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fireworkColors), sizeof(fireworkVelocities), fireworkVelocities);


	// Load shaders and create a shader program (to be used in display())
	program = InitShader("vshader53.glsl", "fshader53.glsl");
	regularProgram = InitShader("vshader42.glsl", "fshader42.glsl");
	fireworkProgram = InitShader("vshaderf.glsl", "fshaderf.glsl");

	glEnable(GL_DEPTH_TEST);


	glClearColor(0.529, 0.807, 0.92, 0.0);
	glLineWidth(2.0);
	glPointSize(3.0);
}

//----------------------------------------------------------------------
// SetUp_Lighting_Uniform_Vars(mat4 mv):
// Set up lighting parameters that are uniform variables in shader.
//
// Note: "LightPosition" in shader must be in the Eye Frame.
//       So we use parameter "mv", the model-view matrix, to transform
//       light_position to the Eye Frame.
//----------------------------------------------------------------------


void SetUp_Lighting_Uniform_Vars(mat4 mv, color4 material_ambient, color4 material_diffuse, color4 material_specular, GLfloat material_shininess)
{
	ambient_product = (global_ambient * material_ambient) + (light_ambient * material_ambient);
	diffuse_product = light_diffuse * material_diffuse;
	specular_product = light_specular * material_specular;
	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"),
		1, ambient_product);
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"),
		1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"),
		1, specular_product);



	glUniform1f(glGetUniformLocation(program, "ConstAtt"),
		const_att);
	glUniform1f(glGetUniformLocation(program, "LinearAtt"),
		linear_att);
	glUniform1f(glGetUniformLocation(program, "QuadAtt"),
		quad_att);

	glUniform1f(glGetUniformLocation(program, "Shininess"),
		material_shininess);
}

void setUpPosLightVars(positional_light light, color4 material_ambient, color4 material_diffuse, color4 material_specular, GLfloat material_shininess) {
	ambient_product = (global_ambient * material_ambient) + (light.ambient * material_ambient);
	diffuse_product = light.diffuse * material_diffuse;
	specular_product = light.specular * material_specular;
	glUniform4fv(glGetUniformLocation(program, "PositionAmbient"),
		1, ambient_product);
	glUniform4fv(glGetUniformLocation(program, "PositionDiffuse"),
		1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program, "PositionSpecular"),
		1, specular_product);



	glUniform1f(glGetUniformLocation(program, "PosConstAtt"),
		light.const_att);
	glUniform1f(glGetUniformLocation(program, "PosLinearAtt"),
		light.linear_att);
	glUniform1f(glGetUniformLocation(program, "PosQuadAtt"),
		light.quad_att);
}


//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices, GLenum mode)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));


	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point4) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(color4) * num_vertices + sizeof(point4) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

	GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point4) * num_vertices + sizeof(color4) * num_vertices + sizeof(vec3) * num_vertices));

	/* Draw a sequence of geometric objs (triangles) from the vertex buffer
	(using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(mode, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
	glDisableVertexAttribArray(vNormal);
	glDisableVertexAttribArray(vTexCoord);
}

void drawFirework(GLuint buffer, int num_vertices, GLenum mode) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint pColor = glGetAttribLocation(fireworkProgram, "color");
	glEnableVertexAttribArray(pColor);
	glVertexAttribPointer(pColor, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint pVelocity = glGetAttribLocation(fireworkProgram, "velocity");
	glEnableVertexAttribArray(pVelocity);
	glVertexAttribPointer(pVelocity, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(fireworkColors)));

	glDrawArrays(mode, 0, num_vertices);

	glDisableVertexAttribArray(pColor);
	glDisableVertexAttribArray(pVelocity);
}

void renderFloor(mat4& mv) {
	mv = LookAt(eye, at, up);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
	drawObj(floor_buffer, 6, GL_TRIANGLES);
}

//----------------------------------------------------------------------------
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");
	Lighting = glGetUniformLocation(program, "lighting");
	Smooth = glGetUniformLocation(program, "smoothFlag");
	Spot = glGetUniformLocation(program, "SpotlightFlag");
	Fog = glGetUniformLocation(program, "fog");
	Linear = glGetUniformLocation(program, "linear");
	Exp = glGetUniformLocation(program, "exponent");
	Square = glGetUniformLocation(program, "square");

	glUniform1f(Smooth, smoothFlag);
	glUniform1f(Lighting, lightingFlag);
	glUniform1f(Spot, spotFlag);

	glUniform1f(Fog, fogFlag);
	glUniform1f(Linear, linearFlag);
	glUniform1f(Exp, expFlag);
	glUniform1f(Square, squareFlag);

	glUniform1f(glGetUniformLocation(program, "Eye"), eyeFrame);
	glUniform1f(glGetUniformLocation(program, "Obj"), objFrame);

	GLuint Eye = glGetUniformLocation(program, "eye");

	glUniform4fv(Eye, 1, eye);

	mat4  p = Perspective(fovy, aspect, zNear, zFar);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, p);

	const vec3 viewer_pos(0.0, 0.0, 2.0);
	mat4 mv = LookAt(original_eye, at, up);
	vec4 light_position_eyeFrame = p * mv * distant_light;
	glUniform4fv(glGetUniformLocation(program, "LightPosition"),
		1, light_position_eyeFrame);
	glUniform4fv(glGetUniformLocation(program, "LightPosition2"), 1, light.position);

	SetUp_Lighting_Uniform_Vars(mv, sphere_ambient, sphere_diffuse, sphere_specular, sphere_shininess);
	setUpPosLightVars(light, sphere_ambient, sphere_diffuse, sphere_specular, sphere_shininess);

	mv = LookAt(eye, at, up);

	mat4 rotation;
	mat4 transformation;
	if (beginFlag) {
		if (AB) {
			rotation *= Rotate(angle, -8, 0, -3);
			transformation *= Translate(-4 + (angle * 6 * M_PI) / (360 * magnitude), 1.0, 4 - (angle * 16 * M_PI) / (360 * sqrt(73))) * rotation;
		}
		else if (BC) {
			rotation *= Rotate(angle, 9, 0, -4);
			transformation *= Translate(-1 + (angle * 8 * M_PI) / (360 * magnitude), 1, -4 + (angle * 18 * M_PI) / (360 * sqrt(97)))  * rotation;
		}
		else if (CA) {
			rotation *= Rotate(angle, -1, 0, 7);
			transformation = Translate(3 - (angle * 2 * M_PI) / 360, 1, 5 - (angle * 2 * M_PI) / (7 * 360)) * rotation;
		}
	}
	else  {
		transformation = Translate(-4, 1, 4);
	}
	mv *= transformation;

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);

	// Set up the Normal Matrix from the model-view matrix
	mv = transformation;
	mat3 normal_matrix = NormalMatrix(mv, 1);
	// Flag in NormalMatrix(): 
	//    1: model_view involves non-uniform scaling
	//    0: otherwise.
	// Using 1 is always correct.  
	// But if no non-uniform scaling, 
	//     using 0 is faster (avoids matrix inverse computation).

	glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
		1, GL_TRUE, normal_matrix);

	if (wireFlag) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		sphereTexFlag = 0.0;
	}
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	if (conFlag) {
		glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
	}
	else glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
	glUniform1f(glGetUniformLocation(program, "Lattice"), latticeFlag);
	glUniform1f(glGetUniformLocation(program, "Upright"), upFlag);
	glUniform1f(glGetUniformLocation(program, "Tilted"), tiltFlag);
	glUniform1f(glGetUniformLocation(program, "TextureFlag"), sphereTexFlag);
	glUniform1f(glGetUniformLocation(program, "Check"), checkFlag);
	glUniform1f(glGetUniformLocation(program, "Vertical"), verticalFlag);
	glUniform1f(glGetUniformLocation(program, "Slanted"), slantedFlag);
	drawObj(cube_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the cube
	glUniform1f(glGetUniformLocation(program, "Vertical"), 0.0);
	glUniform1f(glGetUniformLocation(program, "Slanted"), 0.0);
	glUniform1f(glGetUniformLocation(program, "TextureFlag"), 0.0);
	glUniform1f(glGetUniformLocation(program, "Lattice"), 0.0);

	if (groundFlag) {
		glUniform1f(glGetUniformLocation(program, "TextureFlag"), 1.0);
	}
	else glUniform1f(glGetUniformLocation(program, "TextureFlag"), 0.0);
	glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
	mv = LookAt(original_eye, at, up);
	normal_matrix = NormalMatrix(mv, 1);
	glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),
		1, GL_TRUE, normal_matrix);

	SetUp_Lighting_Uniform_Vars(mv, floor_ambient, floor_diffuse, floor_specular, floor_shininess);
	setUpPosLightVars(light, floor_ambient, floor_diffuse, floor_specular, floor_shininess);
	glDepthMask(GL_FALSE);
	renderFloor(mv);

	glDepthMask(GL_TRUE);
	glUniform1f(glGetUniformLocation(program, "TextureFlag"), 0.0);
	
	GLfloat tempFlag;
	if (shadowFlag) {
		if (blendFlag) {
			glEnable(GL_BLEND);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniform1f(glGetUniformLocation(program, "Lattice"), latticeFlag);
		tempFlag = lightingFlag;
		lightingFlag = 0;
		glUniform1f(Lighting, lightingFlag);
		if (wireFlag) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		mat4 shadowMatrix;
		mv = LookAt(eye, at, up);
		setUpShadowMatrix(shadowMatrix);
		mv *= Translate(light.position) * shadowMatrix  * Translate(-light.position) * transformation;
		glUniformMatrix4fv(glGetUniformLocation(regularProgram, "ModelView"), 1, GL_TRUE, mv);
		glUniformMatrix4fv(glGetUniformLocation(regularProgram, "Projection"), 1, GL_TRUE, p);
		drawObj(shadow_buffer, sphere_NumVertices, GL_TRIANGLES);
		lightingFlag = tempFlag;
		glUniform1f(Lighting, lightingFlag);
		glUniform1f(glGetUniformLocation(program, "Lattice"), 0.0);
		glDisable(GL_BLEND);
	}

	glUseProgram(program);
	mv = LookAt(eye, at, up);
	glUniformMatrix4fv(glGetUniformLocation(program, "ModelView"), 1, GL_TRUE, mv);
	glUniform1f(glGetUniformLocation(program, "lighting"), 0.0);
	drawObj(axes_buffer, axes_NumVertices, GL_LINES);
	glUniform1f(glGetUniformLocation(program, "lighting"), lightingFlag);

	if (groundFlag) {
		glUniform1f(glGetUniformLocation(program, "TextureFlag"), 1.0);
	}
	else glUniform1f(glGetUniformLocation(program, "TextureFlag"), 0.0);
	glUseProgram(program);
	mv = LookAt(eye, at, up);
	SetUp_Lighting_Uniform_Vars(mv, floor_ambient, floor_diffuse, floor_specular, floor_shininess);
	setUpPosLightVars(light, floor_ambient, floor_diffuse, floor_specular, floor_shininess);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	renderFloor(mv);
	glUniform1f(glGetUniformLocation(program, "TextureFlag"), 0.0);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	if (fireworkFlag) {
		glUseProgram(fireworkProgram);
		GLfloat time = glutGet(GLUT_ELAPSED_TIME) - timeInitial;
		time = fmod(time, timeMax);
		glUniform1f(glGetUniformLocation(fireworkProgram, "time"), time);
		drawFirework(firework_buffer, 300, GL_POINTS);
	}
	

	glutSwapBuffers();
}
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void idle(void)
{
	/*Theta[Axis] += 0.01;

	if (Theta[Axis] > 360.0) {
	Theta[Axis] -= 360.0;
	}*/


	if (beginFlag) {
		if ((angle * 2 * M_PI) / 360 <= magnitude) {
			angle += 0.1;
			//angle += 0.01;
		}
		else {
			angle = 0;
			if (AB) {
				AB = false;
				BC = true;
				magnitude = length(point4(3, 1, 5, 1) - point4(-1, 1, -4, 1));

			}
			else if (BC) {
				//angle = 0;
				BC = false;
				CA = true;
				magnitude = length(point4(-4, 1, 4, 1) - point4(3, 1, 5, 1));
			}
			else if (CA) {
				//angle = 0;
				CA = false;
				AB = true;
				magnitude = length(point4(-1, 1, -4, 1) - point4(-4, 1, 4, 1));
			}
		}
	}
	else angle = 0;


	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
	switch (button) {
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN) {
			animationFlag = 1 - animationFlag;
			if (animationFlag == 1 && beginFlag == 1) {
				glutIdleFunc(idle);
			}
			else {
				glutIdleFunc(NULL);
			}
			break;
		}
	}
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {

	case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
	case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
	case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

	case 'b': case 'B':
		beginFlag = 1 - beginFlag;
		break;
	case 'v': case 'V':
		verticalFlag = 1.0;
		slantedFlag = 0.0;
		break;
	case 's': case 'S':
		slantedFlag = 1.0;
		verticalFlag = 0.0;
		break; 
	case 'e': case 'E':
		eyeFrame = 1.0;
		objFrame = 0.0;
		break;
	case 'o': case 'O':
		objFrame = 1.0;
		eyeFrame = 0.0;
		break;
	case 'l': case 'L':
		latticeFlag = 1 - latticeFlag;
		break;
	case 'u': case 'U':
		upFlag = 1.0;
		tiltFlag = 0.0;
		break;
	case 't': case 'T':
		tiltFlag = 1.0;
		upFlag = 0.0;
		break;
	}

	glutPostRedisplay();
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	aspect = (GLfloat)width / (GLfloat)height;
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void optionMenu(int id) {
	switch (id) {
	case 1:
		eye = original_eye;
		break;
	case 2:
		exit(0);
		break;
	case 3:
		shadowFlag = 1 - shadowFlag;
		break;
	case 4:
		lightingFlag = 1 - lightingFlag;
		break;
	case 5:
		wireFlag = 1 - wireFlag;
		break;
	}

}

void typeShade(int id) {
	switch (id) {
	case 1:
		smoothFlag = 0.0;
		break;
	case 2:
		smoothFlag = 1.0;
		break;
	}
}

void typeLight(int id) {
	switch (id) {
	case 1:
		spotFlag = 0.0;
		break;
	case 2:
		spotFlag = 1.0;
		break;
	}
}

void typeFog(int id) {
	switch (id) {
	case 1:
		fogFlag = 0.0;
		break;
	case 2:
		fogFlag = 1.0;
		linearFlag = 1.0;
		expFlag = 0.0;
		squareFlag = 0.0;
		break;
	case 3:
		fogFlag = 1.0;
		expFlag = 1.0;
		linearFlag = 0.0;
		squareFlag = 0.0;
		break;
	case 4:
		fogFlag = 1.0;
		squareFlag = 1.0;
		expFlag = 0.0;
		linearFlag = 0.0;
		break;
	}
}

void blendShadow(int id) {
	switch (id) {
	case 1:
		blendFlag = 1;
		break;
	case 2:
		blendFlag = 0;
		break;
	}
}

void textureGround(int id) {
	switch (id) {
	case 1:
		groundFlag = 1.0;
		break;
	case 2:
		groundFlag = 0.0;
		break;
	}
}

void textureSphere(int id) {
	switch (id) {
	case 1:
		sphereTexFlag = 1.0;
		conFlag = 1.0;
		checkFlag = 0.0;
		break;
	case 2:
		sphereTexFlag = 1.0;
		checkFlag = 1.0;
		conFlag = 0.0;
		break;
	case 3:
		sphereTexFlag = 0.0;
		break;
	}
}

void fireworkOption(int id) {
	switch (id) {
	case 1:
		if (!fireworkFlag) {
			fireworkFlag = 1;
			timeInitial = glutGet(GLUT_ELAPSED_TIME);
		}
		break;
	case 2:
		fireworkFlag = 0;
		break;
	}
}

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{
	int err;

	std::string response;
	std::cout << "What sphere file do you wish to read?" << std::endl;
	std::cin >> response;
	readFile(response);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	// glutInitContextVersion( 3, 2 );
	// glutInitContextProfile( GLUT_CORE_PROFILE );
	glutCreateWindow("Rotating Cube with Shading");


	/* Call glewInit() and error checking */
	err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
		exit(1);
	}

	int shadingMenu = glutCreateMenu(typeShade);
	glutAddMenuEntry("Flat", 1);
	glutAddMenuEntry("Smooth", 2);

	int lightingMenu = glutCreateMenu(typeLight);
	glutAddMenuEntry("Point Source", 1);
	glutAddMenuEntry("Spotlight", 2);

	int fogMenu = glutCreateMenu(typeFog);
	glutAddMenuEntry("No fog", 1);
	glutAddMenuEntry("Linear", 2);
	glutAddMenuEntry("Exponential", 3);
	glutAddMenuEntry("Exponential Square", 4);

	int blendMenu = glutCreateMenu(blendShadow);
	glutAddMenuEntry("Yes", 1);
	glutAddMenuEntry("No", 2);

	int textureGroundMenu = glutCreateMenu(textureGround);
	glutAddMenuEntry("Yes", 1);
	glutAddMenuEntry("No", 2);

	int sphereTextureMenu = glutCreateMenu(textureSphere);
	glutAddMenuEntry("Yes - Contour Lines", 1);
	glutAddMenuEntry("Yes - Checkerboard", 2);
	glutAddMenuEntry("No", 3);

	int fireworkMenu = glutCreateMenu(fireworkOption);
	glutAddMenuEntry("Yes", 1);
	glutAddMenuEntry("No", 2);

	glutCreateMenu(optionMenu);
	glutAddMenuEntry("Default view point", 1);
	glutAddMenuEntry("Quit", 2);
	glutAddMenuEntry("Shadow", 3);
	glutAddMenuEntry("Enable Lighting", 4);
	glutAddMenuEntry("Wireframe Sphere", 5);
	glutAddSubMenu("Shading", shadingMenu);
	glutAddSubMenu("Type of Light", lightingMenu);
	glutAddSubMenu("Fog Options", fogMenu);
	glutAddSubMenu("Blending Shadow", blendMenu);
	glutAddSubMenu("Texture Mapped Ground", textureGroundMenu);
	glutAddSubMenu("Texture Mapped Sphere", sphereTextureMenu);
	glutAddSubMenu("Fireworks", fireworkMenu);
	glutAttachMenu(GLUT_LEFT_BUTTON);

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutIdleFunc(idle);

	init();
	glutMainLoop();
	return 0;
}
