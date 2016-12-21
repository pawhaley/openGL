/*
Parker Whaley
A4
*/
#include <ctime>
#include <vector>
#include "Angel.h"
void m_glewInitAndVersion(void);
void reshape(int width, int height);

#define fine 6
#define numFunc 20
int lightSenario = 0;

const GLfloat rotAng = M_PI * 2 / fine;

GLuint program; 

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

GLuint buffer[9]; //Buffer Object to store the cube vertex attributes
int bufferlength[9];

GLfloat zoom = 1.0;
GLfloat step = .1;
GLfloat d=2.5;
GLfloat transdiff = .1;
int widthInPix;
int hightInPix;
double lightRotation = 0;
double rotAngLight = M_PI / 100;
color4 brown =  color4(1, .829, .609, 1);
int toDestroy = 0;

GLuint	 texBufferID[3];
GLubyte* imageData;


//uniform variable locations
GLuint color_loc;
GLuint proj_loc;
GLuint model_view_loc;
GLuint  AmbientProduct, DiffuseProduct, SpecularProduct, LightPosition1, LightPosition2, LightPosition3, Shininess, ModelWorld, inFrame, TexCoord, texID;


//make sure you start with the default coordinate system
mat4 projmat = Angel::mat4(1.0);
mat4 modelviewStackTop = Translate(0.0, 0.0, 0.0);


//solve a matrix
vec4 solve(mat4 A, vec4 b) {
	for (int i = 0; i < 4; i++) {
		GLfloat maxa = 0;
		GLfloat row = -1;
		for (int j = i; j < 4; j++) {
			if (abs(A[j][i]) > maxa) {
				maxa = abs(A[j][i]);
				row = j;
			}
		}
		GLfloat temp = b[i];
		b[i] = b[row];
		b[row] = temp;
		vec4 tempvec = A[i];
		A[i] = A[row];
		A[row] = tempvec;
		b[i] /= A[i][i];
		A[i] /= A[i][i];
		for (int j = i + 1; j < 4; j++) {
			b[j] -= b[i] * A[j][i];
			A[j] -= A[i] * A[j][i];
		}
	}
	//we now have a diagonal matrix with 1's on the diagonal
	for (int i = 3; i > 0; i--) {
		for (int j = 0; j < i; j++) {
			b[j] -= A[j][i] * b[i];
		}
	}
	return b;
}





//----------------------------------------------------------------------------
//GLuint vColor;
GLuint vPosition;
GLuint vNormal;
// OpenGL initialization


struct node {
	vec4 data;
	color4 color;
	point4 normal;
	vec2 tex;
	node* next;
};

node* pushBack(node* add, vec4 val,color4 color,point4 normal,vec2 tex) {
	node* valnode = new node;
	valnode->data = val;
	valnode->color = color;
	valnode->normal = normal;
	valnode->tex = tex;
	valnode->next = add->next;
	add->next = valnode;
	return valnode;
}

node* setup(vec4 in,color4 color, point4 normal,vec2 tex) {
	node* valnode = new node;
	valnode->next = nullptr;
	valnode->data = in;
	valnode->color = color;
	valnode->normal = normal;
	valnode->tex = tex;
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
		//list->normal = matrix*list->normal;
		list = list->next;
	}
	return retval;
}

