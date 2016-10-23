/*
Parker Whaley
This code illustrates three dementional brownian motion
*/
#include <ctime>
#include "Angel.h"
void m_glewInitAndVersion(void);
void reshape(int width, int height);

GLuint program; 

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint buffer[5];

//defines how many sides the prisim shuld have
#define numOfSides 9
const int NumVertices = 3*(2*numOfSides+ 2*numOfSides); //  2 triangles pre face + number of sides triangles for the top
//disp is the displacement distance for the brownian motion
GLfloat disp = sqrt(sin(2 * M_PI / numOfSides)*sin(2 * M_PI / numOfSides) + (1 - cos(2 * M_PI / numOfSides))*(1 - cos(2 * M_PI / numOfSides))) / 50.0;

point4 points[NumVertices];

GLfloat theta = 0;
GLfloat zoom = 1.0;
GLfloat step = .5;
GLfloat theta_y = 0;
int spinToggle = 0; 
GLfloat d=2.5;
int w; int h;

bool bComplete = false;
//these describe the face to change color
bool makeBlack = false;
int sideToChange = 0;

// Vertices of a unit cube centered at origin, sides aligned with axes
point4 vertices[numOfSides * 2 + 2];





//uniform variable locations
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;


//make sure you start with the default coordinate system
mat4 projmat = Angel::mat4(1.0);
mat4 modelviewStackTop = Angel::mat4(1.0);


//----------------------------------------------------------------------------

#define PI 3.14159265

//a linked list to hold the locations of the particle
struct pointNode
{
	GLfloat x;
	GLfloat y;
	GLfloat z;

	pointNode *next;
};

//how many nodes are in a linked list
int pointCount(struct pointNode* head)
{
	pointNode* tmp;
	tmp = head;
	int count_l = 0;

	while (tmp != NULL)
	{
		count_l++;
		tmp = tmp->next;
	}
	return count_l;
}

//takes a magnitude and the sides by reference and randomizes the direction while setting the magnitude to the magnitude specified
void randomDisplacement(GLfloat magnitude, GLfloat &side1, GLfloat &side2, GLfloat &side3)
{
	GLfloat angle1 = ((GLfloat)rand() / (GLfloat)RAND_MAX) * (2 * PI);
	GLfloat angle2 = ((GLfloat)rand() / (GLfloat)RAND_MAX) * (2 * PI);
	side1 = magnitude * cos(angle1)*sin(angle2);
	side2 = magnitude * sin(angle1)*sin(angle2);
	side3 = magnitude *cos(angle2);
}

//generates a random start point
pointNode* getRandomStart() {
	pointNode* retVal = new pointNode;
	GLfloat randang = ((GLfloat)rand() / (GLfloat)RAND_MAX) * (2 * PI);
	GLfloat randR = ((GLfloat)rand() / (GLfloat)RAND_MAX) * .5*cos(M_PI/numOfSides);
	retVal->x = randR*sin(randang);
	retVal->y = randR*cos(randang);
	retVal->z = ((GLfloat)rand() / (GLfloat)RAND_MAX) - .5;
	retVal->next = NULL;
	return retVal;
}

//adds a random node to the linked list of nodes
pointNode* AddNode(pointNode* node)
{
	pointNode* newNode = new pointNode;
	while (node->next != NULL)
	{
		node = node->next;
	}
	randomDisplacement(disp, newNode->x, newNode->y, newNode->z);
	newNode->x += node->x;
	newNode->y += node->y;
	newNode->z += node->z;
	newNode->next = NULL;
	node->next = newNode;

	return newNode;
}

//checks if the last node has left the prisim
bool checkNode(struct pointNode * curr) {
	if (curr->z > .5) {
		bComplete = true;
		makeBlack = true;
		sideToChange = numOfSides;
		return false;
	}
	if (curr->z < -.5) {
		bComplete = true;
		makeBlack = true;
		sideToChange = numOfSides+1;
		return false;
	}
	for (int i = 0; i < numOfSides; i++) {
		double angstep = 2 * M_PI / (double) numOfSides;
		double rotang = .5*angstep + i*angstep;
		double yrot = curr->y*cos(rotang)+curr->x*sin(rotang);
		if (yrot>.5*cos(.5*angstep)) {
			//we have left the shape
			bComplete = true;
			makeBlack = true;
			sideToChange = i;
			return false;
		}
	}
	return true;
}


pointNode* head;

//generates a array with the data in the linked list in it
GLfloat* copyToArray(struct pointNode * head)
{
	GLfloat * retVal;
	pointNode * tmp;
	int count_l;
	count_l = pointCount(head);
	int i = 0;

	count_l *= 3;

	tmp = head;
	if (count_l > 0)
	{

		retVal = new GLfloat[count_l];
	}
	else
	{
		return NULL;
	}

	while (i < count_l)
	{
		retVal[i] = tmp->x;
		retVal[i + 1] = tmp->y;
		retVal[i + 2] = tmp->z;
		tmp = tmp->next;
		i += 3;
	}
	return retVal;
}










