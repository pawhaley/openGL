/*
Parker Whaley
A4
*/
#include <ctime>
#include <vector>
#include "Angel.h"
void m_glewInitAndVersion(void);
void reshape(int width, int height);

#define fine 7
#define numFunc 20
int lightSenario = 0;

const GLfloat rotAng = M_PI * 2 / fine;

GLuint program; 

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint buffer[6]; //Buffer Object to store the cube vertex attributes
int bufferlength[6];

GLfloat zoom = 1.0;
GLfloat step = .1;
GLfloat d=2.5;
GLfloat transdiff = .1;
int widthInPix;
int hightInPix;
double lightRotation = 0;
double rotAngLight = M_PI / 100;
color4 brown =  color4(1, .829, .609, 1);



//uniform variable locations
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;
GLuint  AmbientProduct, DiffuseProduct, SpecularProduct, LightPosition, Shininess, ModelWorld, inFrame;


//make sure you start with the default coordinate system
mat4 projmat = Angel::mat4(1.0);
mat4 modelviewStackTop = Translate(0.0, 0.0, 0.0);





//----------------------------------------------------------------------------
//GLuint vColor;
GLuint vPosition;
GLuint vNormal;
// OpenGL initialization


struct node {
	vec4 data;
	color4 color;
	point4 normal;
	node* next;
};

node* pushBack(node* add, vec4 val,color4 color,point4 normal) {
	node* valnode = new node;
	valnode->data = val;
	valnode->color = color;
	valnode->normal = normal;
	valnode->next = add->next;
	add->next = valnode;
	return valnode;
}

node* setup(vec4 in,color4 color, point4 normal) {
	node* valnode = new node;
	valnode->next = nullptr;
	valnode->data = in;
	valnode->color = color;
	valnode->normal = normal;
	return valnode;
}

node* combine(node* one, node* two) {
	while(one->next!=nullptr) {
		one = one->next;
	}
	one->next = two;
	return one;
}

node* multiply(Angel::mat4 matrix,node* list) {
	node* retval = list;
	while (list != nullptr) {
		list->data = matrix*list->data;
		list->normal = matrix*list->normal;
		list = list->next;
	}
	return retval;
}

node* copy(node* fromMe) {
	node* retval=setup(fromMe->data, fromMe->color, fromMe->normal);
	node* top=retval;
	while (fromMe->next != nullptr) {
		fromMe = fromMe->next;
		top = pushBack(top, fromMe->data, fromMe->color, fromMe->normal);
	}
	return retval;
}

void destroyNode(node* top) {
	node* temp;
	while (top != nullptr) {
		temp = top->next;
		delete top;
		top = temp;
	}
}

