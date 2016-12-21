/*
Parker Whaley
final
*/
#include <ctime>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include "Angel.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
struct node {
	vec4 data;
	point4 normal;
	vec2 tex;
};


//now our functions
void m_glewInitAndVersion(void);
void reshape(int width, int height);
void mouse(int x, int y);
void mouseCall(int button, int state, int ix, int iy);
void keyboard(unsigned char key, int x, int y);
void display(void);
void init();
vec3 make3(vec4 input);
vec4 make4(vec3 input,GLfloat omega);
vec4 solve(mat4 A, vec4 b);
vec3 IntersectPlane(vec3 point, vec3 normal, vec3 orgVec);
GLfloat DistSphere(vec3 vecFromOrg, mat4 sphereLoc);
GLuint textureLoader(std::string filename);
void animate(int i);
GLfloat randF();

//some constants for the fineness

//control constants
bool clicked;
vec2 previousClickLocation(0, 0);

//window constants
int widthInPix, hightInPix;
mat4 modelviewStackTop = RotateZ(-90)*Translate(-15.0, 0.0, -25.0)*RotateY(-90);
mat4 projmat = Translate(0.0, 0.0, 0.0);

//locations on the GPU
GLuint proj_loc, model_view_loc, AmbientProduct, DiffuseProduct, SpecularProduct, LightPosition, Shininess, ModelWorld, TexCoord, texID, program, vPosition, vNormal;





class object
{
public:
	virtual bool inside(point4 pointToCheck) = 0;
	bool overlap(void* other,mat4 pos);
	virtual GLfloat select(vec3 vecFromOrg, mat4 Loc) = 0;
	GLfloat posBuff() { return buffer[0]; }
	GLfloat normBuff() { return buffer[1]; }
	GLfloat texBuff() { return buffer[2]; }
	int buffLength() { return lengthOfBuff; }
protected:
	void setup();
	std::list<node> points;
	GLuint buffer[3];
	int lengthOfBuff;

};

class sphere : public object
{
public:
	sphere();
	bool inside(point4 pointToCheck);
	GLfloat select(vec3 vecFromOrg, mat4 Loc);
private:
	void addOnPoint(point4 p);
	void devideTryforsphere(int n, point4 p1, point4 p2, point4 p3);
};

class cube : public object
{
public:
	cube();
	bool inside(point4 pointToCheck);
	GLfloat select(vec3 vecFromOrg, mat4 Loc);
private:
};

class landscape : public object
{
public:
	landscape();
	bool inside(point4 pointToCheck);
	GLfloat select(vec3 vecFromOrg, mat4 Loc) { return -1; }
private:
	GLfloat hight(vec2 position);
	GLfloat Dx(vec2 position);
	GLfloat Dy(vec2 position);
	static const int numCon = 5;
	static const int finness = 10;
	GLfloat constants[numCon][numCon];
};

class objectInst
{
public:
	objectInst(object* theObj, mat4 posin, GLfloat radiousin, GLfloat shinein, GLuint texBufferIDin) { shine = shinein; myObject = theObj; pos = posin; radious = radiousin; texBufferID = texBufferIDin; }
	objectInst(mat4 posin, GLfloat radiousin, GLfloat shinein, const objectInst& other) { shine = shinein; myObject = other.myObject; pos = posin; radious = radiousin; texBufferID = other.texBufferID; }
	void print();
	GLfloat select(vec3 vecFromOrg);
	bool inside(point4 pointToCheck);
	bool overlap(objectInst& other);
	void setCOM(point4 newCOM) { COM = newCOM; }
	vec4 getCOM() { return COM; }
	bool toggleMovement() { return (canMove = !canMove); }
	GLfloat getRadious() { return radious; }
	vec4 translateToObject(vec4 in) { return solve(pos,in); }
	vec4 translateToWorld(vec4 in) { return pos*in; }
	vec4 newVelos(objectInst& other, bool & realInteract);
	vec4 getVelocity() { return velocity; }
	void setVelocity(vec4 newVelos) { velocity = newVelos; }
	void step(GLfloat dt);
	void setFastCheck(int n) { fastCheck = n; }
	bool FastChecker(bool& interact, objectInst& other, int callNumber);
private:
	GLfloat bounce = .6;
	point4 COM = vec4(0, 0, 0, 1);
	object* myObject;
	mat4 pos;
	vec4 velocity=vec4(0,0,0,0);
	bool canMove = true;
	GLfloat radious;
	GLuint texBufferID;
	GLfloat shine;
	int fastCheck = -1;
};