int Index = 0;

// quad generates two triangles for each face
void
quad( int a, int b, int c, int d )
{
	points[Index] = vertices[a]; Index++;
	points[Index] = vertices[b]; Index++;
	points[Index] = vertices[c]; Index++;

	points[Index] = vertices[c]; Index++;
	points[Index] = vertices[d]; Index++;
	points[Index] = vertices[a]; Index++;
}

//same as quad but for three vertesis
void triangle(int a, int b, int c) {
	points[Index] = vertices[a]; Index++;
	points[Index] = vertices[b]; Index++;
	points[Index] = vertices[c]; Index++;
}


//makes a cube in a passed buffer
void makeCubeSetup(GLfloat sideL,GLuint& buff) {
	vec4 side[6];
	side[0] = vec4(sideL / 2, sideL / 2, sideL / 2, 1);
	side[1] = vec4(sideL / 2, sideL / 2, -sideL / 2, 1);
	side[2] = vec4(sideL / 2, -sideL / 2, -sideL / 2, 1);
	side[3] = side[2];
	side[4] = vec4(sideL / 2, -sideL / 2, sideL / 2, 1);
	side[5] = side[0];
	mat4 transforms[6];
	for (int i = 0; i < 6; i++) {
		transforms[i] = mat4(1);
	}
	transforms[1] = RotateZ(90)* transforms[0];
	transforms[2] = RotateZ(90)* transforms[1];
	transforms[3] = RotateZ(90)* transforms[2];
	transforms[4] = RotateY(90)* transforms[0];
	transforms[5] = RotateY(-90)* transforms[0];

	glBindBuffer(GL_ARRAY_BUFFER, buffer[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*(6 * 6), NULL, GL_STATIC_DRAW);
	vec4 tempside[6];
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 6; j++) {
			tempside[j] = transforms[i] * side[j];
		}
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * 6 * i, sizeof(vec4) * 6, tempside);
	}
}
//----------------------------------------------------------------------------

// fenerate the prisim using the eadges set up before
void prisim() {
	Index = 0;
	for (int i = 0; i < numOfSides - 1; i++) {
		int offset = 2 * i;
		quad(offset, offset + 1, offset + 3, offset + 2);
	}
	quad(2 * (numOfSides - 1), 2 * (numOfSides - 1) + 1, 1, 0);
	//now do top
	for (int i = 0; i < numOfSides - 1; i++) {
		int offset = 2 * i;
		triangle(offset, 2 * numOfSides, offset + 2);
	}
	triangle((numOfSides-1)*2, 2 * numOfSides, 0);
	//now do bottom
	for (int i = 0; i < numOfSides - 1; i++) {
		int offset = 2 * i;
		triangle(offset+1, 2 * numOfSides+1, offset + 3);
	}
	triangle((numOfSides - 1) * 2+1, 2 * numOfSides+1, 1);
}

//----------------------------------------------------------------------------

