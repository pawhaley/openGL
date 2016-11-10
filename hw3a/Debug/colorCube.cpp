/*
Parker Whaley
question 2
*/
#include <ctime>
#include <vector>
#include "Angel.h"
void m_glewInitAndVersion(void);
void reshape(int width, int height);

//how many sides does a "circle" have?
#define fine 20
#define numFunc 20
//the angle of a triangular wedge
const GLfloat rotAng = M_PI * 2 / fine;

GLuint program; 

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint buffer[6]; //Buffer Object to store the cube vertex attributes
int bufferlength[6];

GLfloat zoom = 1.0;
GLfloat step = .1;
GLfloat d=2.5;
GLfloat transdiff = 3.5;
int widthInPix;
int hightInPix;

color4 brown =  color4(1, .829, .609, 1);



//uniform variable locations
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;
GLuint vcolor;
GLuint bvColor;
GLuint bColor;


//make sure you start with the default coordinate system
mat4 projmat = Angel::mat4(1.0);
mat4 modelviewStackTop = Translate(0.0, 0.0, -10);
//mat4 translation = Translate(0.0, 0.0, -d);





//----------------------------------------------------------------------------
//GLuint vColor;
GLuint vPosition;
// OpenGL initialization
color4 getColor(GLfloat z) {
	color4 white = color4(1, 1, 1, 1);
	color4 green = color4(0, 1, 0, 1);
	color4 darkness = color4(0, 1, 0, 0);
	//return green;
	return (.5 - z) * 2 * (green- ((GLfloat)rand() / (GLfloat)RAND_MAX)*darkness/3) +2 * z * white;
}

struct node {
	vec4 data;
	color4 color;
	node* next;
};

node* pushBack(node* add, vec4 val,color4 color) {
	node* valnode = new node;
	valnode->data = val;
	valnode->color = color;
	valnode->next = add->next;
	add->next = valnode;
	return valnode;
}

node* setup(vec4 in,color4 color) {
	node* valnode = new node;
	valnode->next = nullptr;
	valnode->data = in;
	valnode->color = color;
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
		list = list->next;
	}
	return retval;
}

node* copy(node* fromMe) {
	node* retval=setup(fromMe->data, fromMe->color);
	node* top=retval;
	while (fromMe->next != nullptr) {
		fromMe = fromMe->next;
		top = pushBack(top, fromMe->data, fromMe->color);
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

void CopyToBuffer(node* top, int buff1, int buff2) {
	int length = 0;
	node* temp = top;
	while (temp != nullptr) {
		length++;
		temp = temp->next;
	}
	bufferlength[buff1] = length;
	bufferlength[buff2] = length;
	vec4* points = new vec4[length];
	color4* colors = new color4[length];
	temp = top;
	int n = 0;
	while (temp != nullptr) {
		points[n] = temp->data;
		colors[n] = temp->color;
		n++;
		temp = temp->next;
	}
	glBindBuffer(GL_ARRAY_BUFFER, buffer[buff1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*length, points, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[buff2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*length, colors, GL_STATIC_DRAW);
	delete[] points;
	delete[] colors;
}

node* triangle(color4 color) {
	node* retval = setup(vec4(0, 0, 0, 1),color);
	node* last = retval;
	last = pushBack(last, vec4(1, 0, 0, 1),color);
	last = pushBack(last, vec4(1/2., 0, 1, 1),color);
	return retval;
}

node* square(color4 color) {
	node* retval = setup(vec4(0, 0, 0, 1), color);
	node* last = retval;
	last = pushBack(last, vec4(1, 0, 0, 1), color);
	last = pushBack(last, vec4(1, 0, 1, 1), color);
	last = pushBack(last, vec4(1, 0, 1, 1), color);
	last = pushBack(last, vec4(0, 0, 1, 1), color);
	last = pushBack(last, vec4(0, 0, 0, 1), color);
	return retval;
}

node* roof() {
	node* retval = triangle(Scale(.6, .6, .6) *brown);
	multiply(Translate(0,.125,0), retval);
	combine(retval, multiply(Translate(0,.875 , 0), triangle(Scale(.6, .6, .6) *brown)));
	combine(retval, multiply(RotateY(-63.44)*RotateX(-90.)*Scale(sqrt(1.25)+.001,1,1), square(Scale(.5,.5,.5)*Scale(.6, .6, .6) *brown)));
	combine(retval, multiply(Translate(.5,0,1)*RotateY(63.43)*RotateX(-90.)*Scale(sqrt(1.25) + .001, 1, 1), square(Scale(.5, .5, .5)*Scale(.6, .6, .6) *brown)));
	return retval;
}

node* houseB() {
	node* retval = square(brown);
	combine(retval, multiply(Translate(0, 1, 0), square(brown)));
	combine(retval, multiply(Translate(0, 0, 0)*RotateZ(90), square(brown)));
	combine(retval, multiply(Translate(1, 0, 0)*RotateZ(90), square(brown)));
	return retval;
}

node* house() {
	node* retval = multiply(Translate(0,0,2./3)*Scale(1,1,1./3),roof());
	combine(retval, multiply(Translate(0, .125, 0)*Scale(1, 1-2*.125, 2. / 3), houseB()));
	return retval;
}

node* cone(color4 color) {
	GLfloat rotAng = M_PI * 2. / fine;
	node* retval = setup(vec3(.5+.5*sin(0), .5 + .5*cos(0),0),color);
	node* top = retval;
	top = pushBack(top, vec3(.5, .5, 1),color);
	top = pushBack(top, vec3(.5 + .5*sin(rotAng), .5 + .5*cos(rotAng), 0), color);
	for (int i = 1; i < fine; i++) {
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*i), .5 + .5*cos(rotAng*i), 0), color);
		top = pushBack(top, vec3(.5, .5, 1), color);
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*(i+1)), .5 + .5*cos(rotAng*(i + 1)), 0), color);
	}
	return retval;
}