void CopyToBuffer(node* top, int buff1, int buff2, int buff3) {
	int length = 0;
	node* temp = top;
	while (temp != nullptr) {
		length++;
		temp = temp->next;
	}
	bufferlength[buff1] = length;
	bufferlength[buff2] = length;
	bufferlength[buff3] = length;
	vec4* points = new vec4[length];
	color4* colors = new color4[length];
	point4* normals = new point4[length];
	temp = top;
	int n = 0;
	while (temp != nullptr) {
		points[n] = temp->data;
		colors[n] = temp->color;
		normals[n] = temp->normal;
		n++;
		temp = temp->next;
	}
	glBindBuffer(GL_ARRAY_BUFFER, buffer[buff1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*length, points, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[buff2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*length, colors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[buff3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*length, normals, GL_STATIC_DRAW);
	delete[] points;
	delete[] colors;
	delete[] normals;
}

void devideTryforsphere(int n, point4 p1, point4 p2, point4 p3, color4 color,node* wheretoadd) {
	point4 normal;
	if (n == 0) {
		normal = point4(p1.x, p1.y, p1.z, 0);
		pushBack(wheretoadd, p1, color, normal);
		normal = point4(p2.x, p2.y, p2.z, 0);
		pushBack(wheretoadd, p2, color, normal);
		normal = point4(p3.x, p3.y, p3.z, 0);
		pushBack(wheretoadd, p3, color, normal);
		return;
	}
	n--;
	point4 central[3] = { (p1 + p2)/2,(p1 + p3) / 2,(p3 + p2) / 2 };
	for (int i = 0; i < 3; i++) {
		central[i] = normalize(point4(central[i].x, central[i].y, central[i].z, 0)) + point4(0, 0, 0, 1);
	}
	//devideTryforsphere(n, p1, p2, p3, color, wheretoadd);
	devideTryforsphere(n, p1, central[0], central[1], color, wheretoadd);
	devideTryforsphere(n, p2, central[0], central[2], color, wheretoadd);
	devideTryforsphere(n, p3, central[2], central[1], color, wheretoadd);
	devideTryforsphere(n, central[2], central[0], central[1], color, wheretoadd);
}

node* sphere(color4 color) {
	node* wheretoadd = setup(vec4(0, 0, 0, 0), color, vec4(0, 0, 0, 0));
	point4 v[4] = {//from prof's code
		vec4(0.0, 0.0, 1.0, 1.0),
		vec4(0.0, 0.942809, -0.333333, 1.0),
		vec4(-0.816497, -0.471405, -0.333333, 1.0),
		vec4(0.816497, -0.471405, -0.333333, 1.0)
	};//end prof's code
	devideTryforsphere(fine, v[0], v[1], v[2], color, wheretoadd);
	devideTryforsphere(fine, v[0], v[1], v[3], color, wheretoadd);
	devideTryforsphere(fine, v[0], v[3], v[2], color, wheretoadd);
	devideTryforsphere(fine, v[3], v[1], v[2], color, wheretoadd);
	node* temp = wheretoadd;
	wheretoadd = wheretoadd->next;
	temp->next = nullptr;
	delete temp;
	return wheretoadd;
}

vec4 cordTransformPos(double theta, double phi) {
	double r = .75 + .25*sin(phi);
	vec4 retval(r*cos(theta), r*sin(theta), .25*cos(phi), 1);
	return retval;
}

vec4 cordTransformNorm(double theta, double phi) {
	double r = .25*sin(phi);
	vec4 retval(sin(phi)*cos(theta), sin(phi)*sin(theta), cos(phi), 0);
	return normalize(retval);
}

node* toris(color4 color) {
	node* retval = setup(vec4(0, 0, 0, 0), color, vec4(0, 0, 0, 0));
	for (int i = 0; i < fine * 5; i++) {
		double theta1 = 2 * M_PI*i / (fine * 5);
		double theta2 = 2 * M_PI*(i+1) / (fine * 5);
		for (int j = 0; j < fine * 5; j++) {
			double phi1 = 2 * M_PI*j / (fine * 5);
			double phi2 = 2 * M_PI*(j+1) / (fine * 5);
			pushBack(retval,cordTransformPos(theta1, phi1), color, cordTransformNorm(theta1, phi1));
			pushBack(retval, cordTransformPos(theta2, phi1), color, cordTransformNorm(theta2, phi1));
			pushBack(retval, cordTransformPos(theta2, phi2), color, cordTransformNorm(theta2, phi2));

			pushBack(retval, cordTransformPos(theta2, phi2), color, cordTransformNorm(theta2, phi2));
			pushBack(retval, cordTransformPos(theta1, phi2), color, cordTransformNorm(theta1, phi2));
			pushBack(retval, cordTransformPos(theta1, phi1), color, cordTransformNorm(theta1, phi1));
		}
	}
	node* temp = retval;
	retval = retval->next;
	temp->next = nullptr;
	delete temp;
	return retval;
}


void
init()
{
	srand(std::time(NULL));
	glGenBuffers(6, buffer);
	
	{//sphere
		node* sphereP = sphere(color4(0,0,0,0));
		CopyToBuffer(sphereP, 0, 1,1);//ignore the color buff
		destroyNode(sphereP);
	}

	{//tor
		node* torP = toris(color4(0, 0, 0, 0));
		CopyToBuffer(torP, 2, 3, 3);//ignore the color buff
		destroyNode(torP);
	}

   // Load shaders and use the resulting shader program
    program = InitShader( "vshaderCC_v150.glsl", "fshaderCC_v150.glsl" );
    glUseProgram( program );
	vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);

	proj_loc       = glGetUniformLocation(program, "Projection");
	model_view_loc = glGetUniformLocation(program, "ModelView");
	ModelWorld = glGetUniformLocation(program, "ModelWorld");

	AmbientProduct = glGetUniformLocation(program, "AmbientProduct");
	DiffuseProduct = glGetUniformLocation(program, "DiffuseProduct");
	SpecularProduct = glGetUniformLocation(program, "SpecularProduct");
	LightPosition = glGetUniformLocation(program, "LightPosition");
	Shininess = glGetUniformLocation(program, "Shininess");
	inFrame = glGetUniformLocation(program, "inFrame");

	glUniform1f(Shininess,15);
	glUniform4fv(AmbientProduct,1, color4(0.2, 0.2, 0.2, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.5,.5,.5, 1.0));
	glUniform4fv(SpecularProduct, 1, color4(.8,.8,.8, 1.0));
	glUniform4fv(LightPosition, 1, point4(0,15,-15,1));
   
    glClearColor( 0,0,0, 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
}

//----------------------------------------------------------------------------




void sphere(mat4 pos) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glUniformMatrix4fv(ModelWorld, 1, GL_TRUE, pos);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	for (int i = 0; i < bufferlength[0]; i += 3) {
		glDrawArrays(GL_TRIANGLES, i, 3);
	}
}

void tor(mat4 pos) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glUniformMatrix4fv(ModelWorld, 1, GL_TRUE, pos);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[3]);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	for (int i = 0; i < bufferlength[2]; i += 3) {
		glDrawArrays(GL_TRIANGLES, i, 3);
	}
}



