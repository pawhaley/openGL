/*
Parker Whaley
question 2
*/
#include <ctime>
#include "Angel.h"
void m_glewInitAndVersion(void);
void reshape(int width, int height);

//how many sides does a "circle" have?
#define fine 100
//the angle of a triangular wedge
const GLfloat rotAng = M_PI * 2 / fine;

GLuint program; 

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint buffer[5]; //Buffer Object to store the cube vertex attributes


GLfloat zoom = 1.0;
GLfloat step = .5;
GLfloat theta_y = 0;
int spinToggle = 0; 
GLfloat d=2.5;
int w; int h;






//uniform variable locations
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;


//make sure you start with the default coordinate system
mat4 projmat = Angel::mat4(1.0);
mat4 modelviewStackTop = Angel::mat4(1.0);





//----------------------------------------------------------------------------
//GLuint vColor;
GLuint vPosition;
// OpenGL initialization
void
init()
{
	glGenBuffers(5, buffer);
	
	{//create a cap
		glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
		vec4 points[3 * fine];
			for (int i = 0; i < 3*fine; i+=3) {
				int angmultiply = i / 3;
				points[i] = point4(sin(angmultiply*rotAng), cos(angmultiply*rotAng), 0, 1);
			}
			for (int i = 1; i < 3 * fine; i += 3) {
				points[i] = point4(0, 0, 0, 1);
			}
			for (int i = 2; i < 3 * fine; i += 3) {
				int angmultiply = i / 3+1;
				points[i] = point4(sin(angmultiply*rotAng), cos(angmultiply*rotAng), 0, 1);
			}
			glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	}

	{//create a tube
		glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
		vec4 points[6 * fine];
		for (int i = 0; i < 6 * fine; i += 6) {
			int angmultiply = i / 6;
			points[i] = point4(sin(angmultiply*rotAng), cos(angmultiply*rotAng), .5, 1);
			points[i + 1] = point4(sin(angmultiply*rotAng), cos(angmultiply*rotAng), -.5, 1);
			points[i + 2] = point4(sin((angmultiply+1)*rotAng), cos((angmultiply + 1)*rotAng), -.5, 1);
			points[i + 3] = points[i + 2];
			points[i + 4] = point4(sin((angmultiply + 1)*rotAng), cos((angmultiply + 1)*rotAng), .5, 1);
			points[i + 5] = points[i];
		}
		
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	}


   // Load shaders and use the resulting shader program
    program = InitShader( "vshaderCC_v150.glsl", "fshaderCC_v150.glsl" );
    glUseProgram( program );
	vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	color_loc = glGetUniformLocation(program, "vColor");
	proj_loc       = glGetUniformLocation(program, "projection");
	model_view_loc = glGetUniformLocation(program, "modelview");

   
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
}

//----------------------------------------------------------------------------

//how to display a cap
void cap(mat4 pos) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, pos);
	glDrawArrays(GL_TRIANGLES, 0, 3*fine);
}
//how to display a tube
void tube(mat4 pos) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, pos);
	glDrawArrays(GL_TRIANGLES, 0, 6 * fine);
}
//how to display a cannon
void cannon(mat4 pos) {
	//create the caped gun
	glUniform4fv(color_loc, 1, Angel::vec4(1, 0, 0, 1.0));
	tube(pos*RotateZ(30)*RotateY(90)*Scale(.05, .05, 1));
	glUniform4fv(color_loc, 1, Angel::vec4(0.0, 0.0, 0, 1.0));
	cap(pos*RotateZ(30)*RotateY(90)*Scale(.05, .05, 1)*Translate(0, 0, .5));
	glUniform4fv(color_loc, 1, Angel::vec4(0.5, 0.5, 0.5, 1.0));
	cap(pos*RotateZ(30)*RotateY(90)*Scale(.05, .05, 1)*Translate(0, 0, -.5));

	//axle
	vec4 brown = vec4(130, 90, 44, 0) / 200. + vec4(0, 0, 0, 1);
	glUniform4fv(color_loc, 1, brown);
	tube(pos*Translate(0,-.05+-.01,0)*Scale(.01, .01, .4));
	//one tire
	GLfloat ang = 30 / 180.*M_PI;
	glUniform4fv(color_loc, 1, Angel::vec4(0.5, 0.5, 0.5, 1.0));
	cap(pos*Translate(0, -.05 + -.01, .2+.5*.05)*Scale(.5*sin(ang), .5*sin(ang), 1));
	cap(pos*Translate(0, -.05 + -.01, .2 - .5*.05)*Scale(.5*sin(ang), .5*sin(ang), 1));
	glUniform4fv(color_loc, 1, Angel::vec4(0, 0, 0, 1.0));
	tube(pos*Translate(0, -.05 + -.01, .2)*Scale(.5*sin(ang), .5*sin(ang), .05));
	//the other tire
	glUniform4fv(color_loc, 1, Angel::vec4(0.5, 0.5, 0.5, 1.0));
	cap(pos*Translate(0, -.05 + -.01, -.2 + .5*.05)*Scale(.5*sin(ang), .5*sin(ang), 1));
	cap(pos*Translate(0, -.05 + -.01, -.2 - .5*.05)*Scale(.5*sin(ang), .5*sin(ang), 1));
	glUniform4fv(color_loc, 1, Angel::vec4(0, 0, 0, 1.0));
	tube(pos*Translate(0, -.05 + -.01, -.2)*Scale(.5*sin(ang), .5*sin(ang), .05));

}
//put the cannon on the screen
void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    projmat=Angel::mat4(1.0); //Identity matrix
	//Position and orient the camera
	mat4 modeltemp = Translate(0.0, 0.0, -d)*modelviewStackTop;
	//Set up the camera optics
	projmat = projmat*Perspective(zoom * 30, 1.0, 1.0, 10.0);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	cannon(modeltemp);
	

    glutSwapBuffers();
}


//----------------------------------------------------------------------------

void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {
	case 'z':
		zoom *= 1.05;
		break;
	case 'Z':
		zoom *= .95;
		break;
	case 033:  // Escape key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
    }
}


void idle(void){
	glutPostRedisplay();
}
//----------------------------------------------------------------------------

bool clicked = false;
int ixclick = 0;
int iyclick = 0;

void mouseCall(int button, int state, int ix, int iy) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		clicked = true;
		ixclick = ix;
		iyclick = iy;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		clicked = false;
	}
}

void mouse(int x, int y) {
	if (clicked == true) {
		modelviewStackTop = RotateY((x-ixclick)*step)*modelviewStackTop;
		modelviewStackTop = RotateX((y - iyclick)*step)*modelviewStackTop;
		ixclick = x;
		iyclick = y;
	}
}


int main( int argc, char **argv )
{

	srand(std::time(NULL));
	rand();
    glutInit( &argc, argv );
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize( 512, 512 );
	glutCreateWindow( "Color Cube" );
	 
	m_glewInitAndVersion();
	

    init();

	glutReshapeFunc(reshape);
    glutDisplayFunc( display );

    glutKeyboardFunc( keyboard );
	glutMotionFunc(mouse);
	glutMouseFunc(mouseCall);

	glutIdleFunc(idle);
	

    glutMainLoop();
    return 0;
}

void m_glewInitAndVersion(void)
{
   fprintf(stdout, "OpenGL Version: %s\n", glGetString(GL_VERSION));
   fprintf(stdout, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
   fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
   }
   fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}

void reshape( int width, int height )
//the same objects are shown (possibly scaled) w/o shape distortion 
//original viewport is a square
{

	glViewport( 0, 0, (GLsizei) width, (GLsizei) height );

}