node* copy(node* fromMe) {
	node* retval=setup(fromMe->data, fromMe->color, fromMe->normal,fromMe->tex);
	node* top=retval;
	while (fromMe->next != nullptr) {
		fromMe = fromMe->next;
		top = pushBack(top, fromMe->data, fromMe->color, fromMe->normal,fromMe->tex);
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

void CopyToBuffer(node* top, int buff1, int buff2, int buff3,int buff4) {
	int length = 0;
	node* temp = top;
	while (temp != nullptr) {
		length++;
		temp = temp->next;
	}
	bufferlength[buff1] = length;
	bufferlength[buff2] = length;
	bufferlength[buff3] = length;
	bufferlength[buff4] = length;
	vec4* points = new vec4[length];
	color4* colors = new color4[length];
	point4* normals = new point4[length];
	vec2* texs = new vec2[length];
	temp = top;
	int n = 0;
	while (temp != nullptr) {
		points[n] = temp->data;
		colors[n] = temp->color;
		normals[n] = temp->normal;
		texs[n] = temp->tex;
		n++;
		temp = temp->next;
	}
	glBindBuffer(GL_ARRAY_BUFFER, buffer[buff1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*length, points, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[buff2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*length, colors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[buff3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*length, normals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[buff4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*length, texs, GL_STATIC_DRAW);
	delete[] points;
	delete[] colors;
	delete[] normals;
	delete[] texs;
}

void addOnPoint(point4 p, color4 color, node* wheretoadd) {
	point4 normal;
	normal = point4(p.x, p.y, p.z, 0);
	vec2 point = vec2(p.x,p.y)/sqrt(p.x*p.x+p.y*p.y);
	GLfloat ang = acos(point.x);
	if (point.y < 0) {
		ang = 2 * M_PI - ang;
	}
	pushBack(wheretoadd, p, color, normal, vec2(p.z/2+1,ang/(2*M_PI)));
}

void devideTryforsphere(int n, point4 p1, point4 p2, point4 p3, color4 color,node* wheretoadd) {
	point4 normal;
	if (n == 0) {
		addOnPoint(p1, color, wheretoadd);
		addOnPoint(p2, color, wheretoadd);
		addOnPoint(p3, color, wheretoadd);
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
	node* wheretoadd = setup(vec4(0, 0, 0, 0), color, vec4(0, 0, 0, 0),vec2(0,0));
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

vec2 texTransform(double theta, double phi) {
	return vec2(theta / (2 * M_PI), phi / (2 * M_PI));
}

void pushBackBasedOnTP(color4 color,double theta, double phi, node* retval) {
	pushBack(retval, cordTransformPos(theta, phi), color, cordTransformNorm(theta, phi), texTransform(theta,phi));
}

node* toris(color4 color) {
	node* retval = setup(vec4(0, 0, 0, 0), color, vec4(0, 0, 0, 0),vec2(0,0));
	for (int i = 0; i < fine * 5; i++) {
		double theta1 = 2 * M_PI*i / (fine * 5);
		double theta2 = 2 * M_PI*(i+1) / (fine * 5);
		for (int j = 0; j < fine * 5; j++) {
			double phi1 = 2 * M_PI*j / (fine * 5);
			double phi2 = 2 * M_PI*(j+1) / (fine * 5);
			pushBackBasedOnTP(color, theta1, phi1, retval);
			pushBackBasedOnTP(color, theta2, phi1, retval);
			pushBackBasedOnTP(color, theta2, phi2, retval);

			pushBackBasedOnTP(color, theta2, phi2, retval);
			pushBackBasedOnTP(color, theta1, phi2, retval);
			pushBackBasedOnTP(color, theta1, phi1, retval);
		}
	}
	node* temp = retval;
	retval = retval->next;
	temp->next = nullptr;
	delete temp;
	return retval;
}

node* cilender(color4 color) {
	node* retval = setup(vec4(0, 0, 0, 0), color, vec4(0, 0, 0, 0), vec2(0, 0));
	
	for (int i = 0; i < fine * 5; i++) {
		GLfloat ang1 = i * 2 * M_PI / (fine * 5);
		GLfloat ang2 = (i+1) * 2 * M_PI / (fine * 5);
		vec4 xyv1 = vec4(cos(ang1), sin(ang1), 0, 0);
		vec4 xyv2 = vec4(cos(ang2), sin(ang2), 0, 0);
		pushBack(retval, vec4(0, 0, 1, 1), color, vec4(0, 0, 1, 0), vec2(.5, .5));
		pushBack(retval, vec4(0, 0, 1, 1)+ xyv1, color, vec4(0, 0, 1, 0), vec2(xyv1.x,xyv1.y)/2.+.5);
		pushBack(retval, vec4(0, 0, 1, 1) + xyv2, color, vec4(0, 0, 1, 0), vec2(xyv2.x, xyv2.y) / 2. + .5);

		pushBack(retval, vec4(0, 0, 1, 1) + xyv1, color, xyv1, vec2(.6, (ang1) / (2 * M_PI)));
		pushBack(retval, vec4(0, 0, 1, 1) + xyv2, color, xyv2, vec2(.6, (ang2) / (2 * M_PI)));
		pushBack(retval, vec4(0, 0, 0, 1) + xyv2, color, xyv2, vec2(.3, (ang2) / (2 * M_PI)));

		pushBack(retval, vec4(0, 0, 0, 1) + xyv2, color, xyv2, vec2(.3, (ang2) / (2 * M_PI)));
		pushBack(retval, vec4(0, 0, 0, 1) + xyv1, color, xyv1, vec2(.3, (ang1) / (2 * M_PI)));
		pushBack(retval, vec4(0, 0, 1, 1) + xyv1, color, xyv1, vec2(.6, (ang1) / (2 * M_PI)));

		pushBack(retval, vec4(0, 0, 0, 1), color, vec4(0, 0, -1, 0), vec2(.5, .5));
		pushBack(retval, vec4(0, 0, 0, 1) + xyv1, color, vec4(0, 0, -1, 0), vec2(xyv1.x, xyv1.y) / 2. + .5);
		pushBack(retval, vec4(0, 0, 0, 1) + xyv2, color, vec4(0, 0, -1, 0), vec2(xyv2.x, xyv2.y) / 2. + .5);
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
	int bmpWidth = -1;
	int bmpHeight = -1;
	int bmpSize = -1;
	Angel::loadBitmapFromFile("gold.bmp", &bmpWidth, &bmpHeight, &bmpSize, (unsigned char**)&imageData);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(3, texBufferID);
	glBindTexture(GL_TEXTURE_2D, texBufferID[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmpWidth, bmpHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);
	Angel::loadBitmapFromFile("RedBricks.bmp", &bmpWidth, &bmpHeight, &bmpSize, (unsigned char**)&imageData);
	glBindTexture(GL_TEXTURE_2D, texBufferID[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmpWidth, bmpHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);
	Angel::loadBitmapFromFile("silver.bmp", &bmpWidth, &bmpHeight, &bmpSize, (unsigned char**)&imageData);
	glBindTexture(GL_TEXTURE_2D, texBufferID[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmpWidth, bmpHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);
	

	srand(std::time(NULL));
	glGenBuffers(9, buffer);
	
	{//sphere
		node* sphereP = sphere(color4(0,0,0,0));
		CopyToBuffer(sphereP, 0, 1,1,2);//ignore the color buff
		destroyNode(sphereP);
	}

	{//tor
		node* torP = toris(color4(0, 0, 0, 0));
		CopyToBuffer(torP, 3, 4, 4,5);//ignore the color buff
		destroyNode(torP);
	}

	{//cilender
		node* cilP = cilender(color4(0, 0, 0, 0));
		CopyToBuffer(cilP, 6, 7, 7, 8);//ignore the color buff
		destroyNode(cilP);
	}

   // Load shaders and use the resulting shader program
    program = InitShader( "vshaderCC_v150.glsl", "fshaderCC_v150.glsl" );
    glUseProgram( program );
	vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	TexCoord= glGetAttribLocation(program, "s_vTexCoord");
	glEnableVertexAttribArray(TexCoord);
	texID = glGetUniformLocation(program, "texture");
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(texID, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	

	proj_loc       = glGetUniformLocation(program, "Projection");
	model_view_loc = glGetUniformLocation(program, "ModelView");
	ModelWorld = glGetUniformLocation(program, "ModelWorld");

	AmbientProduct = glGetUniformLocation(program, "AmbientProduct");
	DiffuseProduct = glGetUniformLocation(program, "DiffuseProduct");
	SpecularProduct = glGetUniformLocation(program, "SpecularProduct");
	LightPosition1 = glGetUniformLocation(program, "LightPosition1");
	LightPosition2 = glGetUniformLocation(program, "LightPosition2");
	LightPosition3 = glGetUniformLocation(program, "LightPosition3");
	Shininess = glGetUniformLocation(program, "Shininess");


	

	glUniform1f(Shininess,15);
	glUniform4fv(AmbientProduct,1, color4(0.2, 0.2, 0.2, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.5,.5,.5, 1.0));
	glUniform4fv(SpecularProduct, 1, color4(.8,.8,.8, 1.0));
   
    glClearColor( 0,0,0, 1.0 ); 
	glEnable( GL_DEPTH_TEST );
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);
}

//----------------------------------------------------------------------------




void sphere(mat4 pos) {
	glBindTexture(GL_TEXTURE_2D, texBufferID[0]);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(texID, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glUniformMatrix4fv(ModelWorld, 1, GL_TRUE, pos);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
	glVertexAttribPointer(TexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	for (int i = 0; i < bufferlength[0]; i += 3) {
		glDrawArrays(GL_TRIANGLES, i, 3);
	}
}

void tor(mat4 pos) {
	glBindTexture(GL_TEXTURE_2D, texBufferID[2]);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(texID, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[3]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glUniformMatrix4fv(ModelWorld, 1, GL_TRUE, pos);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[5]);
	glVertexAttribPointer(TexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, buffer[4]);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	for (int i = 0; i < bufferlength[3]; i += 3) {
		glDrawArrays(GL_TRIANGLES, i, 3);
	}
}

void cil(mat4 pos) {
	glBindTexture(GL_TEXTURE_2D, texBufferID[1]);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(texID, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[6]);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glUniformMatrix4fv(ModelWorld, 1, GL_TRUE, pos);

	glBindBuffer(GL_ARRAY_BUFFER, buffer[8]);
	glVertexAttribPointer(TexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glBindBuffer(GL_ARRAY_BUFFER, buffer[7]);
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	for (int i = 0; i <bufferlength[8]; i += 3) {
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
	
	point4 tempLight[3] = { point4(0, 15, 15, 0) ,point4(15 * sin(lightRotation), 15, -30 + 15 * cos(lightRotation), 1) ,point4(0, 0, 0, 1) };
	//glUniform4fv(LightPosition, 1, point4(0, 15, 15, 0));
	glUniform4fv(LightPosition1, 1, tempLight[0]);
	glUniform4fv(LightPosition2, 1, tempLight[1]);
	glUniform4fv(LightPosition3, 1, tempLight[2]);

	//glUniform1f(inFrame, false);
	lightRotation += rotAngLight;
	//glUniform4fv(LightPosition, 1, point4(15 * sin(lightRotation), 15, -30 + 15 * cos(lightRotation), 1));


	//glUniform1f(inFrame, true);
	//glUniform4fv(LightPosition, 1, point4(0, 0, 0, 1));


	glUniform4fv(SpecularProduct, 1, color4(.8, .8, .8, 1.0));
	glUniform4fv(AmbientProduct, 1, color4(0.05, 0.05, 0.05, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.5, .5, .5, 1.0)*color4(1, 1, 1, 1.0));
	glUniform1f(Shininess, 25);
	tor(Translate(-5, 0, -30)*RotateY(-45));


	sphere(Translate(2, 2, -33));

	glUniform4fv(AmbientProduct, 1, color4(0.3, 0.3, 0.3, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.3, .3, .3, 1.0));
	glUniform1f(Shininess, 2);
	cil(Translate(0, 0, -33)*RotateY(90));



    glutSwapBuffers();
}


void
display0(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	projmat = Angel::mat4(1.0); //Identity matrix
								//Position and orient the camera
								//Set up the camera optics
	projmat = projmat* Frustum(-.1*widthInPix / 512., .1*widthInPix / 512., -.1*hightInPix / 512., .1*hightInPix / 512., .5, 1000000);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);

	point4 tempLight[3] = { point4(0, 15, 15, 0) ,point4(15 * sin(lightRotation), 15, -30 + 15 * cos(lightRotation), 1) ,point4(0, 0, 0, 1) };
	//glUniform4fv(LightPosition, 1, point4(0, 15, 15, 0));
	glUniform4fv(LightPosition1, 1, tempLight[0]);
	glUniform4fv(LightPosition2, 1, tempLight[1]);
	glUniform4fv(LightPosition3, 1, tempLight[2]);

	//glUniform1f(inFrame, false);
	lightRotation += rotAngLight;
	//glUniform4fv(LightPosition, 1, point4(15 * sin(lightRotation), 15, -30 + 15 * cos(lightRotation), 1));


	//glUniform1f(inFrame, true);
	//glUniform4fv(LightPosition, 1, point4(0, 0, 0, 1));


	glUniform4fv(SpecularProduct, 1, color4(.8, .8, .8, 1.0));
	glUniform4fv(AmbientProduct, 1, color4(0.05, 0.05, 0.05, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.5, .5, .5, 1.0)*color4(1, 1, 1, 1.0));
	glUniform1f(Shininess, 25);
	//tor(Translate(-5, 0, -30)*RotateY(-45));


	sphere(Translate(2, 2, -33));

	glUniform4fv(AmbientProduct, 1, color4(0.3, 0.3, 0.3, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.3, .3, .3, 1.0));
	glUniform1f(Shininess, 2);
	//cil(Translate(0, 0, -33)*RotateY(90));



	glutSwapBuffers();
}

void
display1(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	projmat = Angel::mat4(1.0); //Identity matrix
								//Position and orient the camera
								//Set up the camera optics
	projmat = projmat* Frustum(-.1*widthInPix / 512., .1*widthInPix / 512., -.1*hightInPix / 512., .1*hightInPix / 512., .5, 1000000);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);

	point4 tempLight[3] = { point4(0, 15, 15, 0) ,point4(15 * sin(lightRotation), 15, -30 + 15 * cos(lightRotation), 1) ,point4(0, 0, 0, 1) };
	//glUniform4fv(LightPosition, 1, point4(0, 15, 15, 0));
	glUniform4fv(LightPosition1, 1, tempLight[0]);
	glUniform4fv(LightPosition2, 1, tempLight[1]);
	glUniform4fv(LightPosition3, 1, tempLight[2]);

	//glUniform1f(inFrame, false);
	lightRotation += rotAngLight;
	//glUniform4fv(LightPosition, 1, point4(15 * sin(lightRotation), 15, -30 + 15 * cos(lightRotation), 1));


	//glUniform1f(inFrame, true);
	//glUniform4fv(LightPosition, 1, point4(0, 0, 0, 1));


	glUniform4fv(SpecularProduct, 1, color4(.8, .8, .8, 1.0));
	glUniform4fv(AmbientProduct, 1, color4(0.05, 0.05, 0.05, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.5, .5, .5, 1.0)*color4(1, 1, 1, 1.0));
	glUniform1f(Shininess, 25);
	tor(Translate(-5, 0, -30)*RotateY(-45));


	//sphere(Translate(2, 2, -33));

	glUniform4fv(AmbientProduct, 1, color4(0.3, 0.3, 0.3, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.3, .3, .3, 1.0));
	glUniform1f(Shininess, 2);
	//cil(Translate(0, 0, -33)*RotateY(90));



	glutSwapBuffers();
}


void
display2(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	projmat = Angel::mat4(1.0); //Identity matrix
								//Position and orient the camera
								//Set up the camera optics
	projmat = projmat* Frustum(-.1*widthInPix / 512., .1*widthInPix / 512., -.1*hightInPix / 512., .1*hightInPix / 512., .5, 1000000);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);

	point4 tempLight[3] = { point4(0, 15, 15, 0) ,point4(15 * sin(lightRotation), 15, -30 + 15 * cos(lightRotation), 1) ,point4(0, 0, 0, 1) };
	//glUniform4fv(LightPosition, 1, point4(0, 15, 15, 0));
	glUniform4fv(LightPosition1, 1, tempLight[0]);
	glUniform4fv(LightPosition2, 1, tempLight[1]);
	glUniform4fv(LightPosition3, 1, tempLight[2]);

	//glUniform1f(inFrame, false);
	lightRotation += rotAngLight;
	//glUniform4fv(LightPosition, 1, point4(15 * sin(lightRotation), 15, -30 + 15 * cos(lightRotation), 1));


	//glUniform1f(inFrame, true);
	//glUniform4fv(LightPosition, 1, point4(0, 0, 0, 1));


	glUniform4fv(SpecularProduct, 1, color4(.8, .8, .8, 1.0));
	glUniform4fv(AmbientProduct, 1, color4(0.05, 0.05, 0.05, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.5, .5, .5, 1.0)*color4(1, 1, 1, 1.0));
	glUniform1f(Shininess, 25);
	//tor(Translate(-5, 0, -30)*RotateY(-45));


	//sphere(Translate(2, 2, -33));

	glUniform4fv(AmbientProduct, 1, color4(0.3, 0.3, 0.3, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.3, .3, .3, 1.0));
	glUniform1f(Shininess, 2);
	cil(Translate(0, 0, -33)*RotateY(90));



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
		exit( EXIT_SUCCESS );
	case 'Q':
		glutDestroyWindow(toDestroy);
    }
}


void idle(void){
	glutPostRedisplay();
}
//----------------------------------------------------------------------------

bool clicked = false;
int ixclick = 0;
int iyclick = 0;

vec3 IntersectPlane(vec3 point, vec3 normal, vec3 orgVec) {
	normal = normalize(normal);
	orgVec = normalize(orgVec);
	vec3 toPlane = normal*dot(point, normal);
	return orgVec*dot(toPlane, toPlane) / dot(toPlane, orgVec);
}

GLfloat DistSphere(vec3 vecFromOrg,mat4 sphereLoc) {
	vec4 temp = sphereLoc*vec4(0, 0, 0, 1);
	vec3 sphereCenter = vec3(temp.x, temp.y, temp.z);
	vec3 normal = normalize(-1 * sphereCenter);
	vec3 intersect = IntersectPlane(sphereCenter, normal, vecFromOrg);
	vec4 vecInSphere = solve(sphereLoc, vec4(intersect.x, intersect.y, intersect.z, 1)) - vec4(0, 0, 0, 1);
	if (dot(vecInSphere, vecInSphere) > 1) {
		return -1;
	}
	return dot(intersect, intersect); //returning the distance square to a point in the sphere
}

GLfloat DistTor(vec3 vecFromOrg, mat4 torLoc) {
	GLfloat retval = -1;
	for (int i = 0; i < 100; i++) {
		GLfloat ang = 360. / 100. * i;
		GLfloat temp = DistSphere(vecFromOrg, torLoc*RotateZ(ang)*Translate(.75, 0, 0)*Scale(.25, .25, .25));
		if (temp != -1) {
			if (retval == -1) {
				retval = temp;
			}
			else if(temp<retval) {
				retval = temp;
			}
		}
	}
	return retval;
}

GLfloat DistCil(vec3 vecFromOrg, mat4 cilLoc) {
	{//test the botom
		vec4 temp1 = cilLoc*vec4(0, 0, 0, 1);
		vec4 temp2 = cilLoc*vec4(0, 0, -1, 1);
		vec3 point = vec3(temp1.x, temp1.y, temp1.z);
		vec4 tempvec = temp2 - temp1;
		vec3 normal = normalize(vec3(tempvec.x, tempvec.y, tempvec.z));
		vec3 intersect = IntersectPlane(point, normal, vecFromOrg);//solved in world need to take it back to cil space
		vec4 vecInCil = solve(cilLoc, vec4(intersect.x, intersect.y, intersect.z, 1)) - vec4(0, 0, 0, 1);
		if (dot(vecInCil, vecInCil) < 1) {
			return dot(intersect, intersect);
		}
	}
	{//test the top
		vec4 temp1 = cilLoc*vec4(0, 0, 1, 1);
		vec4 temp2 = cilLoc*vec4(0, 0, 2, 1);
		vec3 point = vec3(temp1.x, temp1.y, temp1.z);
		vec4 tempvec = temp2 - temp1;
		vec3 normal = normalize(vec3(tempvec.x, tempvec.y, tempvec.z));
		vec3 intersect = IntersectPlane(point, normal, vecFromOrg);//solved in world need to take it back to cil space
		vec4 vecInCil = solve(cilLoc, vec4(intersect.x, intersect.y, intersect.z, 1)) - vec4(0, 0, 1, 1);
		if (dot(vecInCil, vecInCil) < 1) {
			return dot(intersect, intersect);
		}
	}
	{//test some planes
		for (int i = 0; i < 100; i++) {
			vec4 temp1 = cilLoc*vec4(0, 0, 0, 1);
			vec4 temp2 = cilLoc*vec4(sin(M_PI*i/50.), cos(M_PI*i / 50.), 0, 1);
			vec3 point = vec3(temp1.x, temp1.y, temp1.z);
			vec4 tempvec = temp2 - temp1;
			vec3 normal = normalize(vec3(tempvec.x, tempvec.y, tempvec.z));
			vec3 intersect = IntersectPlane(point, normal, vecFromOrg);//solved in world need to take it back to cil space
			vec4 PtInCil = solve(cilLoc, vec4(intersect.x, intersect.y, intersect.z, 1));
			if (PtInCil.z < 1 && PtInCil.z>0 && PtInCil.x*PtInCil.x + PtInCil.y*PtInCil.y < 1) {
				return dot(intersect, intersect);
			}
		}
	}
	return -1;
}

void mouse(int x, int y);

void mouseCall(int button, int state, int ix, int iy);

void SpinOff(int n) {
	{
		toDestroy = glutCreateWindow("object");


		m_glewInitAndVersion();


		init();

		glutReshapeFunc(reshape);
		switch (n)
		{
		case 0:
			std::cout << "here";
			glutDisplayFunc(display0);
			break;
		case 1:
			std::cout << "here";
			glutDisplayFunc(display1);
			break;
		case 2:
			std::cout << "here";
			glutDisplayFunc(display2);
			break;
		default:
			glutDisplayFunc(display);
			break;
		}


		glutKeyboardFunc(keyboard);
		glutMotionFunc(mouse);
		glutMouseFunc(mouseCall);

		glutIdleFunc(idle);
		glutMainLoop();
	}
}

void mouseCall(int button, int state, int ix, int iy) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		clicked = true;
		ixclick = ix;
		iyclick = iy;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		clicked = false;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		vec4 tempvec = solve(projmat, vec4(2*(ix * 2. / widthInPix - 1), 2*((hightInPix-iy) * 2. / hightInPix - 1), 1, 2));
		vec3 direc = normalize(vec3(tempvec.x, tempvec.y, tempvec.z));
		GLfloat test[3] = {
			DistSphere(direc, modelviewStackTop*Translate(2, 2, -33)),
			DistTor(direc, modelviewStackTop*Translate(-5, 0, -30)*RotateY(-45)),
			DistCil(direc, modelviewStackTop*Translate(0, 0, -33)*RotateY(90))
		};
		if (test[0] >= 0 || test[1] >= 0 || test[2] >= 0) {
			//we hit sompthing
			int n = -1;
			GLfloat mindist= -1;
			for (int i = 0; i < 3; i++) {
				if (test[i] >= 0) {
					if (mindist < 0 || test[i] < mindist) {
						n = i;
						mindist = test[i];
					}
				}
			}
			//we hit object n
			SpinOff(n);
		}
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
