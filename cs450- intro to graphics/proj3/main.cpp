/*
 *	Copyright (C) 2011  Kiel Friedt
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996) //This one is annoying
#endif
#define MAC

#include <stdlib.h> /* Must come first to avoid redef error */
#ifdef MAC
#include <GLUI/GLUI.h>
#endif
#ifdef WINDOWS
#include <GL/glui.h>  /* On MAC <GLUI/glui.h> */
#endif

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>

using namespace std;

/************************************************************************/
/*							DATA STRUCTURES                             */
/************************************************************************/

typedef struct
{
	float r, g, b;
} Color;

typedef struct 
{
	int v, n;
} Point;

typedef struct Object
{
	Color color;
	string name;
	float (*normals)[3], (*vertices)[3];
	Point (*faces)[3];
	int numVertices, numNormals, numFaces;
	GLuint displayList;
	float scale_matrix[3]; //x,y,z	
	float (*rotation_matrix)[3];	
	float translate_matrix[3];		
	float objRotMat[16];
} Object;

enum buttonTypes {OBJ_TEXTFIELD = 0, LOAD_BUTTON};
enum colors	{RED = 0, GREEN, BLUE};
enum projections {ORTHO = 0, PERSP};
enum transformations {TRANS = 0, ROT, SCALE}; 
enum camTransforms {CAMROTATE = 0 , TRACK, DOLLY};
const int FOV = 2;

/************************************************************************/
/*                       FUNCTION PROTOTYPES                            */
/************************************************************************/

int		loadObj			(char *, Object &);
void	setupVV			(void);
void	projCB			(int);
void	textCB			(int);
void	buttonCB		(int);
void	colorCB			(int);
void	drawObjects		(GLenum);
void	myGlutDisplay	(void);
void	myGlutReshape	(int, int);
void	myGlutMouse		(int , int , int , int );
void	processHits		(GLint, GLuint[]);
void	initScene		();

/************************************************************************/
/*                   CONSTANTS & GLOBAL VARIABLES                       */
/************************************************************************/

vector<Object> Objects;

const int WIN_WIDTH = 500;
const int WIN_HEIGHT = 500;

int main_window;
int objSelected = -1;
int currentm = 0;
int currentobj = 0;
float red = 1.f;
float green = 1.f;
float blue = 1.f;
float rotdif = 0.0;

int fov = 30;
const int FOVMIN = 0;
const int FOVMAX = 90;

projections projType = ORTHO;
transformations transType = TRANS;

int sizeX = WIN_WIDTH;
int sizeY = WIN_WIDTH;
float vl = -1.f;
float vr = 1.f;
float vb = -1.f;
float vt = 1.f;
const float vn = .5;
const float vf = 11;

float scaleFactor = 1.f;
float camRotMat[16];
float camTrack[2] = {0.0, 0.0};
float camDolly = 0.0;
float camPos[3]={0.0, 0.0, 2.0};
float gCamOld = 0.0;
float gCamTrackold[2] = {0.0, 0.0}; 
float oldmousecord[2] = {0.0, 0.0}; 
int selected = 1;

GLUI *glui;
GLUI_EditText *objFileNameTextField;
GLUI_String fileName = GLUI_String("frog.obj");

void drawaxis(GLenum mode,float x, float y, float z);

/************************************************************************/
/*                       FUNCTION DEFINITIONS                           */
/************************************************************************/

/*
 * Reads the contents of the obj file and appends the data at the end of 
 * the vector of Objects. This time we allow the same object to be loaded
 * several times.
 */
