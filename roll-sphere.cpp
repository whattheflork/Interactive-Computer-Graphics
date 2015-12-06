/*	Matt Florkiewicz
	Interactive Computer Graphics
	Professor Yi-Jen Chiang
	Assignment # 2			*/


#include "Angel-yjc.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <string>

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;	/* shader program object id */ 
GLuint program2;

GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axes_buffer;

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 25.0;

//mat4 trans = Translate(-4.0, 1.0, 4.0);
mat4 rotation = identity();

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3.0, -10.0, 1.0); // initial viewer position
vec4 eye(7.0, 3.0, -10.0, 1.0);               // current viewer position

int animationFlag = 1; // 1: animation; 0: non-animation. Toggled by right click
int beginFlag = 0;

bool AB = true;
bool BC = false;
bool CA = false;
float magnitude = sqrt(73);

point3 axes[6];
color3 axesColors[6];


int sphere_NumVertices; //(6 faces)*(2 triangles/face)*(3 vertices/triangle)


point3 sphere_points[1000]; 
color3 sphere_colors[1000];


const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
color3 floor_colors[floor_NumVertices]; // colors for all vertices



// RGBA colors
color3 vertex_colors[8] = {
    color3( 0.0, 0.0, 0.0),  // black
    color3( 1.0, 0.0, 0.0),  // red
    color3( 1.0, 1.0, 0.0),  // yellow
    color3( 0.0, 1.0, 0.0),  // green
    color3( 0.0, 0.0, 1.0),  // blue
    color3( 1.0, 0.0, 1.0),  // magenta
    color3( 1.0, 1.0, 1.0),  // white
    color3( 0.0, 1.0, 1.0)   // cyan
};

//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
void floor()
{
    floor_colors[0] = vertex_colors[3]; floor_points[0] = point3(5,0,8);
    floor_colors[1] = vertex_colors[3]; floor_points[1] = point3(5,0,-4);
    floor_colors[2] = vertex_colors[3]; floor_points[2] = point3(-5,0,-4);

	floor_colors[3] = vertex_colors[3]; floor_points[3] = point3(-5, 0, -4);
	floor_colors[4] = vertex_colors[3]; floor_points[4] = point3(5, 0, 8);
	floor_colors[5] = vertex_colors[3]; floor_points[5] = point3(-5, 0, 8);
}

void readFile(const std::string& fileName) {
	std::ifstream ifs(fileName);
	int size = 0;
	int index = 0;
	float x;
	float y;
	float z;
	float junk;
	ifs >> size;
	sphere_NumVertices = size * 3;
	while (ifs) {
		ifs >> junk >> x >> y >> z;
		sphere_points[index] = point3(x, y, z);
		index++;
		ifs >> x >> y >> z;
		sphere_points[index] = point3(x, y, z);
		index++;
		ifs >> x >> y >> z;
		sphere_points[index] = point3(x, y, z);
		index++;
	}

	for (size_t i = 0; i < sphere_NumVertices; i++) {
		sphere_colors[i] = color3(1.0, 0.84, 0.0);
	}
}

void fillAxes() {
	axes[0] = point3(0, 0, 0);	axesColors[0] = vertex_colors[1];
	axes[1] = point3(10, 0, 0);	axesColors[1] = vertex_colors[1];
	
	axes[2] = point3(0, 0, 0);	axesColors[2] = vertex_colors[5];
	axes[3] = point3(0, 10, 0);	axesColors[3] = vertex_colors[5];

	axes[4] = point3(0, 0, 0);	axesColors[4] = vertex_colors[4];
	axes[5] = point3(0, 0, 10);	axesColors[5] = vertex_colors[4];

}

//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
	fillAxes();

#if 0 //YJC: The following is not needed
    // Create a vertex array object
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
#endif

 // Create and initialize a vertex buffer object for sphere, to be used in display()
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);

#if 0
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_points) + sizeof(cube_colors),
		 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cube_points), cube_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(cube_points), sizeof(cube_colors),
                    cube_colors);
#endif
#if 1
    glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * sphere_NumVertices + sizeof(color3) * sphere_NumVertices,
		 sphere_points, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * sphere_NumVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * sphere_NumVertices, sizeof(color3) * sphere_NumVertices,
                    sphere_colors);
