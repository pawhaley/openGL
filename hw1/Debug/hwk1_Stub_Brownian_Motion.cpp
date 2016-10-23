//
//G. Kamberov
// A sample brownian motion simulation stub. 
// Game play: 
// A particle is released inside a box with corners:  
//         vec2(minX, minY), vec2(maxX, minY), vec2(maxX, maxY), vec2(minX, maxY)
//         The area of the random intial location is marked with a green square
//         The partilce trajectory is drawn until the particle hits one of the box walls.
//         The exit condition test and teh final position are computed in the fucntions animate and 
//         findExitPoint.
//
// Note that to minimize the size of the buffers all points are stored as 2d vectors. 
// The vertex shader makes sure to interopet the 2D vertex postion attributes  
// as 4D vectors. 
//
// To compile you will need the enclosed header file hwk1.h  in the same directory as this source file. 
//
//
// We use the default orthographic projection
//
//
//GLSL version 110 shaders: 
//   vshader23_v110.glsl
//   fshader23_v110.glsl
//
//
//

#include "hwk1.h"

void randomDisplacement(GLfloat magnitude, GLfloat &side1, GLfloat &side2)
{
	GLfloat angle = ((GLfloat)rand()/(GLfloat)RAND_MAX) * (2 * PI);
	side1 = magnitude * cos(angle);
	side2 = magnitude * sin(angle);
}

//this will count all the nodes after head;
int pointCount(struct pointNode* head)
{
	pointNode* tmp;
	tmp = head;
	int count_l = 0;

	while(tmp != NULL)
	{
		count_l++;
		tmp = tmp->next;
	}
	return count_l;
}


pointNode* getRandomStart(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax)
{
	struct pointNode* retVal;
	GLfloat xLen = xMax - xMin;
	GLfloat yLen = yMax - yMin;

	GLfloat startX = ((GLfloat)rand()/(GLfloat)RAND_MAX) * xLen + xMin;
	GLfloat startY = ((GLfloat)rand()/(GLfloat)RAND_MAX) * xLen + xMin;

	retVal = (pointNode*)malloc(sizeof(pointNode));
	retVal->x = startX;
	retVal->y = startY;
	retVal->next = NULL;

	return retVal;

}

pointNode* AddNode( pointNode* node, GLfloat x, GLfloat y)
{
	struct pointNode* tmp = NULL;
	while(node->next != NULL)
	{
		node = node->next;
	}
	
	tmp = (pointNode *) malloc(sizeof(pointNode));
	tmp->x = x;
	tmp->y = y;

	tmp->next = NULL;
	node->next = tmp;

	return tmp;
}

//this function will determine the length of the displacement vectors
//it will be 1/50 the shortest side.
GLfloat calcDisplacement(GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax)
{
	GLfloat lenX = xMax - xMin;
	GLfloat lenY = yMax - yMin;

	if(lenX < lenY)
	{
		return lenX/50.0/numBox;
	}
	else
	{
		return lenY/50.0/numBox;
	}
}