int loadObj (char *fileName, Object &obj){
	ifstream file;
	file.open (fileName);
	
	if(!file.is_open ())
	{
		cerr << "Cannot open .obj file " << fileName << endl;
		return EXIT_FAILURE;
	}
	
	obj.name = string(fileName);
	obj.color.r = 1.f;
	obj.color.g = 1.f;
	obj.color.b = 1.f;
	
	/// First pass: count the vertices, normals and faces, allocate the arrays.
	int numVertices = 0, numNormals = 0, numFaces = 0;
	string buffer;
	while (getline(file, buffer), !buffer.empty())
	{
		if (buffer[0] == 'v')
		{
			if (buffer[1] == 'n')
				numNormals++;
			else
			{
				if (buffer[1] == ' ')
					numVertices++;
			}
		}
		else
		{
			if (buffer[0] == 'f')
				numFaces++;
		}
	};
	
	obj.vertices = new float [numVertices][3];
	obj.numVertices = numVertices;
	obj.normals = new float [numNormals][3];
	obj.numNormals = numNormals;
	obj.faces = new Point [numFaces][3];
	obj.numFaces = numFaces;
	obj.translate_matrix[0] = 0.0;
	obj.translate_matrix[1] = 0.0;
	obj.translate_matrix[2] = 0.0;
	obj.scale_matrix[0] = 1.0;
	obj.scale_matrix[1] = 1.0;
	obj.scale_matrix[2] = 1.0;
	obj.rotation_matrix = new float [3][3];
	
	
	for( int x = 0; x < 16; x++)
		obj.objRotMat[x]= 0.0;
	obj.objRotMat[0] = 1.0;
	obj.objRotMat[5]= 1.0;
	obj.objRotMat[10]= 1.0;
	obj.objRotMat[15]= 1.0;
	
	file.clear();
	file.seekg (ios::beg);
	
	/// Second pass: populate the arrays
	numFaces = numNormals = numVertices = 0;
	while (getline(file, buffer), !buffer.empty())
	{
		if (buffer[0] == 'v')
		{
			if (buffer[1] == 'n')
			{
				sscanf(	buffer.data() + 2*sizeof(char), " %f %f %f",	
					   &obj.normals[numNormals][0],
					   &obj.normals[numNormals][1],
					   &obj.normals[numNormals][2]);
				numNormals++;
			}
			else
			{
				if (buffer[1] == ' ')
				{
					sscanf(	buffer.data() + sizeof(char), " %f %f %f",	
						   &obj.vertices[numVertices][0],
						   &obj.vertices[numVertices][1],
						   &obj.vertices[numVertices][2]);
					numVertices++;
				}
			}
		}
		else
		{
			if (buffer[0] == 'f')
			{
				sscanf(	buffer.data() + sizeof(char), " %d//%d %d//%d %d//%d",	
					   &obj.faces[numFaces][0].v, &obj.faces[numFaces][0].n,
					   &obj.faces[numFaces][1].v, &obj.faces[numFaces][1].n,
					   &obj.faces[numFaces][2].v, &obj.faces[numFaces][2].n);
				numFaces++;
			}
		}
	};
	
	file.close();
	cout << "Finished loading " << fileName << endl;
	
	/// Now we generate the display list.
	obj.displayList = glGenLists(1);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
	glNewList(obj.displayList, GL_COMPILE);
	{
		glBegin (GL_TRIANGLES);
		{
			for (int faceNum = 0 ; faceNum < obj.numFaces ; faceNum++)
			{
				glNormal3fv (obj.normals[obj.faces[faceNum][0].n-1]);
				glVertex3fv (obj.vertices[obj.faces[faceNum][0].v-1]);
				glNormal3fv (obj.normals[obj.faces[faceNum][1].n-1]);
				glVertex3fv (obj.vertices[obj.faces[faceNum][1].v-1]);
				glNormal3fv (obj.normals[obj.faces[faceNum][2].n-1]);
				glVertex3fv (obj.vertices[obj.faces[faceNum][2].v-1]);
			}
		}
		glEnd ();
	}
	glEndList ();
	return EXIT_SUCCESS;
}

/*
 * Callback functions
 */

/*
 * Sets up Callbacks for translation, rotatin and scale of the object
 */
void transCB(int id){
	switch(transType)
	{
		case TRANS:
			selected = 1;		 
			break;
		case ROT:
			selected = 2;
			break;
		case SCALE:
			selected = 3;
			break;
		default:break;
	}
	glui->sync_live ();
	glutPostRedisplay ();
}

void projCB(int id){
	setupVV();
	glui->sync_live ();
	glutPostRedisplay ();
}

/*
 * Matrix to vector
 */