#endif

    floor();     
 // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
		 floor_points, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
                    floor_colors);

	glGenBuffers(1, &axes_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axes_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes) + sizeof(axesColors),
		axes, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axes), sizeof(axesColors),
		axesColors);

    program = InitShader("vshader42.glsl", "fshader42.glsl");

    glEnable( GL_DEPTH_TEST );
	glClearColor(0.529, 0.807, 0.92, 0.0);
    glLineWidth(2.0);
}
//----------------------------------------------------------------------------

void drawObj(GLuint buffer, int num_vertices, GLenum mode)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glEnableClientState(GL_ARRAY_BUFFER);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
			  BUFFER_OFFSET(0) );

    GLuint vColor = glGetAttribLocation(program, "vColor"); 
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
			  BUFFER_OFFSET(sizeof(point3) * num_vertices) ); 
      // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(mode, 0, num_vertices);

    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}



//----------------------------------------------------------------------------
void display( void )
{
  GLuint  model_view;  // model-view matrix uniform shader variable location
  GLuint  projection;  // projection matrix uniform shader variable location
  GLuint trans;

    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram(program2);
	trans = glGetUniformLocation(program, "trans");

    glUseProgram(program); // Use the shader program

    model_view = glGetUniformLocation(program, "model_view" );
	projection = glGetUniformLocation(program, "projection");

    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p); 
	
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);

    mat4  mv = LookAt(eye, at, up);
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); 
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	mat4 transformation;
	if (AB) {
		rotation *= Rotate(angle, 8, 0, 3);
		transformation = Translate(-4 + (angle * 6 * M_PI) / (360 * sqrt(73)), 1.0, 4 - (angle * 16 * M_PI) / (360 * sqrt(73))) * rotation;
	}
	else if (BC) {
		rotation *= Rotate(angle, -9, 0, 4);
		transformation = Translate(-1 + (angle * 8 * M_PI) / (360 * sqrt(97)), 1, -4 + (angle * 18 * M_PI) / (360 * sqrt(97))) * rotation;
	}
	else if (CA) {
		rotation *= Rotate(angle, 1, 0, -7);
		transformation = Translate(3 - (angle * 2 * M_PI) / 360, 1, 5 - (angle * 2 * M_PI) / (7 * 360)) * rotation;
	}
	mv = mv * transformation;
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);

    drawObj(sphere_buffer, sphere_NumVertices, GL_TRIANGLES);  // draw the sphere

	mv = LookAt(eye, at, up);
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); 
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawObj(floor_buffer, floor_NumVertices, GL_TRIANGLES);  // draw the floor
	drawObj(axes_buffer, 6, GL_LINES);


    glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle (void)
{
  //angle += 0.02;
	if (beginFlag) {
		if ((angle * 2 * M_PI) / 360 <= magnitude) {
			angle += 0.2;
		}
		else if (AB) {
			angle = 0;
			AB = false;
			BC = true;
			magnitude = sqrt(97);
		}
		else if (BC) {
			angle = 0;
			BC = false;
			CA = true;
			magnitude = 7;
		}
		else if (CA) {
			angle = 0;
			CA = false;
			AB = true;
		}
	}
  glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {

        case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

		case 'b': case 'B':
			beginFlag = 1 - beginFlag;
			break;

    }
    glutPostRedisplay();
}

void mymouse(int button, int state, int x, int y) {
	switch (button) {
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN) {
			animationFlag = 1 - animationFlag;
			if (animationFlag == 1 && beginFlag == 1) glutIdleFunc(idle);
			else                    glutIdleFunc(NULL);
			break;
		}
	}
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}

void optionMenu(int id) {
	switch (id) {
	case 1:
		eye = init_eye;
		break;
	case 2:
		exit(0);
		break;
	}
}

//----------------------------------------------------------------------------
int main(int argc, char **argv)
{ int err;
	
	std::string response;
	std::cout << "What file would you like to read?" << std::endl;
	std::cin >> response;
	readFile(response);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    // glutInitContextVersion(3, 2);
    // glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("Rolling Sphere");

	
  /* Call glewInit() and error checking */
  err = glewInit();
  if (GLEW_OK != err)
  { printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
    exit(1);
  }

  glutCreateMenu(optionMenu);
  glutAddMenuEntry("Default view point", 1);
  glutAddMenuEntry("Quit", 2);
  glutAttachMenu(GLUT_LEFT_BUTTON);


    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
	glutMouseFunc(mymouse);

    init();
    glutMainLoop();
    return 0;
}