void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    projmat=Angel::mat4(1.0); //Identity matrix
	//Position and orient the camera
	//Set up the camera optics
	projmat = projmat* Frustum(-.1*widthInPix/512., .1*widthInPix / 512., -.1*hightInPix / 512., .1*hightInPix / 512., .5, 1000000);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
	switch (lightSenario)
	{
	case 0:
		glUniform1f(inFrame, false);
		glUniform4fv(LightPosition, 1, point4(0, 15, 15, 0));
		break;
	case 1:
		glUniform1f(inFrame, false);
		lightRotation += rotAngLight;
		glUniform4fv(LightPosition, 1, point4(15 * sin(lightRotation), 15, -30+15*cos(lightRotation), 1));
		break;
	case 2:
		glUniform1f(inFrame, true);
		glUniform4fv(LightPosition, 1, point4(0,0,0, 1));
		break;
	default:
		break;
	}

	if (lightSenario != 1) {
		glUniform4fv(SpecularProduct, 1, color4(.8, .8, .8, 1.0));
		glUniform4fv(AmbientProduct, 1, color4(0.05, 0.05, 0.05, 1.0));
		glUniform4fv(DiffuseProduct, 1, color4(.5, .5, .5, 1.0)*color4(1, 1, 1, 1.0));
		glUniform1f(Shininess, 25);
		tor(Translate(-5, 0, -30)*RotateY(-45));

		glUniform4fv(DiffuseProduct, 1, color4(.5, .5, .5, 1.0)*color4(1, .84, 0, 1.0));
		sphere(Translate(2, 2, -33));

		glUniform4fv(AmbientProduct, 1, color4(0.3, 0.3, 0.3, 1.0)*color4(1, .11, .19, 1.0));
		glUniform4fv(DiffuseProduct, 1, color4(.3, .3, .3, 1.0)*color4(1, .11, .19, 1.0));
		glUniform1f(Shininess, 5);
		sphere(Translate(0, 0, -33)*Scale(.5, 1, .25));
	}
	else {
		glUniform4fv(SpecularProduct, 1, color4(0,0,0, 1.0));
		glUniform4fv(AmbientProduct, 1, color4(0.05, 0.05, 0.05, 1.0));
		glUniform4fv(DiffuseProduct, 1, color4(0,0,0, 1.0)*color4(1, 1, 1, 1.0));
		glUniform1f(Shininess, 25);
		tor(Translate(-5, 0, -30)*RotateY(-45));

		glUniform4fv(SpecularProduct, 1, color4(.8, .8, .8, 1.0));
		glUniform4fv(DiffuseProduct, 1, color4(.5, .5, .5, 1.0)*color4(1, .84, 0, 1.0));
		sphere(Translate(2, 2, -33));

		glUniform4fv(SpecularProduct, 1, color4(0,0,0, 1.0));
		glUniform4fv(AmbientProduct, 1, color4(.1,.1,.1, 1.0)*color4(1, .11, .19, 1.0));
		glUniform4fv(DiffuseProduct, 1, color4(0,0,0, 1.0)*color4(1, .11, .19, 1.0));
		glUniform1f(Shininess, 5);
		sphere(Translate(0, 0, -33)*Scale(.5, 1, .25));
	}



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
	case 'w':
		modelviewStackTop = Translate(0.0, 0.0, transdiff)*modelviewStackTop;
		break;
	case 's':
		modelviewStackTop = Translate(0.0, 0.0, -transdiff)*modelviewStackTop;
		break;
	case 'd':
		modelviewStackTop = Translate(-transdiff,0,0)*modelviewStackTop;
		break;
	case 'a':
		modelviewStackTop = Translate(transdiff,0,0)*modelviewStackTop;
		break;
	case 'e':
		modelviewStackTop = RotateZ(step*10)*modelviewStackTop;
		break;
	case 'q':
		modelviewStackTop = RotateZ(-step*10)*modelviewStackTop;
		break;
	case 'c':
		lightSenario++;
		lightSenario %= 3;
		break;
	case 033:  // Escape key
	case 'Q':
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
		modelviewStackTop = RotateY(-(x-ixclick)*step)*modelviewStackTop;
		modelviewStackTop = RotateX(-(y - iyclick)*step)*modelviewStackTop;
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
	glutCreateWindow( "world" );
	 
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

void reshape( int w, int h )
//the same objects are shown (possibly scaled) w/o shape distortion 
//original viewport is a square
{

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	widthInPix = w;
	hightInPix = h;
	std::cout << w << " " << h << std::endl;

}