void matrixtovector(float matrix[16],float vectorp[3], float vectorw[3] ){
	for (int x = 0; x < 12; x+=4) 
		vectorw[x/4] = matrix[x]*vectorp[0] + matrix[x+1]*vectorp[1] + matrix[x+2]*vectorp[2] + matrix[x+3]*1;
}

void fovCB(int id){	
	projType = PERSP;
	setupVV();
	glui->sync_live ();
}

/*
 * Callback for camera rotation
 */
void camRotationCB(int id){
	float light0_pos[]	=	{0.f, 3.f, 2.f, 0.f}; 
	float diffuse0[]	=	{1.f, 1.f, 1.f, .5f}; 
	float ambient0[]	=	{.1f, .1f, .1f, 1.f}; 
	float specular0[]	=	{1.f, 1.f, 1.f, .5f};
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0); 
	glutPostRedisplay();
}

/*
 * Callback for camera z direction
 */
void dollyCB(int id) {
	
	float changez = camDolly-gCamOld;
	float tempvec[3] = {0.0,0.0,0.0};
	tempvec[2] = changez;
	float tempvec2[3] = {0.0,0.0,0.0};
	matrixtovector(camRotMat, tempvec, tempvec2);
	for (int x = 0; x<3; x++) 
		camPos[x]+=tempvec2[x];
	gCamOld = camDolly;
	
	float light0_pos[]	=	{0.f, 3.f, 2.f, 0.f}; 
	float diffuse0[]	=	{1.f, 1.f, 1.f, .5f}; 
	float ambient0[]	=	{.1f, .1f, .1f, 1.f}; 
	float specular0[]	=	{1.f, 1.f, 1.f, .5f};
	
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0); 
	glutPostRedisplay();
}

/*
 * Callback for traking camera via x and y
 */
void trackXYCB(int id) {
	
	float changex = camTrack[0]-gCamTrackold[0];
	float changey = camTrack[1]-gCamTrackold[1];
	float tempvec[3] = {0.0,0.0,0.0};
	tempvec[0] = changex;
	tempvec[1] = changey;
	float tempvec2[3] = {0.0,0.0,0.0};
	matrixtovector(camRotMat, tempvec, tempvec2);
	for (int x = 0; x<3; x++) 
		camPos[x]+=tempvec2[x];
	gCamTrackold[0]=camTrack[0];
	gCamTrackold[1]=camTrack[1];
	
	float light0_pos[]	=	{0.f, 3.f, 2.f, 0.f}; 
	float diffuse0[]	=	{1.f, 1.f, 1.f, .5f}; 
	float ambient0[]	=	{.1f, .1f, .1f, 1.f}; 
	float specular0[]	=	{1.f, 1.f, 1.f, .5f};
	
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0); 
	glutPostRedisplay();
}

void textCB(int id){
	glui->sync_live ();
	glutPostRedisplay ();
}

/*
 * User enters in object and loads attributes from the object file to the object
 */
void buttonCB(int control){
	Objects.push_back(Object());
	printf("Loading %s\n", objFileNameTextField->get_text());
	loadObj((char *)objFileNameTextField->get_text(), Objects.back());
	glui->sync_live ();
	glutPostRedisplay ();
}

/*
 * Color Callback 
 */
void colorCB(int id){
	glui->sync_live ();
	if (objSelected != -1)
	{
		Objects.at(objSelected).color.r = red;
		Objects.at(objSelected).color.g = green;
		Objects.at(objSelected).color.b = blue;
	}
	glui->sync_live ();
	glutPostRedisplay ();
}

/*
 * Draws x,y and z axis 
 */
void drawaxis(){
	
	glBegin(GL_LINES);
	// draw line for x axis
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(-5.0, 0.0, 0.0);
	glVertex3f(5.0, 0.0, 0.0);
	// draw line for y axis
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, -5.0, 0.0);
	glVertex3f(0.0, 5.0, 0.0);
	// draw line for Z axis
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, -5.0);
	glVertex3f(0.0, 0.0, 5.0);
	glEnd();
	
}

/*
 * Draws cones at the ends of the axis of the objects
 * x cone = -2, y cone = -3, z cone = -4
 */