GLuint vPosition;
// OpenGL initialization
void
init()
{

	//randomize the start position
	head = getRandomStart();

	//set up the vertesis we will need
	for (int i = 0; i < numOfSides * 2; i+=2) {
		GLfloat angle = 2 * M_PI*(i / 2) / ((GLfloat)numOfSides);
		vertices[i]= point4(.5*sin(angle), .5*cos(angle), 0.5, 1.0);
		vertices[i + 1] = point4(.5*sin(angle), .5*cos(angle), -0.5, 1.0);
	}
	vertices[numOfSides * 2] = point4(0, 0, 0.5, 1.0);
	vertices[numOfSides * 2+1] = point4(0, 0, -0.5, 1.0);
    prisim(); //create the shape

    // Create and initialize a buffer object
    glGenBuffers( 5, buffer);

	

	glBindBuffer( GL_ARRAY_BUFFER, buffer[0]);
	//store the prisim in buffer 0
    glBufferData( GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW );

	//generate a prisim slightly furthur out than the original prisim and put it in buffer 1
	for (int i = 0; i < numOfSides * 2 + 2; i++) {
		double epsilon = 1.001;
		vertices[i].x *= epsilon;
		vertices[i].y *= epsilon;
		vertices[i].z *= epsilon;
	}
	Index = 0;
	prisim();
	glBindBuffer( GL_ARRAY_BUFFER, buffer[1]);
    glBufferData( GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW );

   // Load shaders and use the resulting shader program
    program = InitShader( "vshaderCC_v150.glsl", "fshaderCC_v150.glsl" );
    glUseProgram( program );
	vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	color_loc = glGetUniformLocation(program, "vColor");
	proj_loc       = glGetUniformLocation(program, "projection");
	model_view_loc = glGetUniformLocation(program, "modelview");

   
    glClearColor( 1.0, 1.0, 1.0, 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
	//put a cube in buffer 3
	makeCubeSetup(.01, buffer[3]);
}

//----------------------------------------------------------------------------
//makes a square
void dispSquare() {
	glBindBuffer(GL_ARRAY_BUFFER, buffer[3]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glDrawArrays(GL_TRIANGLES, 0, 6 * 6);
}

//draw the sene
void
display( void )
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    projmat=Angel::mat4(1.0); //Identity matrix
	//Position and orient the camera
	mat4 modelviewStackToptemp = Translate(0.0, 0.0, -d)*modelviewStackTop;
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackToptemp);
	//Set up the camera optics
	projmat = projmat*Perspective(zoom * 30, 1.0, 1.0, 10.0);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);

	pointNode* end = head;
	while (end->next != nullptr) {
		end = end->next;
	}
	GLfloat * trajectoryBuffer = copyToArray(head);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * pointCount(head), trajectoryBuffer, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUniform4fv(color_loc, 1, Angel::vec4(0.0, 0.0, .5, 1.0));
	glDrawArrays(GL_LINE_STRIP, 0, pointCount(head));
	delete[] trajectoryBuffer;

	mat4 modelviewStackToptemp2 = modelviewStackToptemp*Translate(head->x, head->y, head->z);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackToptemp2);
	glUniform4fv(color_loc, 1, Angel::vec4(0.0, 0.0, 1, 1.0));
	dispSquare();
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackToptemp);



	
	pointNode* tail = head;
	while (tail->next != nullptr) {
		tail = tail->next;
	}
	mat4 modelviewStackToptemp3 = modelviewStackToptemp*Translate(tail->x, tail->y, tail->z);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackToptemp3);
	if (bComplete) {
		glUniform4fv(color_loc, 1, Angel::vec4(0.0, 1, 1, 1.0));
	}
	else {
		glUniform4fv(color_loc, 1, Angel::vec4(1,0,0,1));
	}
	dispSquare();
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackToptemp);


	
	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glUniform4fv(color_loc, 1, point4(0, 0, 0, 1));
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	if (makeBlack) {
		if (sideToChange < numOfSides) {
			glDrawArrays(GL_TRIANGLES, sideToChange * 6, 6);
		}
		else {
			glDrawArrays(GL_TRIANGLES, (sideToChange- numOfSides)*numOfSides*3+ numOfSides * 6, numOfSides*3);
		}
	}
	glDrawArrays(GL_LINE_STRIP, 0, NumVertices/2);



	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glUniform4fv(color_loc, 1, point4(0, 1, 0, .1));
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glDrawArrays( GL_TRIANGLES, 0, NumVertices);



    glutSwapBuffers();
}

bool bPaused = true;
//bassically the same animate function as assignment 1
void animate(int i)
{
	GLfloat x, y;
	pointNode * last;
	if (!bPaused && !bComplete)
	{
		last = head;
		while (last->next != nullptr) {
			last = last->next;
		}
		last = AddNode(last);
		int count_l;
		count_l = pointCount(head);

		
		//We only want to keep going if 
		if (checkNode(last))
		{
			//keep a roughly constat fps
			glutTimerFunc(17, animate, 0);
		}
		else
		{
			bComplete = true;
		}
		glutPostRedisplay();
	}
}



//----------------------------------------------------------------------------
//handel keybord imput
void
keyboard( unsigned char key, int x, int y )
{
    switch( key ) {

	case 'Z':{	zoom *= 0.95; }
	break;

	case 'z':{	zoom *= 1.05; }
	break;

	case 's':
	case 'S':
		if (head->next == nullptr) {
			bPaused = !bPaused;
			animate(0);
		}
		break;
	case 'i':
	case 'I':
		if (head->next != nullptr) {
			bPaused = !bPaused;
			animate(0);
		}
		break;

	case 033:  // Escape key
	case 'q': case 'Q':
	    exit( EXIT_SUCCESS );
    }
}

//keep the screen updated
void idle(void){

//Spin the wire or pause the spinning
theta_y += step*spinToggle; 
if (theta_y > 360.0) {
		theta_y -= 360.0;
		}
glutPostRedisplay();
}
//----------------------------------------------------------------------------
//lets the program tell how far the user moved there mouse with the left button down
bool clicked = false;
int ixclick = 0;
int iyclick = 0;

//records the initial click
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

//updates the orientation of the sene
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
	glutTimerFunc(1000, animate, 0);
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