node* disk(color4 color) {
	GLfloat rotAng = M_PI * 2. / fine;
	node* retval = setup(vec3(.5 + .5*sin(0), .5 + .5*cos(0), 0), color);
	node* top = retval;
	top = pushBack(top, vec3(.5, .5, 0), color);
	top = pushBack(top, vec3(.5 + .5*sin(rotAng), .5 + .5*cos(rotAng), 0), color);
	for (int i = 1; i < fine; i++) {
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*i), .5 + .5*cos(rotAng*i), 0), color);
		top = pushBack(top, vec3(.5, .5, 0), color);
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*(i + 1)), .5 + .5*cos(rotAng*(i + 1)), 0), color);
	}
	return retval;
}

node* cil(color4 color) {
	GLfloat rotAng = M_PI * 2. / fine;
	node* retval = setup(vec3(.5 + .5*sin(0), .5 + .5*cos(0), 0), color);
	node* top = retval;
	top = pushBack(top, vec3(.5 + .5*sin(0), .5 + .5*cos(0), 1), color);
	top = pushBack(top, vec3(.5 + .5*sin(rotAng), .5 + .5*cos(rotAng), 1), color);
	top = pushBack(top, vec3(.5 + .5*sin(rotAng), .5 + .5*cos(rotAng), 1), color);
	top = pushBack(top, vec3(.5 + .5*sin(rotAng), .5 + .5*cos(rotAng), 0), color);
	top = pushBack(top, vec3(.5 + .5*sin(0), .5 + .5*cos(0), 0), color);
	for (int i = 1; i < fine; i++) {
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*i), .5 + .5*cos(rotAng*i), 0), color);
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*i), .5 + .5*cos(rotAng*i), 1), color);
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*(i+1)), .5 + .5*cos(rotAng*(i+1)), 1), color);
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*(i + 1)), .5 + .5*cos(rotAng*(i + 1)), 1), color);
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*(i + 1)), .5 + .5*cos(rotAng*(i + 1)), 0), color);
		top = pushBack(top, vec3(.5 + .5*sin(rotAng*i), .5 + .5*cos(rotAng*i), 0), color);
	}
	return retval;
}

node* treeTop(color4 color) {
	node* retval = cone(color);
	combine(retval, disk(color));
	return retval;
}

node* treeBot(color4 color) {
	node* retval = cil(color);
	combine(retval, disk(color));
	return retval;
}

node* tree() {
	node* retval =multiply(Translate(0,0,1./6)*Scale(1,1,5./6), treeTop(color4(0,.35,0,1)));
	combine(retval, multiply(Translate(0, 0, 1- 4. / 6)*Scale(1, 1, 4. / 6), treeTop(color4(0, .35, 0, 1))));
	combine(retval, multiply(Translate(1. / 3, 1. / 3, 0)*Scale(1. / 3, 1. / 3, 1. / 6), treeBot(Scale(.5, .5, .5)*Scale(.6, .6, .6) *brown)));
	return retval;
}