void drawaxis(GLenum mode,float x, float y, float z){
	
	//Translate the OBJ Cord model
	glPushMatrix();
	
	glTranslated(Objects.at(currentobj).translate_matrix[0], Objects.at(currentobj).translate_matrix[1], Objects.at(currentobj).translate_matrix[2]);
	//Use glutSolidCone to draw the cone
	
	//xCone
	glPushMatrix();
	glScaled(Objects.at(currentobj).scale_matrix[0], 1, 1);
	glLoadName(-2);	
	glColor3f(1, 0, 0);
	glBegin(GL_LINES);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(1.f, 0.f, 0.f);
	glEnd();
	
	glLoadName(-2);	
	glPushMatrix();
	glTranslatef(1.f,0,0);
	glRotatef(90, 0, 1, 0);
	glutSolidCone(0.05f,.15f,20,10);
	glPopMatrix();
	glPopMatrix();	
	
	//yCone
	glPushMatrix();
	glScaled(1, Objects.at(currentobj).scale_matrix[1], 1);
	glLoadName(-3);
	glColor3f(0, 1, 0);
	glBegin(GL_LINES);	
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 1.f, 0.f);
	glEnd();
	
	glLoadName(-3);
	glPushMatrix();
	glTranslatef(0,1.f,0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(0.05f,.15f,20,10);
	glPopMatrix();
	glPopMatrix();
	
	//zCone
	glPushMatrix();
	glScaled(1, 1, Objects.at(currentobj).scale_matrix[2]);
	glLoadName(-4);
	glColor3f(0, 0, 1);
	glBegin(GL_LINES);	
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 0.f, 1.f);
	glEnd();
	
	glLoadName(-4);
	glPushMatrix();
	glTranslatef(0,0,1.f);
	glutSolidCone(0.05f,.15f,20,10);
	glPopMatrix();
	glPopMatrix();
	
	//glLineWidth(1);
	glEnable (GL_LIGHTING);
	glPopMatrix();
	
	glutPostRedisplay ();
}

/*
 * Draws the objects with current gl calls with opengl methods
 */
void drawObjects(GLenum mode)
{
	glMatrixMode(GL_MODELVIEW);
	
	
	if (mode == GL_RENDER){
		glTranslatef(0, 0, (camDolly-2.f));
		glMultMatrixf(camRotMat);
		glTranslatef(0, 0, -(camDolly-2.f));
		glTranslatef(camTrack[0]/sizeX, camTrack[1]/sizeY, camDolly-2.f);
	}
	
	for (int i = 0 ; i < (int) Objects.size() ; i++)
	{
		
		
		if (mode == GL_SELECT)
			glLoadName(i);
		
		if (mode == GL_RENDER)
			glColor3f(Objects.at(i).color.r, Objects.at(i).color.g, Objects.at(i).color.b);
		
		if(objSelected != i)
		{
			glPushMatrix();
		    glTranslatef(Objects.at(i).translate_matrix[0], Objects.at(i).translate_matrix[1], Objects.at(i).translate_matrix[2]);
		    glScalef(Objects.at(i).scale_matrix[0], Objects.at(i).scale_matrix[1], Objects.at(i).scale_matrix[2]);
			glMultMatrixf(Objects.at(i).objRotMat);
			glCallList(Objects.at(i).displayList);
			glPopMatrix();
		}
		
		//translate rot scale draw
		
		if (objSelected == i)//||currentm == -2 || currentm == -3 || currentm == -4)
		{
			glDisable (GL_LIGHTING);
			glEnable (GL_POLYGON_OFFSET_LINE);
			glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
			glPushMatrix();
			//glColor3f (1.f - Objects.at(i).color.r, 1.f - Objects.at(i).color.g, 1.f - Objects.at(i).color.b);
			glColor3f(1.0, 0.0, 0.0);  // Set color to red
			glTranslatef(Objects.at(i).translate_matrix[0], Objects.at(i).translate_matrix[1], Objects.at(i).translate_matrix[2]);
			glScalef(Objects.at(i).scale_matrix[0], Objects.at(i).scale_matrix[1], Objects.at(i).scale_matrix[2]);
			glMultMatrixf(Objects.at(i).objRotMat);
			glCallList (Objects.at(i).displayList);
			
			glPopMatrix();
			glEnable (GL_LIGHTING);
			glDisable (GL_POLYGON_OFFSET_LINE);
			glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
			drawaxis(mode,Objects.at(i).translate_matrix[0], Objects.at(i).translate_matrix[1], Objects.at(i).translate_matrix[2]);
			
			currentobj = objSelected;
		}
		
	}
	drawaxis();
	
	glColor3f(.5,.5,.5);  // Set color to greyish
	glLoadName(-1);
	glBegin(GL_POLYGON);
	glVertex3f(-4, -2, 4);
	glVertex3f(4, -2, 4);
	glVertex3f(4, -2, -4);
	glVertex3f(-4, -2, -4);
	glEnd();
}

