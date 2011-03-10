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
#include <stdlib.h> /* Must come first to avoid redef error */
#include <GLUI/GLUI.h>
#include <iostream>
// Header Files
#include <stdio.h>
#include <OpenGL/gl.h>		// Header File For The OpenGL32 Library
#include <OpenGL/glu.h>		// Header File For The GLu32 Library
#include <GLUT/glut.h>		// Header File For The GLut Library
#include <string.h>
#include <vector>
#include <fstream>
using namespace std;

#define NEAR 0.1
#define FAR 50

/** Constants and enums **/
enum buttonTypes {OBJ_TEXTFIELD = 0, LOAD_BUTTON,EXIT_BUTTON=2};
enum colors { RED, GREEN, BLUE};
enum projections { ORTHO, PERSP, FOV};

const int WIN_WIDTH = 500;
const int WIN_HEIGHT = 500;

/** These are the live variables modified by the GUI ***/
int main_window;
int red = 255;
int green = 255;
int blue = 255;
int fov = 30;
int projType = ORTHO; 
GLubyte Lists[10];
int ind =0;
struct Obj * head = NULL;

/** Globals **/
GLUI *glui;
GLUI_EditText *objFileNameTextField;
GLUI_Spinner *redValue;
GLUI_Spinner *greenValue;
GLUI_Spinner *blueValue;
GLUI_RadioGroup *projGroup;
GLUI_Spinner *fovSpinner;

struct Obj
{
	bool select;
	int id;
	//char *name;
	int v1;
	int vn1;
	int f1;
	
	int red;
	int green;
	int blue;
	
	float *v; 
	float *vn; 
	int *face;
	
	Obj * next;
};

float asp = 1;
int f1 = 0, v1 = 0, vn1 = 0;
float vl = -5.0, vr = 5.0, vb = -5.0, vt = 5.0;

//Prototypes
void drawobj(GLenum mode);
void processHits(GLint hits, GLuint buffer[]);


/*
 * Callback functions
 */

/*
 * Projection selection either ortho or perspective
 */
void projCB(int id)
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	if(projType == ORTHO)
		glOrtho(-5, 5, -5, 5, .1, 14);   //l,r, b, t, near, far
	else
	{
		gluPerspective (fov, (GLdouble)asp, 0.1, 20.0);
		gluLookAt (0., 0., 10., 0., 0., 0., 0., 1, 0.);
	}
	glMatrixMode( GL_MODELVIEW );
	glutPostRedisplay();
}

void textCB(int id)
{
	glutPostRedisplay();
}

/*
 * User types in obj file and loads attribute from the file to the object.
 * Parses the file and reads in Vertex, Vertex normals and faces
 */
void buttonCB(int control)
{
	struct  Obj * obj;
	struct  Obj * current= head;
	const char *filename;
	char buff[100];
	char buf[10];
	filename = objFileNameTextField->get_text();
	obj = (struct Obj*)malloc(sizeof(struct Obj));
	obj->select = 0;
	obj->id = ind++;
	obj->v1 =0;
	obj->vn1 =0;
	obj->f1 =0;
	obj->red = 255;
	obj->green = 255;
	obj->blue = 255;
	obj->next = NULL;
	ifstream OpenFile;
	OpenFile.open(filename);
	if (OpenFile.is_open())
		while(OpenFile.getline(buff, 100)){
			strcpy(buf, strtok(buff, " "));
			if(!strcmp(buf, "v"))
				obj->v1++;
			else if(!strcmp(buf, "vn"))
				obj->vn1++;
			else if(!strcmp(buf, "f"))
				obj->f1++;
		}
	else {
		cout << "cant open file!";
		return;
	}
	cout << obj->v1 <<"\n"<<obj->vn1<<"\n"<<obj->f1<<"\n";
	OpenFile.clear();
	OpenFile.seekg(0);
	obj->v = (float*)malloc(obj->v1*sizeof(float)*3);
	obj->vn = (float*)malloc(obj->vn1*sizeof(float)*3);
	obj->face = (int*)malloc(obj->f1*sizeof(int)*6);
	int x = 0, y = 0, z = 0;
	while(OpenFile.getline(buff, 100)){
		strcpy(buf, strtok(buff, " "));
		if(!strcmp(buf, "v")){
			obj->v[x++] = atof(strtok(NULL," "));
			obj->v[x++] = atof(strtok(NULL," "));
			obj->v[x++] = atof(strtok(NULL," "));
		}
		else if(!strcmp(buf, "vn")){
			obj->vn[y++] = atof(strtok(NULL," "));
			obj->vn[y++] = atof(strtok(NULL," "));
			obj->vn[y++]= atof(strtok(NULL," "));
		}	
		else if(!strcmp(buf, "f")){
			obj->face[z++] = atoi(strtok(NULL,"/"));
			obj->face[z++] = atoi(strtok(NULL," /"));
			obj->face[z++] = atoi(strtok(NULL,"/"));
			obj->face[z++] = atoi(strtok(NULL," /"));
			obj->face[z++] = atoi(strtok(NULL,"/"));
			obj->face[z++] = atoi(strtok(NULL," /"));
		}	
	}
	OpenFile.close();
	if(head == NULL)
		head = obj;
	else{
		current = head;
		while(current->next != NULL)
			current = current->next;
		current ->next = obj;
	}
	glutPostRedisplay();
}