//things to be desplayed
std::list<objectInst> ObjInWorld;





int main(int argc, char **argv)
{

	srand(std::time(NULL));
	rand();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutCreateWindow("world");

	m_glewInitAndVersion();


	init();

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);
	glutMotionFunc(mouse);
	glutMouseFunc(mouseCall);

	glutTimerFunc(1000, animate, 0);
	glutMainLoop();
	return 0;
}

void animate(int i){
	if (i % 1 == 0) {
		std::unordered_map<objectInst*, vec4> newVelocities;
		for (objectInst& obj1 : ObjInWorld) {
			bool interact = false;
			for (objectInst& obj2 : ObjInWorld) {
				if (&obj1 == &obj2) { continue; }
				//they are not the same object
				//can they interact?
				GLfloat radDist = obj1.getRadious() + obj2.getRadious();
				vec4 temp = obj1.translateToWorld(vec4(0, 0, 0, 1)) - obj2.translateToWorld(vec4(0, 0, 0, 1));
				if (radDist*radDist < dot(temp, temp)) { continue; }//this is a quick check, fail false

				if (!obj1.FastChecker(interact, obj2, 0)) {
					if (!obj1.overlap(obj2)) { continue; }//slow but thurogh
					interact = true;
				}
				if (!interact) { continue; }
				//if we are here then the two intersect
				vec4 newVel = obj1.newVelos(obj2, interact);

				if (interact) { newVelocities[&obj1] = newVel; }
				break;
			}
			if (!interact) { newVelocities[&obj1] = obj1.getVelocity(); }
		}
		for (objectInst& obj1 : ObjInWorld) {
			obj1.setVelocity(newVelocities[&obj1]);
		}
	}
	for (objectInst& obj1 : ObjInWorld) {
		obj1.step(1 / 60.);
	}









	glutTimerFunc(17, animate, i+1);
	glutPostRedisplay();
	
}

void reshape(int w, int h)
//the same objects are shown (possibly scaled) w/o shape distortion 
//original viewport is a square
{

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	widthInPix = w;
	hightInPix = h;

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

void mouse(int x, int y) {
	if (clicked == true) {
		GLfloat step = .1;
		modelviewStackTop = RotateY(-(x - previousClickLocation.x)*step)*modelviewStackTop;
		modelviewStackTop = RotateX(-(y - previousClickLocation.y)*step)*modelviewStackTop;
		previousClickLocation = vec2(x, y);
	}
}

void mouseCall(int button, int state, int ix, int iy) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		clicked = true;
		previousClickLocation = vec2(ix, iy);
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		clicked = false;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		vec4 tempvec = solve(projmat, vec4(2 * (ix * 2. / widthInPix - 1), 2 * ((hightInPix - iy) * 2. / hightInPix - 1), 1, 2));
		vec3 direc = normalize(make3(tempvec));
		std::vector<GLfloat> test;
		for (objectInst& obj : ObjInWorld) {
			test.push_back(obj.select(direc));
		}
		for (int i = 0; i < test.size(); i++) {
			if (test[i] >= 0) {
				std::cout << "hit object " << i << " at " << test[i] << std::endl;
			}
		}
		std::cout << std::endl;
	}
}

void keyboard(unsigned char key, int x, int y){
	GLfloat transdiff = .1;
	GLfloat step = .1;
	switch (key) {
	case 'w':
		modelviewStackTop = Translate(0.0, 0.0, transdiff)*modelviewStackTop;
		break;
	case 's':
		modelviewStackTop = Translate(0.0, 0.0, -transdiff)*modelviewStackTop;
		break;
	case 'd':
		modelviewStackTop = Translate(-transdiff, 0, 0)*modelviewStackTop;
		break;
	case 'a':
		modelviewStackTop = Translate(transdiff, 0, 0)*modelviewStackTop;
		break;
	case 'e':
		modelviewStackTop = RotateZ(step * 10)*modelviewStackTop;
		break;
	case 'q':
		modelviewStackTop = RotateZ(-step * 10)*modelviewStackTop;
		break;
	case 033: case 'Q':  // Escape key
		exit(EXIT_SUCCESS);
	}
}