/*
 * perspective projection
 */
void setupVV(){
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	
	if (projType == ORTHO)
		glOrtho(vl, vr, vb, vt, vn, vf);
	else{
		float t = vn * tan(fov * 3.1415 /360);
		if(vr <= vt)
			glFrustum( -t, t, -t*vt/vr, t*vt/vr,vn,vf);
		else 
			glFrustum(-t*vr/vt, t*vr/vt, -t,t,vn,vf);
	}
}

/*
 * calls draw objects
 */
void myGlutDisplay(void){
	glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glMultMatrixf(camRotMat);
	glTranslatef(-camPos[0],-camPos[1],-camPos[2]);	
	drawObjects(GL_RENDER);
	glutSwapBuffers();
}

//-------------------------------------------------------------------------
//  This function is passed to the glutReshapeFunc and is called 
//  whenever the window is resized.
//-------------------------------------------------------------------------
void myGlutReshape(int x, int y){
	sizeX = x;
	sizeY = y;
	glViewport(0, 0, sizeX, sizeY);
	
	if(sizeX <= sizeY)
	{
		vb = vl * sizeY / sizeX;
		vt = vr * sizeY / sizeX;
	}
	else
	{
		vl = vb * sizeX / sizeY;
		vr = vt * sizeX / sizeY;
	}
	
	setupVV ();
	glutPostRedisplay ();
}


//-------------------------------------------------------------------------
//  This function is passed to the glutMouseFunc and is called 
//  whenever the mouse is clicked.
//-------------------------------------------------------------------------
void myGlutMouse (int button, int button_state, int x, int y){
	GLuint selectBuffer[512];
	GLint viewport[4];
	
	if(button != GLUT_LEFT_BUTTON || button_state != GLUT_DOWN)
		return;
	
	glGetIntegerv(GL_VIEWPORT, viewport);
	
	glSelectBuffer(512, selectBuffer);
	glRenderMode(GL_SELECT);
	
	glInitNames();
	glPushName(-1); 
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	
	gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3]-y), 5.0, 5.0, viewport);
	
	if (projType == ORTHO)
		glOrtho(vl, vr, vb, vt, vn, vf);
	else{
		float t = vn * tan(fov * 3.1415 /360);
		if(vr <= vt)
			glFrustum( -t, t, -t*vt/vr, t*vt/vr,vn,vf);
		else 
			glFrustum(-t*vr/vt, t*vr/vt, -t,t,vn,vf);
	}
	drawObjects(GL_SELECT);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glFlush();
	
	processHits( glRenderMode(GL_RENDER), selectBuffer);
	
	glutPostRedisplay();
}

/*
 * Calculates the mouse clicks on the object and reports which objects were touched 
 * based on depth.
 */
void processHits(GLint hits, GLuint buffer[]){
	int pick = -1;
	unsigned int min_depth = 4294967295;
	GLuint *ptr = (GLuint *) buffer;
	for(int i = 0 ; i < hits ; i++, ptr+=4)
	{
		if (*(ptr+1) < min_depth)
		{
			min_depth = *(ptr+1);
			pick = *(ptr+3);
		}
	}
	
	if(pick < -1)
		currentm=pick;
	else{
		objSelected = pick; 
		currentm =-1;
	}
	if (pick > -1)
	{
		red = Objects.at(pick).color.r;
		green = Objects.at(pick).color.g;
		blue = Objects.at(pick).color.b;
	}
	else
	{
		red = 1.f;
		green = 1.f;
		blue = 1.f;
	}
	glui->sync_live();
}