/*
//this function will check to see if the node is withing the box
//this function will check intercept
bool checkNode(struct pointNode * last, GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax)
{//                                                                                                             <----------------------------needs work
	pointNode* curr = last->next;
	if(curr->x < xMin || curr->y < yMin || curr->x > xMax || curr->y > yMax )
	{
		return false;
	}
	//we are in the box
	//do we hit a horizontal bar
	for (int y = 0; y < numBox+1; y++) {
		double hight = maxY-(maxY - minY) / numBox*(y);
		bool crossBar = (hight - last->y)*(hight - curr->y) <= 0;  //if true we crossed
		if (crossBar) {
			double slope = (curr->y - last->y) / (curr->x - last->x);
			double overdist = slope*(hight - curr->y) + curr->x;//the cross point
			int x = (int)((overdist - minX) / ((maxX - minX) / numBox));
			if (wallsH[x][y]) {
				cout << "B" << " " << curr->x << " " << curr->y << " " << last->x << " " << last->y << " " << hight << " " << (curr->y - last->y) << "/" << (curr->x - last->x)<< "=" << slope << " " << y << " " << x << endl;
				return false;
			}
		}
	}
	//do we hit a vertical bar
	for (int x = 0; x < numBox + 1; x++) {
		double over = minX + x*((maxX - minX) / numBox);
		bool crossBar = (over - last->x)*(over - curr->x) <= 0;  //if true we crossed
		if (crossBar) {
			double slope = (curr->y - last->y) / (curr->x - last->x);
			//double hight = (over-curr->x)/slope + curr->y;//the cross point
			double hight = (curr->x - last->x)*(over - curr->x) / (curr->y - last->y) + curr->y;
			int y = (int)((maxY - hight)/ ((maxY - minY) / numBox));
			if (wallsV[x][y]) {
				cout << "B" << " " << curr->x << " " << curr->y << " " << last->x << " " << last->y << " " << hight << " " << (curr->y - last->y) << "/" << (curr->x - last->x) << "=" << slope << " " << y << " " << x << endl;
				return false;
			}
		}
	}
	return true;
}*/