struct objectInWorld
{
	int type;
	GLfloat rotateT;
	GLfloat hightT;
	GLfloat lengthT;
	GLfloat widthT;
	GLfloat x;
	GLfloat y;
};

std::vector<objectInWorld> allObj;


void
init()
{
	for (int i = 0; i < 3; i++) {
		for (int l = 0; l < 3; l++) {
			for (int j = 0; j < 30; j++) {
				for (int k = 0; k < 30; k++) {
					if (i == 1 && l==1) {
						
					}
					else {
						GLfloat Rval = (GLfloat)rand() / (GLfloat)RAND_MAX;
						GLfloat x = i * 300 + j * 10;
						GLfloat y = l * 300 + k * 10;
						GLfloat f = 1 / (1 + ((x-300)*(x - 300) + (y-450)*(y - 450)) / (900));
						if (Rval <= f) {
							//build a house
							objectInWorld house;
							house.type = 1;
							house.rotateT = (GLfloat)rand() / (GLfloat)RAND_MAX * 360;
							house.hightT = 3 + 7 * (GLfloat)rand() / (GLfloat)RAND_MAX;
							house.lengthT = 3 + 5 * (GLfloat)rand() / (GLfloat)RAND_MAX;
							house.widthT = 3 + 5 * (GLfloat)rand() / (GLfloat)RAND_MAX;
							house.x = x;
							house.y = y;
							allObj.push_back(house);
						}
						else {
							GLfloat Rval = (GLfloat)rand() / (GLfloat)RAND_MAX;
							if (Rval < 1. / 5) {
								objectInWorld tree;
								tree.type = 2;
								tree.rotateT = (GLfloat)rand() / (GLfloat)RAND_MAX * 360;
								tree.hightT = 6 + 7 * (GLfloat)rand() / (GLfloat)RAND_MAX;
								tree.lengthT = 1 + 3 * (GLfloat)rand() / (GLfloat)RAND_MAX;
								tree.widthT = tree.lengthT;
								tree.x = x;
								tree.y = y;
								allObj.push_back(tree);
							}
						}
					}
				}
			}
		}
	}
	srand(std::time(NULL));
	glGenBuffers(6, buffer);
	
	{//mountin
		//produced via forier interpolation
		glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
		//fine^2 quads=fine^2*6 points
		int n = 0;
		point4 points[fine*fine * 6*9];
		point4 colors[fine*fine * 6*9];
		GLfloat vertex[fine*3 + 1][fine*3 + 1];
		for (int i = 0; i < fine * 3 + 1; i++) {
			for (int j = 0; j < fine * 3 + 1; j++) {
				vertex[i][j] = 0;
			}
		}
		
		GLfloat con[numFunc][numFunc];
		for (int i = 0; i < numFunc; i++) {
			for (int j = 0; j <= i; j++) {
				con[i - j][j] = ((GLfloat)rand() / (GLfloat)RAND_MAX*2-1) / ((i + 3)*(i + 3));
			}
		}
		for (int i = 0; i <= fine; i++) {
			for (int j = 0; j <= fine; j++) {
				GLfloat x = i / (GLfloat)fine;
				GLfloat y = j / (GLfloat)fine;
				GLfloat temp = sin(M_PI*x)*sin(M_PI*y);
				for (int k = 0; k < numFunc; k++) {
					for (int l = 0; l <= k; l++) {
						temp += con[k - l][l]*sin(M_PI*(3 + 2 * (k - l))*x)*sin(M_PI*(3 + 2 * (l))*y);
					}
				}
				vertex[i+fine][j+fine] = std::sqrt(std::abs(temp))-.5;
				if (vertex[i+fine][j+fine] < 0) {
					vertex[i+fine][j+fine] = 0;
				}
			}
		}
		point4 vertexcolor[fine * 3 + 1][fine * 3 + 1];
		for (int i = 0; i < fine * 3 + 1; i++) {
			for (int j = 0; j < fine * 3 + 1; j++) {
				vertexcolor[i][j] = getColor(vertex[i][j]);
			}
		}
		for (int i = 0; i < fine*3; i++) {
			for (int j = 0; j < fine*3; j++) {
				GLfloat x = i / (GLfloat)fine*3;
				GLfloat y = j / (GLfloat)fine*3;
				GLfloat dx = 1 / (GLfloat)fine*3;
				GLfloat dy = 1 / (GLfloat)fine*3;
				
				colors[n] = vertexcolor[i][j];
				points[n] = point4(x, y, vertex[i][j], 1); n++;
				colors[n] = vertexcolor[i + 1][j];
				points[n] = point4(x+dx, y, vertex[i+1][j], 1); n++;
				colors[n] = vertexcolor[i + 1][j + 1];
				points[n] = point4(x+dx, y+dy, vertex[i + 1][j+1], 1); n++;
				colors[n] = vertexcolor[i + 1][j + 1];
				points[n] = point4(x + dx, y + dy, vertex[i + 1][j + 1], 1); n++;
				colors[n] = vertexcolor[i][j + 1];
				points[n] = point4(x, y + dy, vertex[i][j + 1], 1); n++;
				colors[n] = vertexcolor[i][j];
				points[n] = point4(x, y, vertex[i][j], 1); n++;
			}
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
		bufferlength[0] = fine*fine * 6 * 9;
		glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
		bufferlength[1] = fine*fine * 6 * 9;
	}
	{//house
		node* houseP = house();
		CopyToBuffer(houseP, 2, 3);
		destroyNode(houseP);
	}
	{//tree
		node* treeP = tree();
		CopyToBuffer(treeP, 4, 5);
		destroyNode(treeP);
	}


   // Load shaders and use the resulting shader program
    program = InitShader( "vshaderCC_v150.glsl", "fshaderCC_v150.glsl" );
    glUseProgram( program );
	vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	vcolor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vcolor);

	color_loc = glGetUniformLocation(program, "Color");
	bvColor= glGetUniformLocation(program, "bvColor");
	bColor = glGetUniformLocation(program, "bColor");
	proj_loc       = glGetUniformLocation(program, "projection");
	model_view_loc = glGetUniformLocation(program, "modelview");

   
    glClearColor( 0,0,0, 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
}

//----------------------------------------------------------------------------


void mountan(mat4 pos) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glVertexAttribPointer(vcolor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, pos);
	glDrawArrays(GL_TRIANGLES, 0, bufferlength[1]);
}