/*
 * Color Callback
 */
void colorCB(int id)
{
	struct Obj * current = head;
	while(current != NULL)
	{
		if( current->select == 1 )
		{
			current->red = red;
			current->green = green;
			current->blue = blue;	
		}
		current = current->next;
	}
	glutPostRedisplay();
}


void myGlutDisplay(void)
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
	drawobj(GL_RENDER);
	glutSwapBuffers();
	glFlush();	
}

/*
 * Draw the current object wiht opengl calls, maybe in wire frame.
 */
void drawobj(GLenum mode)
{
	struct  Obj * current=head;
	int i=0,vert1, vert2, vert3, norm1, norm2, norm3, wire=0;
	while (current != NULL){	
		if(mode == GL_SELECT) 
			glLoadName(current->id);
		if(current->select == 1 && wire == 0)
			wire =1;
		else 
			wire = 0;
		if(wire == 1)
		{
			glEnable(GL_POLYGON_OFFSET_LINE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_LIGHTING);
			glColor3f(1.0, 0.0, 0.0);  // Set color to red
		}
		else
			glColor3f(((float) current->red)/255.0f, ((float) current->green)/255.0f, ((float) current->blue)/255.0f);
		
		for(i=0; i<current->f1*6; i=i+6)
		{
			vert1 = (current->face[i]-1)*3, vert2 = (current->face[i+2]-1)*3, vert3 = (current->face[i+4]-1)*3;
			norm1 = (current->face[i+1]-1)*3, norm2 = (current->face[i+3]-1)*3, norm3 = (current->face[i+5]-1)*3;
			glBegin(GL_TRIANGLES);
			glNormal3f(current->vn[norm1],current->vn[norm1+1],current->vn[norm1+2]);
			glVertex3f(current->v[vert1],current->v[vert1+1],current->v[vert1+2]);
			glNormal3f(current->vn[norm2],current->vn[norm2+1],current->vn[norm2+2]);
			glVertex3f(current->v[vert2],current->v[vert2+1],current->v[vert2+2]);
			glNormal3f(current->vn[norm3],current->vn[norm3+1],current->vn[norm3+2]);
			glVertex3f(current->v[vert3],current->v[vert3+1],current->v[vert3+2]);
			glEnd();
		}
		if(wire == 1)
		{
			glEnable(GL_LIGHTING);
			glDisable(GL_POLYGON_OFFSET_LINE);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if(wire== 0)
			current = current->next;
	}
}

//-------------------------------------------------------------------------
//  This function is passed to the glutReshapeFunc and is called 
//  whenever the window is resized.
//-------------------------------------------------------------------------
void myGlutReshape(int w, int h)
{
	//Set viewport to match window!
	glViewport(0,0,w, h);
	// Now lets' make the view volume ar match that of the viewport
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h) {
		vb = vl * (GLfloat)h/ (GLfloat)w;
		vt = vr * (GLfloat) h/ (GLfloat)w;
		glOrtho(vl,vr,vb,vt,NEAR,FAR);          
	} else{
		vl = vb * (GLfloat) w/ (GLfloat)h;
		vr = vt * (GLfloat) w/ (GLfloat)h;
		glOrtho(vl,vr,vb,vt, NEAR, FAR);
	}
	asp = h/w;
	glMatrixMode(GL_MODELVIEW);
}