//there were rounding problems with using the slope
double findXcross(pointNode * last, double hight) {
	double N = 100;
	double x1 = last->x;
	double y1 = last->y;
	double x2 = last->next->x;
	double y2 = last->next->y;
	if(!(y1>hight)){
		double temp = y1;
		y1 = y2;
		y2 = temp;
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (!(y1 > hight) || (y2 > hight)) {
		return 1000000;//ERROR!!!
	}
	//assert: y1 > hight > y2
	double ytemp;
	for (int i = 1; i < N; i++) {
		ytemp = (y1 + y2) / 2;
		if (ytemp > hight) {
			y1 = ytemp;
			x1 = (x1 + x2) / 2;
		}
		else {
			y2 = ytemp;
			x2 = (x1 + x2) / 2;
		}
	}
	return (x1 + x2) / 2;
}

//there were rounding problems with using the slope
double findYcross(pointNode * last, double over) {
	double N = 100;
	double x1 = last->x;
	double y1 = last->y;
	double x2 = last->next->x;
	double y2 = last->next->y;
	if (!(x1 > over)) {
		double temp = y1;
		y1 = y2;
		y2 = temp;
		temp = x1;
		x1 = x2;
		x2 = temp;
	}
	if (!(x1 > over) || (x2 > over)) {
		return 1000000;//ERROR!!!
	}
	//assert: x1 > over > x2
	double xtemp;
	for (int i = 1; i < N; i++) {
		xtemp = (x1 + x2) / 2;
		if (xtemp > over) {
			x1 = xtemp;
			y1 = (y1 + y2) / 2;
		}
		else {
			x2 = xtemp;
			y2 = (y1 + y2) / 2;
		}
	}
	return (y1 + y2) / 2;
}



bool checkNode(struct pointNode * last, GLfloat xMin, GLfloat xMax, GLfloat yMin, GLfloat yMax)
{//                                                                                                             <----------------------------needs work
	pointNode* curr = last->next;
	
	//we are in the box
	//do we hit a horizontal bar
	for (int y = 0; y < numBox + 1; y++) {
		double hight = maxY - (maxY - minY) / numBox*(y);
		bool crossBar = (hight - last->y)*(hight - curr->y) <= 0;  //if true we crossed
		if (crossBar) {
			double slope = (curr->y - last->y) / (curr->x - last->x);
			double overdist = findXcross(last, hight); //= slope*(hight - curr->y) + curr->x;//the cross point
			int x = (int)((overdist - minX) / ((maxX - minX) / numBox));
			if (wallsH[x][y]) {
				done = true;
				return false;
			}
		}
	}
	//do we hit a vertical bar
	for (int x = 0; x < numBox + 1; x++) {
		double over = minX + x*((maxX - minX) / numBox);
		bool crossBar = (over - last->x)*(over - curr->x) <= 0;  //if true we crossed
		if (crossBar) {
			double slope = (curr->y - last->y) / (curr->x - last->x);
			//double hight = (over-curr->x)/slope + curr->y;//the cross point
			double hight = findYcross(last, over); //= (curr->x - last->x)*(over - curr->x) / (curr->y - last->y) + curr->y;
			int y = (int)((maxY - hight) / ((maxY - minY) / numBox));
			if (wallsV[x][y]) {
				done = true;
				return false;
			}
		}
	}
	if (curr->x < xMin || curr->y < yMin || curr->x > xMax || curr->y > yMax)
	{
		done = true;
		escape = true;
		return false;
	}
	return true;
}



void init()
{
	//copy the cross into a buffer 
	glGenBuffers(5, &buffers[0]);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(crossB), crossB, GL_STATIC_DRAW);


	for (int i = 0; i < numBox; i++) {
		for (int j = 0; j < numBox + 1; j++) {
			wallsH[i][j] = true;
			wallsV[j][i] = true;
			/*if ((j > 0 && j < numBox)) {
				wallsH[i][j] = ((GLfloat)rand() / (GLfloat)RAND_MAX) > .75;
			}
			if ((j > 0 && j < numBox)) {
				wallsV[j][i] = ((GLfloat)rand() / (GLfloat)RAND_MAX) > .75;
			}*/
		}
	}
	int num = 2;
	int sidesLeft = 4 * numBox;
	for (int i = 0; i < numBox; i++) {
		if (((GLfloat)rand() / (GLfloat)RAND_MAX) <= num / (double)sidesLeft) {
			num--;
			wallsH[i][0] = false;
		}
		sidesLeft--;
		if (((GLfloat)rand() / (GLfloat)RAND_MAX) <= num / (double)sidesLeft) {
			num--;
			wallsH[i][numBox] = false;
		}
		sidesLeft--;
		if (((GLfloat)rand() / (GLfloat)RAND_MAX) <= num / (double)sidesLeft) {
			num--;
			wallsV[0][i] = false;
		}
		sidesLeft--;
		if (((GLfloat)rand() / (GLfloat)RAND_MAX) <= num / (double)sidesLeft) {
			num--;
			wallsV[numBox][i] = false;
		}
		sidesLeft--;
	}
	//Angel::vec2 PtsH[(numBox + 1)*(numBox + 1)]
	for (int i = 0; i < numBox+1; i++) {
		for (int j = 0; j < numBox + 1; j++) {
			(PtsH[j + i*(numBox + 1)]).x = minX+j*((maxX-minX) / numBox);
			(PtsH[j + i*(numBox + 1)]).y = maxY-i*((maxY - minY) / numBox);
			(PtsV[j + i*(numBox + 1)]).x = minX+i*((maxX - minX) / numBox);
			(PtsV[j + i*(numBox + 1)]).y = maxY - j*((maxY - minY) / numBox);
		}
	}

	glClearColor (0.0, 0.0, 0.0, 0.0);

    //Starte the Brownian motion
	head = getRandomStart(minX, maxX, minY, maxY);
	curr = head;

		
	
	


    glBindBuffer( GL_ARRAY_BUFFER, buffers[0] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(bBox), bBox, GL_STATIC_DRAW );

	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PtsH), PtsH, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PtsV), PtsV, GL_STATIC_DRAW);

	// Load shaders and use the resulting shader program
    program = InitShader( "vshader23_v110.glsl","fshader23_v110.glsl" );
    glUseProgram( program );
	projmat_loc = glGetUniformLocation(program, "projmat");
	modelview_loc = glGetUniformLocation(program, "modelview");
	draw_color_loc = glGetUniformLocation(program, "vColor");


    // set up vertex attributes arrays
    GLuint vPosition = glGetAttribLocation( program, "vPosition" );
	glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );
	
	width = maxX - minX;
	height = maxY - minY;
}