void display(void){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	projmat = Frustum(-.1*widthInPix / 512., .1*widthInPix / 512., -.1*hightInPix / 512., .1*hightInPix / 512., .5, 1000000);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, projmat);
	glUniformMatrix4fv(model_view_loc, 1, GL_TRUE, modelviewStackTop);
	
	int n = 0;
	for (objectInst& obj : ObjInWorld) {
		if (n == 0) {
			glUniform4fv(LightPosition, 1, obj.translateToWorld(obj.getCOM()));
			glUniform4fv(AmbientProduct, 1, color4(1,.1,.1, 1.0));
			glUniform4fv(DiffuseProduct, 1, color4(0,0,0, 1.0));
			glUniform4fv(SpecularProduct, 1, color4(0,0,0, 1.0));
		}
		else {
			glUniform4fv(AmbientProduct, 1, color4(0.2, 0.2, 0.2, 1.0));
			glUniform4fv(DiffuseProduct, 1, color4(1,1,1, 1.0)* color4(1, .1, .1, 1.0));
			glUniform4fv(SpecularProduct, 1, color4(.8, .8, .8, 1.0)* color4(1, .1, .1, 1.0));
		}
		n = 1;
		obj.print();
	}
	


	



	glutSwapBuffers();
}

void init() {
	srand(std::time(NULL));

	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(texID, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	// Load shaders and use the resulting shader program
	program = InitShader("vshaderCC_v150.glsl", "fshaderCC_v150.glsl");
	glUseProgram(program);
	vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	TexCoord = glGetAttribLocation(program, "s_vTexCoord");
	glEnableVertexAttribArray(TexCoord);
	
	texID = glGetUniformLocation(program, "texture");
	proj_loc = glGetUniformLocation(program, "Projection");
	model_view_loc = glGetUniformLocation(program, "ModelView");
	ModelWorld = glGetUniformLocation(program, "ModelWorld");
	AmbientProduct = glGetUniformLocation(program, "AmbientProduct");
	DiffuseProduct = glGetUniformLocation(program, "DiffuseProduct");
	SpecularProduct = glGetUniformLocation(program, "SpecularProduct");
	LightPosition = glGetUniformLocation(program, "LightPosition");
	Shininess = glGetUniformLocation(program, "Shininess");



	glUniform4fv(AmbientProduct, 1, color4(0.2, 0.2, 0.2, 1.0));
	glUniform4fv(DiffuseProduct, 1, color4(.5, .5, .5, 1.0));
	glUniform4fv(SpecularProduct, 1, color4(.8, .8, .8, 1.0));

	glClearColor(0, 0, 0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthFunc(GL_LEQUAL);

	//objects created
	sphere* s = new sphere;
	cube* c = new cube;
	landscape* l = new landscape;
	GLfloat gold = textureLoader("gold.bmp");
	GLfloat silver = textureLoader("silver.bmp");
	GLfloat water = textureLoader("water.bmp");
	GLfloat felt = textureLoader("felt.bmp");
	
	//things to be desplayed
	objectInst temp= objectInst(c, Translate(0, 0, -20), 1, 50, silver);
	temp.setVelocity(vec4(1, 1, 30, 0));
	ObjInWorld.push_back(temp );

	temp= objectInst(l, Translate(0, 0, -35)*Scale(15,15,15), 100, 1, felt);
	temp.toggleMovement();
	temp.setCOM(point4(0, 0, -1000, 1));
	temp.setFastCheck(1);
	ObjInWorld.push_back(temp);

	temp = objectInst(s, Translate(0, -5, -20), 1, 50, gold);
	temp.setFastCheck(0);
	ObjInWorld.push_back(temp);
	temp = objectInst(s, Translate(5, 0, -20), 1, 50, gold);
	temp.setFastCheck(0);
	ObjInWorld.push_back(temp);
	temp = objectInst(s, Translate(-5, 0, -20), 1, 50, gold);
	temp.setFastCheck(0);
	ObjInWorld.push_back(temp);
	temp = objectInst(s, Translate(0, 5, -20), 1, 50, gold);
	temp.setFastCheck(0);
	ObjInWorld.push_back(temp);
	temp = objectInst(s, Translate(0, 0, -25), 1, 50, gold);
	temp.setFastCheck(0);
	ObjInWorld.push_back(temp);
	temp = objectInst(s, Translate(0, 0, -15), 1, 50, gold);
	temp.setFastCheck(0);
	ObjInWorld.push_back(temp);


}

vec3 make3(vec4 input) {
	return vec3(input.x, input.y, input.z);
}

vec4 make4(vec3 input, GLfloat omega) {
	return vec4(input.x, input.y, input.z, omega);
}

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

vec3 IntersectPlane(vec3 point, vec3 normal, vec3 orgVec) {
	normal = normalize(normal);
	orgVec = normalize(orgVec);
	vec3 toPlane = normal*dot(point, normal);
	return orgVec*dot(toPlane, toPlane) / dot(toPlane, orgVec);
}

GLfloat DistSphere(vec3 vecFromOrg, mat4 sphereLoc) {
	//first take the vector from the origin an put it in the spheres cordinates
	point4 origin = solve(sphereLoc, point4(0, 0, 0, 1));
	vec3 vecFromOrgSphere = make3(solve(sphereLoc, make4(vecFromOrg, 1)) - origin);
	vec3 sphereCenter = - make3(origin);
	vec3 normal = normalize(make3(origin));
	vec3 intersect = IntersectPlane(sphereCenter, normal, vecFromOrgSphere);
	vec3 vecInSphere = intersect + make3(origin);
	if (dot(vecInSphere, vecInSphere) > 1) {
		return -1;
	}
	return sqrt(dot(sphereLoc*vec4(0,0,0,1), sphereLoc*vec4(0, 0, 0, 1))); //returning the distance to a point in the sphere
}

GLuint textureLoader(std::string filename) {
	GLuint retval;
	char* imageData;
	int bmpWidth = -1;
	int bmpHeight = -1;
	int bmpSize = -1;
	Angel::loadBitmapFromFile(filename.c_str(), &bmpWidth, &bmpHeight, &bmpSize, (unsigned char**)&imageData);
	glGenTextures(1, &retval);
	glBindTexture(GL_TEXTURE_2D, retval);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmpWidth, bmpHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);
	return retval;
}

GLfloat randF() {
	return (GLfloat)rand() / (GLfloat)RAND_MAX;
}







void objectInst::print() {
	glBindTexture(GL_TEXTURE_2D, texBufferID);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(texID, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glUniform1f(Shininess, shine);
	glUniformMatrix4fv(ModelWorld, 1, GL_TRUE, pos);

	glBindBuffer(GL_ARRAY_BUFFER, myObject->posBuff());
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, myObject->normBuff());
	glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glBindBuffer(GL_ARRAY_BUFFER, myObject->texBuff());
	glVertexAttribPointer(TexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glDrawArrays(GL_TRIANGLES, 0, myObject->buffLength());
	//glDrawArrays(GL_TRIANGLES, 0, 3);
	
}

GLfloat objectInst::select(vec3 vecFromOrg) {
	return myObject->select(vecFromOrg, modelviewStackTop*pos);
}

bool objectInst::inside(point4 pointToCheck) {
	return myObject->inside(solve(pos,pointToCheck));
}

bool objectInst::overlap(objectInst& other) {
	return myObject->overlap(&other,pos) || other.myObject->overlap(this,other.pos);
}

vec4 objectInst::newVelos(objectInst& other,bool & realInteract) {
	vec4 fromto = normalize(translateToWorld(COM) - other.translateToWorld(other.COM));
	if (dot(velocity, fromto) > 0 || !canMove) {
		realInteract = false;
		return velocity;
	}
	if (!other.canMove) {
		vec4 newVelocity = velocity
			- 2 * dot(velocity, fromto)*fromto;
		return newVelocity*bounce;
	}
	vec4 newVelocity = velocity
		- dot(velocity, fromto)*fromto
		+ dot(other.velocity, fromto)*fromto;
	return newVelocity*bounce;
}

void objectInst::step(GLfloat dt) {
	if (!canMove) { return; }
	pos = Translate(dt*make3(velocity))*pos;
	velocity.z += -dt*9.8;//9.8m/s^2=gravatational acceleration
}

//this is a fast collision checker, if animations are to slow for two objects write a fast checker for them
//if you don't want to write one just put return false.
bool objectInst::FastChecker(bool& interact, objectInst& other,int callNumber) {
	switch (fastCheck)
	{
	case 0:
		//we are a sphere
		switch (other.fastCheck)
		{
		case 1:
			//it is land 
			interact = other.inside(translateToWorld(COM) - vec4(0, 0, radious,0));
			return true;
		}
		break;
	}
	//we Wern't able to handel it
	if (callNumber == 1) {
		return false;
	}
	return other.FastChecker(interact, *this, 1);
}

void object::setup() {
	glGenBuffers(3, buffer);
	lengthOfBuff = points.size();
	vec4* pointsarr = new vec4[lengthOfBuff];
	point4* normals = new point4[lengthOfBuff];
	vec2* texs = new vec2[lengthOfBuff];
	int n = 0;
	for (node temp : points) {
		pointsarr[n] = temp.data;
		normals[n] = temp.normal;
		texs[n] = temp.tex;
		n++;
	}
	glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*lengthOfBuff, pointsarr, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*lengthOfBuff, normals, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*lengthOfBuff, texs, GL_STATIC_DRAW);
	delete[] pointsarr;
	delete[] normals;
	delete[] texs;
}

bool object::overlap(void* other,mat4 pos) {
	objectInst* otherp = (objectInst*)other;
	for (node n : points) {
		if (otherp->inside(pos*n.data)) {
			return true;
		}
	}
	return false;
}


//landscape stuff
GLfloat landscape::hight(vec2 position) {
	GLfloat retval = 0;
	for (int i = 0; i < numCon; i++) {
		for (int j = 0; j < numCon; j++) {
			retval += constants[i][j]*((std::cos(M_PI*(1+2*i)*position.x)+1)*(std::cos(M_PI*(1 + 2 * j)*position.y)+1)/2-1);
		}
	}
	return retval;
}

GLfloat landscape::Dx(vec2 position) {
	GLfloat retval = 0;
	for (int i = 0; i < numCon; i++) {
		for (int j = 0; j < numCon; j++) {
			retval += constants[i][j] * ((-M_PI*(1 + 2 * i)*std::sin(M_PI*(1 + 2 * i)*position.x))*(std::cos(M_PI*(1 + 2 * j)*position.y) + 1) / 2 - 1);
		}
	}
	return retval;
}

GLfloat landscape::Dy(vec2 position) {
	GLfloat retval = 0;
	for (int i = 0; i < numCon; i++) {
		for (int j = 0; j < numCon; j++) {
			retval += constants[i][j] * ((std::cos(M_PI*(1 + 2 * i)*position.x) + 1)*(-M_PI*(1 + 2 * j)*std::sin(M_PI*(1 + 2 * j)*position.y)) / 2 - 1);
		}
	}
	return retval;
}

landscape::landscape() {
	constants[0][0] = 1;
	for (int i = 0; i < numCon; i++) {
		for (int j = 0; j < numCon; j++) {
			//if (i == 0 && j == 0) { continue; }
			constants[i][j] = (randF() - .5) / std::pow(i + j + 2,3);
		}
	}
	point4 grid[finness][finness];
	point4 normalGrid[finness][finness];
	for (int i = 0; i < finness; i++) {
		for (int j = 0; j < finness; j++) {
			GLfloat x = (i / (GLfloat)(finness - 1)) * 2 - 1;
			GLfloat y = (j / (GLfloat)(finness - 1)) * 2 - 1;
			grid[i][j] = vec4(x, y, hight(vec2(x, y)), 1);
			normalGrid[i][j] = normalize(vec4(-Dx(vec2(x, y)), -Dy(vec2(x, y)), 1, 0));
		}
	}
	node temp[4];
	for (int i = 0; i < finness-1; i++) {
		for (int j = 0; j < finness-1; j++) {
			temp[0].data = grid[i][j];
			temp[1].data = grid[i+1][j];
			temp[2].data = grid[i+1][j+1];
			temp[3].data = grid[i][j+1];

			temp[0].normal = normalGrid[i][j];
			temp[1].normal = normalGrid[i + 1][j];
			temp[2].normal = normalGrid[i + 1][j + 1];
			temp[3].normal = normalGrid[i][j + 1];
			for (int k = 0; k < 4; k++) {
				temp[k].tex = .5*(vec2(temp[k].data.x, temp[k].data.y) + vec2(1, 1));
				
			}
			points.push_back(temp[0]);
			points.push_back(temp[1]);
			points.push_back(temp[2]);

			points.push_back(temp[2]);
			points.push_back(temp[3]);
			points.push_back(temp[0]);
		}
	}
	setup();
}

bool landscape::inside(point4 pointToCheck) {
	return pointToCheck.z < hight(vec2(pointToCheck.x, pointToCheck.y));
}


//cube stuff
cube::cube() {
	GLfloat dist = 1 / sqrt(3);
	vec4 normal(1,0,0,1);
	vec4 Apoints[6] = { vec4(dist,dist,dist,1),vec4(dist,-dist,dist,1),vec4(dist,-dist,-dist,1),vec4(dist,-dist,-dist,1),vec4(dist,dist,-dist,1),vec4(dist,dist,dist,1) };
	vec2 texcord[6] = { vec2(1,1),vec2(1,0),vec2(0,0),vec2(0,0),vec2(0,1),vec2(1,1) };
	mat4 rotations[6] = { RotateZ(0),RotateZ(90),RotateZ(90 * 2),RotateZ(90 * 3),RotateY(90),RotateY(-90) };
	node temp;
	for (mat4& rot : rotations) {
		temp.normal=rot*normal - vec4(0, 0, 0, 1);
		for (int i = 0; i < 6; i++) {
			temp.data = rot*Apoints[i];
			temp.tex = texcord[i];
			points.push_back(temp);
		}
	}
	setup();
}

bool cube::inside(point4 pointToCheck) {
	return (1 / sqrt(3)) > std::fmax(std::fmax(std::abs(pointToCheck.x), std::abs(pointToCheck.y)), std::abs(pointToCheck.z));
}

GLfloat cube::select(vec3 vecFromOrg, mat4 Loc) {
	GLfloat dist = 1 / sqrt(3);
	GLfloat eps = 1e-6;
	vec3 center = make3(Loc*vec4(0, 0, 0, 1));
	vec3 face[6] = { make3(Loc*vec4(dist, 0, 0, 1)),make3(Loc*vec4(-dist, 0, 0, 1)),
		make3(Loc*vec4(0,dist, 0, 1)) ,make3(Loc*vec4(0,-dist, 0, 1)),
		make3(Loc*vec4(0, 0,dist, 1)) ,make3(Loc*vec4(0, 0,-dist, 1)) };
	vec3 intersect;
	vec4 intersectInCube;
	for (int i = 0; i < 6; i++) {
		intersect = IntersectPlane(face[i], normalize(face[i] - center), vecFromOrg);
		intersectInCube = solve(Loc, make4(intersect, 1));
		if (inside(intersectInCube*(1 - eps))) {
			return std::sqrt(dot(intersect, intersect));
		}
	}
	return -1;
}







//sphere stuff
GLfloat sphere::select(vec3 vecFromOrg, mat4 Loc) {
	return DistSphere(vecFromOrg, Loc);
}

void sphere::addOnPoint(point4 p) {
	GLfloat eps = 1e-5;
	node temp;
	temp.data = p;
	temp.normal = point4(p.x, p.y, p.z, 0);
	vec2 point = vec2(p.x, p.y) / sqrt(p.x*p.x + p.y*p.y + eps);
	GLfloat ang = acos(point.x);
	if (point.y < 0) {
		ang = 2 * M_PI - ang;
	}
	temp.tex = vec2(p.z / 2 + 1, ang / (2 * M_PI));
	points.push_back(temp);
}

void sphere::devideTryforsphere(int n, point4 p1, point4 p2, point4 p3) {
	if (n == 0) {
		addOnPoint(p1);
		addOnPoint(p2);
		addOnPoint(p3);
		return;
	}
	n--;
	point4 central[3] = { (p1 + p2) / 2,(p1 + p3) / 2,(p3 + p2) / 2 };
	for (int i = 0; i < 3; i++) {
		central[i] = normalize(point4(central[i].x, central[i].y, central[i].z, 0)) + point4(0, 0, 0, 1);
	}
	devideTryforsphere(n, p1, central[0], central[1]);
	devideTryforsphere(n, p2, central[0], central[2]);
	devideTryforsphere(n, p3, central[2], central[1]);
	devideTryforsphere(n, central[2], central[0], central[1]);
}

sphere::sphere() {
	int fine = 4;
	point4 v[4] = {//from prof's code
		vec4(0.0, 0.0, 1.0, 1.0),
		vec4(0.0, 0.942809, -0.333333, 1.0),
		vec4(-0.816497, -0.471405, -0.333333, 1.0),
		vec4(0.816497, -0.471405, -0.333333, 1.0)
	};//end prof's code
	devideTryforsphere(fine, v[0], v[1], v[2]);
	devideTryforsphere(fine, v[0], v[1], v[3]);
	devideTryforsphere(fine, v[0], v[3], v[2]);
	devideTryforsphere(fine, v[3], v[1], v[2]);
	setup();
}

bool sphere::inside(point4 pointToCheck) {
	vec3 point = make3(pointToCheck);
	return dot(point, point) < 1;
}



