//-------------------------------------------------------------------------
//  This function is passed to the glutMouseFunc and is called 
//  whenever the mouse is clicked.
//-------------------------------------------------------------------------
void myGlutMouse (int button, int state, int x, int y)
{
	GLuint selectBuffer[512];
	GLint viewport[4];
	GLint hits = 0;
	// Left button wasn't clicked
	if(button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
		return;
	glSelectBuffer(512, selectBuffer); // Holds hits
	glRenderMode(GL_SELECT); // Set selection mode
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();  // Save original matrix
	glLoadIdentity(); // Clean state
	glGetIntegerv(GL_VIEWPORT, viewport); // Get viewport
	gluPickMatrix((GLdouble) x, (GLdouble) (viewport[3]-y), 5.0, 5.0, viewport);
	glInitNames(); // Initialize name stack
	glPushName(0); // Init with unused name	
	if(projType == ORTHO)
		glOrtho(-5, 5, -5, 5, .1, 14);   //l,r, b, t, near, far
	else
	{
		gluPerspective (fov, (GLdouble)asp, 0.1, 20.0);
		gluLookAt (0., 0., 10., 0., 0., 0., 0., 1, 0.);
	}
	drawobj(GL_SELECT); // Draw objects in selection mode
	glMatrixMode(GL_PROJECTION); // Set matrix mode back to projection
	glPopMatrix();		// Restore original matrix
	glFlush();
	
	hits = glRenderMode(GL_RENDER);
	if (hits != 0){
		processHits(hits,selectBuffer);
	}
	glutPostRedisplay();
	
}

/*
 * Calculates the mouse clicks on the object and reports which objects were touched 
 * based on depth.
 */
void processHits(GLint hits, GLuint buffer[])
{
	struct Obj * current = head;
	int i, id, picked_object;
	float current_depth, min_depth;
	GLuint *ptr = (GLuint *) buffer;
	for(i = 0; i < hits; i++)
	{
		// Get depth and ID
		current_depth = (float) *(ptr+1)/0x7fffffff; 
		id = (int) *(ptr+3);
		// Find picked object
		if(i == 0) 
			min_depth =current_depth;
		else 
			min_depth = min_depth;
		
		if(i == 0) 
			picked_object =id;
		else 
			picked_object =picked_object;  
		if(current_depth < min_depth) 
			picked_object = id;
		else
			picked_object = picked_object;	
		// To next hit
		ptr += *ptr + 3; 
	}
	// Set selected in list, update color spinner
	while(current != NULL)
	{
		if(current->id == picked_object) 
			current->select =1; 
		else
			current->select =0;
		if(current->id == picked_object)
		{
			redValue->set_int_val(current->red);
			blueValue->set_int_val(current->blue);
			greenValue->set_int_val(current->green);
		}
		current = current->next;
	}
}

/*
 * Sets up scene with intial values
 */
void initScene()
{
	float light0_pos[] = {0.0, 3.0, 0.0, 1.0}; 
	float diffuse0[] = {1.0, 1.0, 1.0, 0.5}; 
	float ambient0[] = {0.1, 0.1, 0.1, 1.0}; 
	float specular0[] = {1.0, 1.0, 1.0, 0.5};
	glEnable(GL_LIGHTING); 
	glEnable(GL_LIGHT0);
	glEnable(GL_LINE_SMOOTH);
	glShadeModel(GL_SMOOTH); 
	glEnable(GL_COLOR_MATERIAL); 
	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos); 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0); 
	// You need to add the rest of the important GL state inits
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-5, 5, -5, 5, 0, 10);   //l,r, b, t, near, far
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
	glFlush();
}

/*
 * Sets up the opengl loop and calls the main loop
 */
int main(int argc, char **argv)
{
	// setup glut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH );  
	glutInitWindowPosition( 50, 50 );
	glutInitWindowSize( WIN_WIDTH, WIN_HEIGHT);
	//You'll need a handle for your main window for GLUI
	main_window = glutCreateWindow( "OBJ viewer" );
	glutDisplayFunc( myGlutDisplay );
	glutReshapeFunc(myGlutReshape);  
	glutMouseFunc( myGlutMouse );
	// Initialize my Scene
	initScene();
	//Build the GUI
	glui = GLUI_Master.create_glui( "OBJ Options", 0, 600, 50 ); /* name, flags, x, and y */
	GLUI_Panel *objPanel = glui->add_panel("Obj Files");
	objFileNameTextField = glui->add_edittext_to_panel(objPanel, "Filename:",GLUI_EDITTEXT_TEXT,0,OBJ_TEXTFIELD,textCB);
	objFileNameTextField->set_text("Sammi");
	glui->add_button_to_panel(objPanel, "Load", LOAD_BUTTON, buttonCB);
	glui->add_separator();
	GLUI_Panel *projPanel = glui->add_panel("Projection");
	projGroup = glui->add_radiogroup_to_panel(projPanel, &projType, -1,projCB);
	glui->add_radiobutton_to_group(projGroup, "Orthographic");
	glui->add_radiobutton_to_group(projGroup, "Perspective");
	fovSpinner = glui->add_spinner_to_panel(projPanel, "FOV", GLUI_SPINNER_INT, &fov, FOV, projCB);
	fovSpinner->set_int_limits(0, 90);
	GLUI_Panel *colorPanel = glui->add_panel("Color");
	/* These should be done with floats but the speed won't work */
	redValue = glui->add_spinner_to_panel(colorPanel, "Red", 2, &red, RED, colorCB);
	redValue->set_int_limits(0, 255);
	greenValue = glui->add_spinner_to_panel(colorPanel, "Green", 2, &green,GREEN, colorCB);
	greenValue->set_int_limits(0,255);
	blueValue = glui->add_spinner_to_panel(colorPanel, "Blue", 2, &blue, BLUE, colorCB);
	blueValue->set_int_limits(0, 255);
	glui->set_main_gfx_window( main_window );
	GLUI_Master.set_glutIdleFunc( NULL );
	glutMainLoop();
	return EXIT_SUCCESS;
}