bool colorLineG(bool H, int x, int y) {
	int dx = x - boxSelectX;
	int dy = y - boxSelectY;
	if (isBoxSelect && H) {
		//there is a box selected
		switch (dy){
		case -2: case 1:
			if (dx == -1) {
				return true;
			}
			break;
		case -1: case 0:
			if (dx == -1 || dx==-2 || dx==0) {
				return true;
			}
			break;
		}
	}
	if (isBoxSelect && !H) {
		//there is a box selected
		switch (dx) {
		case -2: case 1:
			if (dy == -1) {
				return true;
			}
			break;
		case -1: case 0:
			if (dy == -1 || dy == -2 || dy == 0) {
				return true;
			}
			break;
		}
	}
	return false;
}


void display()
{
	if (setup) {
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		projmat = Angel::mat4(1.0);
		glUniformMatrix4fv(projmat_loc, 1, GL_TRUE, projmat);


		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);

		//Draw box

		modelview = Angel::mat4(1.0);
		glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glUniform4fv(draw_color_loc, 1, yelow_box_edge);
		//glDrawArrays(GL_LINE_LOOP, 1, 4);

		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		modelview = Angel::mat4(1.0);
		glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glUniform4fv(draw_color_loc, 1, yelow_box_edge);

		//glDrawArrays(GL_LINE_STRIP, 0 + 0*(numBox + 1), 7);
		for (int i = 0; i < numBox; i++) {
			for (int j = 0; j < numBox + 1; j++) {
				if (wallsH[i][j] == true) {
					glDrawArrays(GL_LINES, i + j*(numBox + 1), 2);
				}
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		modelview = Angel::mat4(1.0);
		glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glUniform4fv(draw_color_loc, 1, yelow_box_edge);

		//glDrawArrays(GL_LINE_STRIP, 0 + 0*(numBox + 1), 7);
		for (int i = 0; i < numBox; i++) {
			for (int j = 0; j < numBox + 1; j++) {
				if (wallsV[j][i] == true) {
					glDrawArrays(GL_LINES, i + j*(numBox + 1), 2);
				}
			}
		}



		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		//Mark initial location
		modelview = Angel::mat4(1.0)*Angel::Translate(vec4(head->x, head->y, 0.0, 0.0))*Angel::Scale(width / 25.0, height / 25.0, 1.0);
		glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glUniform4fv(draw_color_loc, 1, green_start_marker);
		glDrawArrays(GL_QUADS, 0, 4);


		pointNode* end = head;
		while (end->next != nullptr) {
			end = end->next;
		}
		if (done && !escape) {
			glBindBuffer(GL_ARRAY_BUFFER, buffers[4]);
			//Mark initial location
			modelview = Angel::mat4(1.0)*Angel::Translate(vec4(end->x, end->y, 0.0, 0.0))*Angel::Scale(width / 100.0, height / 100.0, 1.0);
			glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
			glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
			glUniform4fv(draw_color_loc, 1, red_exit_marker);
			glDrawArrays(GL_LINES, 0, 2);
			glDrawArrays(GL_LINES, 2, 2);
		}
		else {
			glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
			//Mark current location
			modelview = Angel::mat4(1.0)*Angel::Translate(vec4(end->x, end->y, 0.0, 0.0))*Angel::Scale(width / 50.0, height / 50.0, 1.0);
			glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
			glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
			glUniform4fv(draw_color_loc, 1, wight_fly);
			glDrawArrays(GL_QUADS, 0, 4);
		}

		//Draw trajectory
		modelview = Angel::mat4(1.0);
		glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
		//copy the trajectory into a buffer 
		GLfloat * trajectoryBuffer = copyToArray(head);
		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * pointCount(head), trajectoryBuffer, GL_STREAM_DRAW);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glUniform4fv(draw_color_loc, 1, blue_trajectory);
		glDrawArrays(GL_LINE_STRIP, 0, pointCount(head));

		glutSwapBuffers();

		delete[] trajectoryBuffer;
	}
	else {
		//not setup yet
		glClearColor(1, 1, 1, 0);
		glClear(GL_COLOR_BUFFER_BIT);
		projmat = Angel::mat4(1.0);
		glUniformMatrix4fv(projmat_loc, 1, GL_TRUE, projmat);
		



		//Draw box



		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		modelview = Angel::mat4(1.0);
		glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glUniform4fv(draw_color_loc, 1, red_exit_marker);
		
		//glDrawArrays(GL_LINE_STRIP, 0 + 0*(numBox + 1), 7);
		for (int i = 0; i < numBox; i++) {
			for (int j = 0; j < numBox + 1; j++) {
				if (wallsH[i][j] == true && !colorLineG(true, i, j)) {
					glDrawArrays(GL_LINES, i + j*(numBox + 1), 2);
				}
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
		modelview = Angel::mat4(1.0);
		glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glUniform4fv(draw_color_loc, 1, green_start_marker);

		//glDrawArrays(GL_LINE_STRIP, 0 + 0*(numBox + 1), 7);
		for (int i = 0; i < numBox; i++) {
			for (int j = 0; j < numBox + 1; j++) {
				if (wallsH[i][j] == true && colorLineG(true, i, j)) {
					glDrawArrays(GL_LINES, i + j*(numBox + 1), 2);
				}
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		modelview = Angel::mat4(1.0);
		glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glUniform4fv(draw_color_loc, 1, red_exit_marker);

		//glDrawArrays(GL_LINE_STRIP, 0 + 0*(numBox + 1), 7);
		for (int i = 0; i < numBox; i++) {
			for (int j = 0; j < numBox + 1; j++) {
				if (wallsV[j][i] == true && !colorLineG(false, j, i)) {
					glDrawArrays(GL_LINES, i + j*(numBox + 1), 2);
				}
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, buffers[3]);
		modelview = Angel::mat4(1.0);
		glUniformMatrix4fv(modelview_loc, 1, GL_TRUE, modelview);
		glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
		glUniform4fv(draw_color_loc, 1, green_start_marker);

		//glDrawArrays(GL_LINE_STRIP, 0 + 0*(numBox + 1), 7);
		for (int i = 0; i < numBox; i++) {
			for (int j = 0; j < numBox + 1; j++) {
				if (wallsV[j][i] == true && colorLineG(false, j, i)) {
					glDrawArrays(GL_LINES, i + j*(numBox + 1), 2);
				}
			}
		}

		glutSwapBuffers();


	}
}

GLfloat* copyToArray(struct pointNode * head)
{
	GLfloat * retVal; 
	pointNode * tmp;
	int count_l;
	count_l = pointCount(head);
	int i = 0;
	
	count_l *= 2;

	tmp = head;
	if(count_l > 0 )
	{
	
		retVal = new GLfloat[count_l];
	}
	else
	{
		return NULL;
	}

	while(i < count_l)
	{
		retVal[i] = tmp->x;
		retVal[i+1] = tmp->y;
		tmp = tmp->next;
		i+=2;
	}
	return retVal;
}

void animate(int i)
{
	GLfloat x, y;
	pointNode * last;
	if(!bPaused && !bComplete && setup)
	{
		GLfloat displacement = calcDisplacement(minX, maxX, minY, maxY);
		
		last = curr;
		randomDisplacement(displacement, x, y);
		curr = AddNode(curr, curr->x + x, curr->y + y);
		int count_l;
		count_l = pointCount(head);

		glutPostRedisplay();
		//We only want to keep going if 
		if(checkNode(last, minX, maxX, minY, maxY))
		{
			//keep a roughly constat fps
			glutTimerFunc(17, animate, 0);
		}else
		{
			findExitPoint(last, curr);
			bComplete = true;
		}
	}
}

void keyboard (unsigned char key, int x, int y)
{
	switch (key) {
	case 'p':
		bPaused = !bPaused;
		if (!bPaused) {
			animate(0);
		}
		break;
	case 'Q': case 'q':
		exit(EXIT_SUCCESS);
		break;
	case 'd':
		if (!setup) {
			setup = true;
			animate(0);
		}
	}
}

void findExitPoint(struct pointNode * prev, struct pointNode * last)
{
	GLfloat slope;
	GLfloat b; // y -intercept of  the line y = slope*x + b

	GLfloat yExit;
	GLfloat xExit;
	
	//this will make sure one of the nodes is inside the other is outside
	if(true || checkNode(prev, minX, maxX, minY, maxY) && !checkNode(last, minX, maxX, minY, maxY))
	{
		slope = (last->y - prev->y)/(last->x - prev->x);
		b = prev->y - slope*prev->x;

		if(last->x > maxX)
		{
			yExit = slope * maxX + b;
			xExit = maxX;
			if(yExit < minY)
			{
				yExit = minY;
				xExit = (yExit - b) / slope;
			}
			if(yExit > maxY)
			{
				yExit = maxY;
				xExit = (yExit - b) / slope;
			}
		}
		else if(last->x < minX)
		{
			yExit = slope * minX + b;
			xExit = minX;
			if(yExit < minY)
			{
				yExit = minY;
				xExit = (yExit - b) / slope;
			}
			if(yExit > maxY)
			{
				yExit = maxY;
				xExit = (yExit - b) / slope;
			}
		}
		else if(last->y > maxY)
		{
			yExit = maxY;
			xExit = (yExit - b) / slope;
		}
		else if(last->y < minY)
		{
			yExit = minY;
			xExit = (yExit - b) / slope;
		}
		
	}
}

void convert(int ix,int iy,GLdouble& x,GLdouble& y){
	x=ix/(double) glutGet(GLUT_WINDOW_WIDTH)*2-1;
	//cout << x << " " << ix << " " << glutGet(GLUT_WINDOW_WIDTH) << " " << width << endl;
	y = iy / (double)glutGet(GLUT_WINDOW_HEIGHT) * 2 - 1;
}

void killEadge(int dx, int dy) {
	if (dx != 0) {
		switch (dx)
		{
		case -1:
			wallsV[boxSelectX][boxSelectY - 1] = false;
			break;
		case 1:
			wallsV[boxSelectX - 1][boxSelectY - 1] = false;
		}
	}
	else {
		switch (dy)
		{
		case -1:
			wallsH[boxSelectX - 1][boxSelectY] = false;
			break;
		case 1:
			wallsH[boxSelectX - 1][boxSelectY - 1] = false;
		}
	}
}

void mouseCall(int button, int state,int ix, int iy) {
	double x;
	double y;
	convert(ix, iy, x, y);
	int BoxX = (int)((x - minX) / (maxX - minX)*numBox + 1);
	int BoxY = (int)((y - minY) / (maxY - minY)*numBox + 1);

	if (setup) {
		return;
	}
	if (state == GLUT_UP) {
		return;
	}
	switch(button) {
	case GLUT_RIGHT_BUTTON:
		if (minX < x &&x<maxX && minY<y && y<maxY) { 
			//we want to select a box
			isBoxSelect = true;
			boxSelectX = BoxX;
			boxSelectY = BoxY;
		}
		break;
	case GLUT_LEFT_BUTTON:
		if (isBoxSelect && minX < x &&x < maxX && minY < y && y < maxY) {
			int dx = boxSelectX - BoxX;
			int dy = boxSelectY - BoxY;
			if (((dx == -1 || dx == 1) && dy == 0) || ((dy == -1 || dy == 1) && dx == 0)) {
				isBoxSelect = false;
				//we have a good box, now kill the eadge
				killEadge(dx, dy);
			}
		}
	}

}

int main(int argc, char** argv)
{

	struct pointNode* tmp = NULL;

	srand( time(NULL));
	rand();

	height = maxY - minY;
	width  =  maxX - minX;
	
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize (int(width*400), int(height*400)); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ("Brownian Motion in 1 by 1 Box");
	m_glewInitAndVersion();
	init();
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseCall);
	//wait a bit before we start
	glutTimerFunc(1000, animate, 0);
	glutMainLoop();
	return 0; 
}
                                   