void house(mat4 pos) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, buffer[3]);
	glVertexAttribPointer(vcolor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, pos);
	glDrawArrays(GL_TRIANGLES, 0, bufferlength[2]);
}

void tree(mat4 pos) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer[4]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, buffer[5]);
	glVertexAttribPointer(vcolor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, pos);
	glDrawArrays(GL_TRIANGLES, 0, bufferlength[4]);
}

void seane() {
	for (int i = 0; i < allObj.size();i++) {
		if (allObj[i].type == 1) {
			house(Translate(allObj[i].x, allObj[i].y, 0)*RotateZ(allObj[i].rotateT)*Scale(allObj[i].lengthT, allObj[i].widthT, allObj[i].hightT)*Translate(-.5, -.5, 0));
		}
		if (allObj[i].type == 2) {
			tree(Translate(allObj[i].x, allObj[i].y, 0)*RotateZ(allObj[i].rotateT)*Scale(allObj[i].lengthT, allObj[i].widthT, allObj[i].hightT)*Translate(-.5, -.5, 0));
		}
	}
}

//put the cannon on the screen
void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    projmat=Angel::mat4(1.0); //Identity matrix
	//Position and orient the camera
	//Set up the camera optics
	projmat = projmat* Frustum(-.1*widthInPix/512., .1*widthInPix / 512., -.1*hightInPix / 512., .1*hightInPix / 512., .5, 1000000)*modelviewStackTop;
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniform4fv(color_loc, 1, Angel::vec4(1, 0, 0, 1.0));
	glUniform4fv(bColor, 1, vec4(0, 0, 0, 0));
	glUniform4fv(bvColor, 1, vec4(1, 1, 1, 1));
	mountan(Scale(900,900,3000)*Scale(1./9, 1. / 9, 1. / 9));
	seane();
	
	tree(Translate(900, 900, 0)*Scale(4, 4, 15));
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
		modelviewStackTop = RotateZ(step*10)*modelviewStackTop;
		break;
	case 'a':
		modelviewStackTop = RotateZ(-step*10)*modelviewStackTop;
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