/*
 * Translates, Rotates and Scales object with mouse coordinates
 */
void mousecord( int x, int y)
{	
	float changex = (x-oldmousecord[0])/30;
	float changey = (oldmousecord[1]-y)/30;

	if((changex > 2||changex < -2)||( changey>  2||changey< -2)){
		changex = 0;
		changey = 0;
	}
	rotdif += changex;
	
	switch (selected) {
		case 1: //translate
			//x = -1  y = -2  z = -3 
			if (currentm == -2|| currentm == -3||currentm == -4) {
				switch (currentm) {
					case -2:
						Objects.at(currentobj).translate_matrix[0]+=(changex*.05);
						break;
					case -3:
						Objects.at(currentobj).translate_matrix[1]+=(changey*.05);
						break;
					case -4:
						Objects.at(currentobj).translate_matrix[2]+=(changex*.05);
						break;
					default:
						break;
				}
				currentobj=0;
			}
			
			break;
		case 2: //rotate
			//x = -1  y = -2  z = -3 
			if (currentm == -2|| currentm == -3||currentm == -4) {
				switch (currentm) {
					case -2:
						glPushMatrix();
						glLoadIdentity();
						glRotated(rotdif, 1, 0, 0); 
						glMultMatrixf (Objects.at(currentobj).objRotMat);
						glGetFloatv(GL_MODELVIEW_MATRIX, Objects.at(currentobj).objRotMat);
						glPopMatrix();
						break;
					case -3:
						glPushMatrix();
						glLoadIdentity();
						glRotated(rotdif, 0, 1, 0); 
						glMultMatrixf (Objects.at(currentobj).objRotMat);
						glGetFloatv(GL_MODELVIEW_MATRIX, Objects.at(currentobj).objRotMat);
						glPopMatrix();
						break;
					case -4:
						glPushMatrix();
						glLoadIdentity();
						glRotated(rotdif, 0, 0, 1); 
						glMultMatrixf (Objects.at(currentobj).objRotMat);
						glGetFloatv(GL_MODELVIEW_MATRIX, Objects.at(currentobj).objRotMat);
						glPopMatrix();
						break;
					default:
						break;
				}
			}
			
			break;
		case 3: //scale
			//x = -1  y = -2  z = -3 
			if (currentm == -2|| currentm == -3||currentm == -4) {
				switch (currentm) {
					case -2:
						Objects.at(currentobj).scale_matrix[0]+=(changex*.05);
						break;
					case -3:
						Objects.at(currentobj).scale_matrix[1]+=(changey*.05);
						break;
					case -4:
						Objects.at(currentobj).scale_matrix[2]+=(changex*.05);
						break;
					default:
						break;
				}
				currentobj=0;
				
				
			}
			
			break;
		default:
			break;
	}
	oldmousecord[0]=x;
	oldmousecord[1]=y;
	glutPostRedisplay();
}

/*
 * Sets up scene with intial values
 */
void initScene(){
	float light0_pos[]	=	{0.f, 3.f, 2.f, 0.f}; 
	float diffuse0[]	=	{1.f, 1.f, 1.f, .5f}; 
	float ambient0[]	=	{.1f, .1f, .1f, 1.f}; 
	float specular0[]	=	{1.f, 1.f, 1.f, .5f};
	glEnable(GL_LIGHTING); 
	glEnable(GL_LIGHT0); 
	glShadeModel(GL_SMOOTH); 
	glEnable(GL_COLOR_MATERIAL); 
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0); 
	glEnable (GL_DEPTH_TEST);
	setupVV();
}

/*
 * Sets up the opengl loop and calls the main loop
 */
int main(int argc, char **argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);  
	glutInitWindowPosition( 50, 50 );
	glutInitWindowSize( WIN_WIDTH, WIN_HEIGHT);
	
	main_window = glutCreateWindow( "OBJ Loader" );
	glutDisplayFunc( myGlutDisplay );
	glutReshapeFunc(myGlutReshape);  
	glutMouseFunc( myGlutMouse );
	glutMotionFunc(mousecord);
	
	initScene();
	
	glui = GLUI_Master.create_glui( "OBJ Loader GUI", 0, 600, 50 );
	
	GLUI_Panel *objPanel = glui->add_panel("Obj Files");
    //ram  MAC: &fileName  must be changed to 0...or make fileName and array of char
#ifdef MAC
	objFileNameTextField = glui->add_edittext_to_panel(objPanel, "Filename:",GLUI_EDITTEXT_TEXT,0,OBJ_TEXTFIELD,textCB);
#endif
	
#ifdef WINDOWS
	objFileNameTextField = glui->add_edittext_to_panel(objPanel, "Filename:",GLUI_EDITTEXT_TEXT,&fileName,OBJ_TEXTFIELD,textCB);
#endif
	
	glui->add_button_to_panel(objPanel, "Load", LOAD_BUTTON, buttonCB);
	
	glui->add_separator();
	
	GLUI_Panel *transformationsPanel = glui->add_panel ("Object Transformation Mode");
	GLUI_RadioGroup *transGroup = glui->add_radiogroup_to_panel(transformationsPanel, (int *)&transType, -1, transCB);
	glui->add_radiobutton_to_group(transGroup, "Translation");
	glui->add_radiobutton_to_group(transGroup, "Rotation");
	glui->add_radiobutton_to_group(transGroup, "Scale");
	
	
	glui->add_separator ();
	GLUI_Panel *cameraPanel = glui->add_panel("Camera Manipulation Mode");
	GLUI_Rotation *camRotationManip = glui->add_rotation_to_panel(cameraPanel, "Camera Rotation", camRotMat, CAMROTATE,camRotationCB);
	camRotationManip->reset();
	glui->add_column_to_panel(cameraPanel, true);
	GLUI_Translation *trackXYManip = glui->add_translation_to_panel(cameraPanel, "Track XY", GLUI_TRANSLATION_XY, camTrack, TRACK, trackXYCB);
	trackXYManip->set_speed(.005);
	glui->add_column_to_panel(cameraPanel, true);
	GLUI_Translation *dollyManip = glui->add_translation_to_panel(cameraPanel, "Dolly", GLUI_TRANSLATION_Z, &camDolly, DOLLY, dollyCB);
	dollyManip->set_speed(.005);
	glui->add_separator ();
	
	
	GLUI_Panel *projPanel = glui->add_panel("Projection");
	GLUI_RadioGroup *projGroup = glui->add_radiogroup_to_panel(projPanel, (int *)&projType, -1, projCB);
	glui->add_radiobutton_to_group(projGroup, "Orthographic");
	glui->add_radiobutton_to_group(projGroup, "Perspective");
	GLUI_Spinner *fovSpinner = glui->add_spinner_to_panel(projPanel, "FOV", GLUI_SPINNER_INT, &fov, FOV, fovCB);
	fovSpinner->set_int_limits(FOVMIN, FOVMAX);
	
	GLUI_Panel *colorPanel = glui->add_panel("Color");
	GLUI_Spinner *redValue = glui->add_spinner_to_panel(colorPanel, "Red", GLUI_SPINNER_FLOAT, &red, RED, colorCB);
	redValue->set_float_limits(0.f, 1.f);
	
	GLUI_Spinner *greenValue = glui->add_spinner_to_panel(colorPanel, "Green", GLUI_SPINNER_FLOAT, &green,GREEN, colorCB);
	greenValue->set_float_limits(0.f, 1.f);
	
	GLUI_Spinner *blueValue = glui->add_spinner_to_panel(colorPanel, "Blue", GLUI_SPINNER_FLOAT, &blue, BLUE, colorCB);
	blueValue->set_float_limits(0.f, 1.f);
	
	glui->set_main_gfx_window( main_window );
	
	// We register the idle callback with GLUI, *not* with GLUT 
	GLUI_Master.set_glutIdleFunc( NULL );
	glui->sync_live();
	glutMainLoop();
	return EXIT_SUCCESS;
}