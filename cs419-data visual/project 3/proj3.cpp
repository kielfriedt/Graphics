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
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif

#include <GL/gl.h>
#include "glut.h"
#include "glui.h"

#ifdef MAC
#include <GLUI/GLUI.h>
#endif
#ifdef WINDOWS
#include <GL/glui.h>  /* On MAC <GLUI/glui.h> */

#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>



//
//	The objective is to draw a colored point cloud
//		with radio buttons
//
//	The left mouse button allows rotation
//	The middle mouse button allows scaling
//	The glui window allows:
//		1. The 3d object to be transformed
//		2. The projection to be changed
//		3. The color of the axes to be changed
//		4. The axes to be turned on and off
//		5. The transformations to be reset
//		6. The program to quit
//
//	Author: Kiel Friedt
//



inline float SQR( float x )
{
	return x * x;
}

// title of these windows:

const char *WINDOWTITLE = { "Project 3 -- KIEL FRIEDT" };
const char *GLUITITLE   = { "GL User Interface Window" };



// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


// Temperature and color settings

const int CUBEROOTOFNODES = 50;
const float COORDRANGE = 2;

const float TEMPMIN = {   0.f };
const float TEMPMAX = { 100.f };

const float GRAYMIN = { 0.20f };
const float GRAYMAX = { 1.00f };

const float XMIN = {-1.0f };
const float XMAX = { 1.0f };

const float YMIN = {-1.0f };
const float YMAX = {1.0f };

const float ZMIN = {-1.0 };
const float ZMAX = { 1.0 };

const float RADMIN = {0};
const float RADMAX = {sqrt(3.0)};

const float GRADMIN = {0.};
const float GRADMAX = {580.};


//project3 variables
char str[128];
float hsv[3];
float rgb[3];

#define TEMP	0
const char *	TEMPFORMAT = { "Temperature: %5.2f - %5.2f" };
float			 TempLowHigh[2];
GLUI_HSlider *		TempSlider;
GLUI_StaticText *	TempLabel;

#define XXX		1
const char *	XFORMAT = { "X-axis: %5.2f - %5.2f" };
float			XLowHigh[2];
GLUI_HSlider *		XSlider;
GLUI_StaticText *	XLabel;

#define YYY		2
const char *	YFORMAT = { "Y-axis: %5.2f - %5.2f" };
float			YLowHigh[2];
GLUI_HSlider *		YSlider;
GLUI_StaticText *	YLabel;

#define ZZZ		3
const char *	ZFORMAT = { "Z-axis: %5.2f - %5.2f" };
float			ZLowHigh[2];
GLUI_HSlider *		ZSlider;
GLUI_StaticText *	ZLabel;

#define RAD		4
const char *	RADFORMAT = { "Radius: %5.2f - %5.2f" };
float			RadLowHigh[2];
GLUI_HSlider *		RadSlider;
GLUI_StaticText *	RadLabel;

#define GRAD		5
const char *	GRADFORMAT = { "Gradient: %5.2f - %5.2f" };
float			GradLowHigh[2];
GLUI_HSlider *		GradSlider;
GLUI_StaticText *	GradLabel;


// structure declaration
struct node
{
	float x, y, z;          // location
	float t;                // temperature
	float rgb[3];		// the assigned color (to be used later)
	float rad;              // radius (to be used later)
	float grad;             // total gradient (to be used later)
};

node ***nodeArray;


// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// able to use the left mouse for either rotation or scaling,
// in case have only a 2-button mouse:

enum LeftButton
{
	ROTATE,
	SCALE
};


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT   = { 4 };
const int MIDDLE = { 2 };
const int RIGHT  = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const float BACKCOLOR[] = { 0., 0., 0., 0. };


// line width for the axes:

const GLfloat AXES_WIDTH   = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};


// the color definitions:
// this order must match the radio button order

const GLfloat Colors[8][3] = 
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };



//
// non-constant global variables:
//

int	ActiveButton;		// current button that is down
GLuint	AxesList;		// list to hold the axes
int	AxesOn;			// != 0 means to draw the axes
int	DebugOn;			// != 0 means to print debugging info
int	DepthCueOn;		// != 0 means to use intensity depth cueing
GLUI *	Glui;			// instance of glui window
int	GluiWindow;		// the glut id for the glui window
int	LeftButton;		// either ROTATE or SCALE
GLuint	PointCloud;		// object display list
int	MainWindow;		// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichColor;		// index into Colors[]
int	WhichProjection;	// ORTHO or PERSP
int	Xmouse, Ymouse;		// mouse values
float	Xrot, Yrot;		// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets




//
// function prototypes:
//

void	Animate( void );
void	Buttons( int );
void	Display( void );
void	DoRasterString( float, float, float, char * );
void	DoStrokeString( float, float, float, float, char * );
float	ElapsedSeconds( void );
void	InitGlui( void );
void	InitGraphics( void );
void	InitLists( void );
void	Keyboard( unsigned char, int, int );
void	MouseButton( int, int, int, int );
void	MouseMotion( int, int );
void	Reset( void );
void	Resize( int, int );
void	Visibility( int );

void	Arrow( float [3], float [3] );
void	Cross( float [3], float [3], float [3] );
float	Dot( float [3], float [3] );
float	Unit( float [3], float [3] );
void	Axes( float );
void	HsvRgb( float[3], float [3] );
void	Buttons( int );
void	Sliders( int );

void	SBM( int, int, int );
void	SBR( int, int, int );
void	SBB( int, int );

//
// main program:
//

int
main( int argc, char *argv[] )
{
	// Set up node structure
	
	// Allocate memory for the 3d node array
	nodeArray = new node**[CUBEROOTOFNODES];
	for (int i = 0; i < CUBEROOTOFNODES; i++) {
		nodeArray[i] = new node*[CUBEROOTOFNODES];
		for (int j = 0; j < CUBEROOTOFNODES; j++){
			nodeArray[i][j] = new node[CUBEROOTOFNODES];
		}
	}
	
	
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)
	
	glutInit( &argc, argv );
	
	
	// setup all the graphics stuff:
	
	InitGraphics();
	
	
	// create the display structures that will not change:
	
	InitLists();
	
	
	// init all the global variables used by Display():
	// this will also post a redisplay
	// it is important to call this before InitGlui()
	// so that the variables that glui will control are correct
	// when each glui widget is created
	
	Reset();
	
	
	// setup all the user interface stuff:
	
	InitGlui();
	
	
	// draw the scene once and wait for some interaction:
	// (will never return)
	
	glutMainLoop();
	
	
	// this is here to make the compiler happy:
	
	return 0;
}



//
// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display() from here -- let glutMainLoop() do it
//

void
Animate( void )
{
	// put animation stuff in here -- change some global variables
	// for Display() to find:
	
	
	
	// force a call to Display() next time it is convenient:
	
	glutSetWindow( MainWindow );
	glutPostRedisplay();
}


//
// glui buttons callback:
//
void
Buttons( int id )
{
	switch( id )
	{
		case RESET:
			Reset();
			sprintf( str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1] );
			TempLabel->set_text( str );
			sprintf( str, XFORMAT, XLowHigh[0], XLowHigh[1] );
			XLabel->set_text( str );
			sprintf( str, YFORMAT, YLowHigh[0], YLowHigh[1] );
			YLabel->set_text( str );
			sprintf( str, ZFORMAT, ZLowHigh[0], ZLowHigh[1] );
			ZLabel->set_text( str );
			sprintf( str, RADFORMAT, RadLowHigh[0], RadLowHigh[1] );
			RadLabel->set_text( str );
			sprintf( str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1] );
			GradLabel->set_text( str );
			Glui->sync_live();
			glutSetWindow( MainWindow );
			glutPostRedisplay();
			break;
			
		case QUIT:
			// gracefully close the glui window:
			// gracefully close out the graphics:
			// gracefully close the graphics window:
			// gracefully exit the program:
			
			Glui->close();
			glutSetWindow( MainWindow );
			glFinish();
			glutDestroyWindow( MainWindow );
			exit( 0 );
			break;
			
		default:
			fprintf( stderr, "Don't know what to do with Button ID %d\n", id );
	}
	
}

/*
 * Slider Callback
 */
void
Sliders( int id )
{
	switch( id )
	{
		case TEMP:
			sprintf( str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1] );
			TempLabel->set_text( str );
			break;
		case XXX:
			sprintf( str, XFORMAT, XLowHigh[0], XLowHigh[1] );
			XLabel->set_text( str );
			break;
		case YYY:
			sprintf( str, YFORMAT, YLowHigh[0], YLowHigh[1] );
			YLabel->set_text( str );
			break;
		case ZZZ:
			sprintf( str, ZFORMAT, ZLowHigh[0], ZLowHigh[1] );
			ZLabel->set_text( str );
			break;
		case RAD:
			sprintf( str, RADFORMAT, RadLowHigh[0], RadLowHigh[1] );
			RadLabel->set_text( str );
			break;
		case GRAD:
			sprintf( str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1] );
			GradLabel->set_text( str );
			break;
	}
	
	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}

//
// draw the complete scene:
//
void
Display( void )
{
	GLsizei vx, vy, v;		// viewport dimensions
	GLint xl, yb;		// lower-left corner of viewport
	GLfloat scale2;		// real glui scale factor
	
	if( DebugOn != 0 )
	{
		fprintf( stderr, "Display\n" );
	}
	
	// set which window we want to do the graphics into:
	
	glutSetWindow( MainWindow );
	
	
	// erase the background:
	
	glDrawBuffer( GL_BACK );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );
	
	
	// specify shading to be flat:
	
	glShadeModel( GL_FLAT );
	
	
	// set the viewport to a square centered in the window:
	
	vx = glutGet( GLUT_WINDOW_WIDTH );
	vy = glutGet( GLUT_WINDOW_HEIGHT );
	v = vx < vy ? vx : vy;			// minimum dimension
	xl = ( vx - v ) / 2;
	yb = ( vy - v ) / 2;
	glViewport( xl, yb,  v, v );
	
	
	// set the viewing volume:
	// remember that the Z clipping  values are actually
	// given as DISTANCES IN FRONT OF THE EYE
	// USE gluOrtho2D() IF YOU ARE DOING 2D !
	
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	if( WhichProjection == ORTHO )
		glOrtho( -3., 3.,     -3., 3.,     0.1, 1000. );
	else
		gluPerspective( 90., 1.,	0.1, 1000. );
	
	
	// place the objects into the scene:
	
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	
	// set the eye position, look-at position, and up-vector:
	// IF DOING 2D, REMOVE THIS -- OTHERWISE ALL YOUR 2D WILL DISAPPEAR !
	
	gluLookAt( 0., 0., 3.,     0., 0., 0.,     0., 1., 0. );
	
	
	// translate the objects in the scene:
	// note the minus sign on the z value
	// this is to make the appearance of the glui z translate
	// widget more intuitively match the translate behavior
	// DO NOT TRANSLATE IN Z IF YOU ARE DOING 2D !
	
	glTranslatef( (GLfloat)TransXYZ[0], (GLfloat)TransXYZ[1], -(GLfloat)TransXYZ[2] );
	
	
	// rotate the scene:
	// DO NOT ROTATE (EXCEPT ABOUT Z) IF YOU ARE DOING 2D !
	
	glRotatef( (GLfloat)Yrot, 0., 1., 0. );
	glRotatef( (GLfloat)Xrot, 1., 0., 0. );
	glMultMatrixf( (const GLfloat *) RotMatrix );
	
	
	// uniformly scale the scene:
	
	glScalef( (GLfloat)Scale, (GLfloat)Scale, (GLfloat)Scale );
	scale2 = 1. + Scale2;		// because glui translation starts at 0.
	if( scale2 < MINSCALE )
		scale2 = MINSCALE;
	glScalef( (GLfloat)scale2, (GLfloat)scale2, (GLfloat)scale2 );
	
	
	// set the fog parameters:
	// DON'T NEED THIS IF DOING 2D !
	
	if( DepthCueOn != 0 )
	{
		glFogi( GL_FOG_MODE, FOGMODE );
		glFogfv( GL_FOG_COLOR, FOGCOLOR );
		glFogf( GL_FOG_DENSITY, FOGDENSITY );
		glFogf( GL_FOG_START, FOGSTART );
		glFogf( GL_FOG_END, FOGEND );
		glEnable( GL_FOG );
	}
	else
	{
		glDisable( GL_FOG );
	}
	
	
	// possibly draw the axes:
	
	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( AxesList );
	}
	
	
	// set the color of the object:
	
	glColor3fv( Colors[WhichColor] );
	
	
	// draw the current object:
	glBegin (GL_POINTS);
	glPointSize(2);
	for(int i=0; i<CUBEROOTOFNODES; i++){
		for(int j=0; j<CUBEROOTOFNODES; j++){
			for(int k=0; k<CUBEROOTOFNODES; k++){
				if(nodeArray[i][j][k].x < XLowHigh[0] || nodeArray[i][j][k].x > XLowHigh[1])
					continue;
				
				else if(nodeArray[i][j][k].y < YLowHigh[0] || nodeArray[i][j][k].y > YLowHigh[1])
					continue;
				
				else if(nodeArray[i][j][k].z < ZLowHigh[0] || nodeArray[i][j][k].z > ZLowHigh[1])
					continue;
				
				else if(nodeArray[i][j][k].t < TempLowHigh[0]  || nodeArray[i][j][k].t >= TempLowHigh[1] )
					continue;
				
				else if(nodeArray[i][j][k].rad < RadLowHigh[0]  || nodeArray[i][j][k].rad > RadLowHigh[1] )
					continue;
				
				else if(nodeArray[i][j][k].grad < GradLowHigh[0] || nodeArray[i][j][k].grad > GradLowHigh[1])
					continue;
				
				glColor3f( nodeArray[i][j][k].rgb[0], nodeArray[i][j][k].rgb[1], nodeArray[i][j][k].rgb[2]);
				glVertex3f(nodeArray[i][j][k].x, nodeArray[i][j][k].y, nodeArray[i][j][k].z);
			}
		}
	}
	glEnd( );
	
	
	// swap the double-buffered framebuffers:
	
	glutSwapBuffers();
	
	
	// be sure the graphics buffer has been sent:
	// note: be sure to use glFlush() here, not glFinish() !
	
	glFlush();
}



//
// use glut to display a string of characters using a raster font:
//
void
DoRasterString( float x, float y, float z, char *s )
{
	char c;			// one character to print
	glRasterPos3f( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutBitmapCharacter( GLUT_BITMAP_TIMES_ROMAN_24, c );
	}
}



//
// use glut to display a string of characters using a stroke font:
//
void
DoStrokeString( float x, float y, float z, float ht, char *s )
{
	char c;			// one character to print
	float sf;		// the scale factor
	
	glPushMatrix();
	glTranslatef( (GLfloat)x, (GLfloat)y, (GLfloat)z );
	sf = ht / ( 119.05 + 33.33 );
	glScalef( (GLfloat)sf, (GLfloat)sf, (GLfloat)sf );
	for( ; ( c = *s ) != '\0'; s++ )
	{
		glutStrokeCharacter( GLUT_STROKE_ROMAN, c );
	}
	glPopMatrix();
}



//
// return the number of seconds since the start of the program:
//
float
ElapsedSeconds( void )
{
	// get # of milliseconds since the start of the program:
	
	int ms = glutGet( GLUT_ELAPSED_TIME );
	
	// convert it to seconds:
	
	return (float)ms / 1000.;
}

struct centers
{
	float xc, yc, zc;       // center location
	float a;                // amplitude
} Centers[] =
{
	{	 1.00f,	 0.00f,	 0.00f,	 90.00f	},
	{	-1.00f,	 0.30f,	 0.00f,	120.00f	},
	{	 0.00f,	 1.00f,	 0.00f,	120.00f	},
	{	 0.00f,	 0.40f,	 1.00f,	170.00f	},
};

/*
 * Sets up temperature values in the cube
 */
float
Temperature( float x, float y, float z )
{
	float t = 0.0;
	
	for( int i = 0; i < 4; i++ )
	{
		float dx = x - Centers[i].xc;
		float dy = y - Centers[i].yc;
		float dz = z - Centers[i].zc;
		float rsqd = SQR(dx) + SQR(dy) + SQR(dz);
		t += Centers[i].a * exp( -5.*rsqd );
	}
	
	if( t > TEMPMAX )
		t = TEMPMAX;
	
	return t;
}

//
// initialize the glui window:
//
void
InitGlui( void )
{
	GLUI_Panel *panel;
	GLUI_RadioGroup *group;
	GLUI_Rotation *rot;
	GLUI_Translation *trans, *scale;
	
	
	// setup the glui window:
	
	glutInitWindowPosition( INIT_WINDOW_SIZE + 50, 0 );
	Glui = GLUI_Master.create_glui( (char *) GLUITITLE );
	
	
	Glui->add_statictext( (char *) GLUITITLE );
	Glui->add_separator();
	
	Glui->add_checkbox( "Axes", &AxesOn );
	
	Glui->add_checkbox( "Perspective", &WhichProjection );
	
	Glui->add_checkbox( "Intensity Depth Cue", &DepthCueOn );
	
	panel = Glui->add_panel(  "Axes Color" );
	group = Glui->add_radiogroup_to_panel( panel, &WhichColor );
	Glui->add_radiobutton_to_group( group, "Red" );
	Glui->add_radiobutton_to_group( group, "Yellow" );
	Glui->add_radiobutton_to_group( group, "Green" );
	Glui->add_radiobutton_to_group( group, "Cyan" );
	Glui->add_radiobutton_to_group( group, "Blue" );
	Glui->add_radiobutton_to_group( group, "Magenta" );
	Glui->add_radiobutton_to_group( group, "White" );
	Glui->add_radiobutton_to_group( group, "Black" );
	
	panel = Glui->add_panel( "Object Transformation" );
	
	rot = Glui->add_rotation_to_panel( panel, "Rotation", (float *) RotMatrix );
	
	// allow the object to be spun via the glui rotation widget:
	
	rot->set_spin( 1.0 );
	
	
	Glui->add_column_to_panel( panel, GLUIFALSE );
	scale = Glui->add_translation_to_panel( panel, "Scale",  GLUI_TRANSLATION_Y , &Scale2 );
	scale->set_speed( 0.005f );
	
	Glui->add_column_to_panel( panel, GLUIFALSE );
	trans = Glui->add_translation_to_panel( panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0] );
	trans->set_speed( 0.05f );
	
	Glui->add_column_to_panel( panel, GLUIFALSE );
	trans = Glui->add_translation_to_panel( panel, "Trans Z",  GLUI_TRANSLATION_Z , &TransXYZ[2] );
	trans->set_speed( 0.05f );
	
	TempLowHigh[0] = TEMPMIN;
	TempLowHigh[1] = TEMPMAX;
	XLowHigh[0] = XMIN;
	XLowHigh[1] = XMAX;
	YLowHigh[0] = YMIN;
	YLowHigh[1] = YMAX;
	ZLowHigh[0] = ZMIN;
	ZLowHigh[1] = ZMAX;
	RadLowHigh[0] = RADMIN;
	RadLowHigh[1] = RADMAX;
	GradLowHigh[0] = GRADMIN;
	GradLowHigh[1] = GRADMAX;
	
	TempSlider = Glui->add_slider( true, GLUI_HSLIDER_FLOAT, TempLowHigh, TEMP, (GLUI_Update_CB) Sliders );
	TempSlider->set_float_limits( TEMPMIN, TEMPMAX );
	TempSlider->set_w( 200 );
	sprintf( str, TEMPFORMAT, TempLowHigh[0], TempLowHigh[1] );
	TempLabel = Glui->add_statictext( str );
	
	XSlider = Glui->add_slider( true, GLUI_HSLIDER_FLOAT, XLowHigh, XXX, (GLUI_Update_CB) Sliders );
	XSlider->set_float_limits( XMIN, XMAX);
	XSlider->set_w ( 200 );
	sprintf( str, XFORMAT, XLowHigh[0], XLowHigh[1] );
	XLabel  = Glui->add_statictext ( str );
	
	YSlider = Glui->add_slider( true, GLUI_HSLIDER_FLOAT, YLowHigh, YYY, (GLUI_Update_CB) Sliders );
	YSlider->set_float_limits( YMIN, YMAX);
	YSlider->set_w ( 200 );
	sprintf( str, YFORMAT, YLowHigh[0], YLowHigh[1] );
	YLabel  = Glui->add_statictext ( str );
	
	ZSlider = Glui->add_slider( true, GLUI_HSLIDER_FLOAT, ZLowHigh, ZZZ, (GLUI_Update_CB) Sliders );
	ZSlider->set_float_limits( ZMIN, ZMAX );
	ZSlider->set_w ( 200 );
	sprintf( str, ZFORMAT, ZLowHigh[0], ZLowHigh[1] );
	ZLabel  = Glui->add_statictext ( str );
	
	RadSlider = Glui->add_slider( true, GLUI_HSLIDER_FLOAT, RadLowHigh, RAD, (GLUI_Update_CB) Sliders );
	RadSlider->set_float_limits( RADMIN, RADMAX);
	RadSlider->set_w ( 200 );
	sprintf( str, RADFORMAT, RadLowHigh[0], RadLowHigh[1] );
	RadLabel  = Glui->add_statictext ( str );
	
	GradSlider = Glui->add_slider( true, GLUI_HSLIDER_FLOAT, GradLowHigh, GRAD, (GLUI_Update_CB) Sliders );
	GradSlider->set_float_limits( GRADMIN, GRADMAX);
	GradSlider->set_w ( 200 );
	sprintf( str, GRADFORMAT, GradLowHigh[0], GradLowHigh[1] );
	GradLabel  = Glui->add_statictext ( str );
	
	
	Glui->add_checkbox( "Debug", &DebugOn );
	
	
	panel = Glui->add_panel( "", GLUIFALSE );
	
	Glui->add_button_to_panel( panel, "Reset", RESET, (GLUI_Update_CB) Buttons );
	
	Glui->add_column_to_panel( panel, GLUIFALSE );
	
	Glui->add_button_to_panel( panel, "Quit", QUIT, (GLUI_Update_CB) Buttons );
	
	
	// tell glui what graphics window it needs to post a redisplay to:
	
	Glui->set_main_gfx_window( MainWindow );
	
	
	// set the graphics window's idle function:
	
	GLUI_Master.set_glutIdleFunc( NULL );
}



//
// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
//
void
InitGraphics( void )
{
	float tempz = 1;
	float dtDx, dtDy, dtDz;
	for(int i = 0; i < CUBEROOTOFNODES; i++){
		float tempy = 1;
		for(int j = 0; j < CUBEROOTOFNODES; j++){
		 	float tempx = 1;
			for(int k = 0; k < CUBEROOTOFNODES; k++){
				nodeArray[i][j][k].x = tempx;
				nodeArray[i][j][k].y = tempy; 
				nodeArray[i][j][k].z = tempz; 
				tempx -= COORDRANGE / (CUBEROOTOFNODES);
				nodeArray[i][j][k].t = Temperature(nodeArray[i][j][k].x, nodeArray[i][j][k].y, nodeArray[i][j][k].z);
				if(nodeArray[i][j][k].t >= TEMPMAX)
					nodeArray[i][j][k].t = (TEMPMAX -1);
				if(nodeArray[i][j][k].t <= TEMPMIN)
					nodeArray[i][j][k].t = (TEMPMIN +1);
				hsv[0] = 240. - (240 * ((nodeArray[i][j][k].t - TEMPMIN)/(TEMPMAX - TEMPMIN)));
				hsv[1] = 1.;
				hsv[2] = 1.;
				HsvRgb( hsv, rgb );
				nodeArray[i][j][k].rad = sqrt(SQR(nodeArray[i][j][k].x) + SQR(nodeArray[i][j][k].y) + SQR(nodeArray[i][j][k].z));
				nodeArray[i][j][k].rgb[0] = rgb[0]; nodeArray[i][j][k].rgb[1] = rgb[1]; nodeArray[i][j][k].rgb[2] = rgb[2];
				
			}
			tempy -= COORDRANGE / (CUBEROOTOFNODES);
		}
		tempz -= COORDRANGE / (CUBEROOTOFNODES); 
	}
	
	
	for(int i = 0; i<CUBEROOTOFNODES; i++)
		for(int j = 0; j<CUBEROOTOFNODES; j++)
			for(int k =0; k <CUBEROOTOFNODES; k++){
				if(!(i == CUBEROOTOFNODES-1 || j == CUBEROOTOFNODES-1 || k == CUBEROOTOFNODES-1 || i == 0 || j == 0 || k == 0)){
					float zeroX = ( nodeArray[i+1][j][k].x - nodeArray[i-1][j][k].x );
					if(zeroX == 0) 
						zeroX = 1;
					float zeroY = ( nodeArray[i][j+1][k].y - nodeArray[i][j-1][k].y );
					if(zeroY == 0) 
						zeroY = 1;
					float zeroZ = ( nodeArray[i][j][k+1].z - nodeArray[i][j][k-1].z );
					if(zeroZ == 0)
						zeroZ = 1;
					dtDx = ( nodeArray[i+1][j][k].t - nodeArray[i-1][j][k].t ) / zeroX;
					dtDy = ( nodeArray[i][j+1][k].t - nodeArray[i][j-1][k].t ) / zeroY;
					dtDz = ( nodeArray[i][j][k+1].t - nodeArray[i][j][k-1].t ) / zeroZ;
					nodeArray[i][j][k].grad = sqrt( SQR(dtDx) + SQR(dtDy) + SQR(dtDz));
					if(nodeArray[i][j][k].grad > GRADMAX) nodeArray[i][j][k].grad = GRADMAX-1;
					
				}
				else{
					if(i == CUBEROOTOFNODES-1)
						nodeArray[i][j][k].grad = nodeArray[i-1][j][k].grad;
					if(j == CUBEROOTOFNODES-1)
						nodeArray[i][j][k].grad = nodeArray[i][j-1][k].grad;
					if(k == CUBEROOTOFNODES-1)
						nodeArray[i][j][k].grad = nodeArray[i][j][k-1].grad;
					if(i == 0 || j == 0 || k == 0){
						if(i == CUBEROOTOFNODES-1)
							dtDx = (nodeArray[i][j][k].t - nodeArray[i][j][k].t / nodeArray[i-11][j][k].x - nodeArray[i][j][k].x);
						else
							dtDx = (nodeArray[i+1][j][k].t - nodeArray[i][j][k].t / nodeArray[i+1][j][k].x - nodeArray[i][j][k].x);
						
						if(j == CUBEROOTOFNODES-1)
							dtDy = (nodeArray[i][j][k].t - nodeArray[i][j][k].t / nodeArray[i][j-1][k].y - nodeArray[i][j][k].y);
						else
							dtDy = (nodeArray[i][j+1][k].t - nodeArray[i][j][k].t / nodeArray[i][j+1][k].y - nodeArray[i][j][k].y);
						
						if(k == CUBEROOTOFNODES-1)
							dtDz = (nodeArray[i][j][k].t - nodeArray[i][j][k].t / nodeArray[i][j][k-1].z - nodeArray[i][j][k].z);
						
						else
							dtDz = (nodeArray[i][j][k+1].t - nodeArray[i][j][k].t / nodeArray[i][j][k+1].z - nodeArray[i][j][k].z);
						
						nodeArray[i][j][k].grad = sqrt( SQR(dtDx) + SQR(dtDy) + SQR(dtDz));
						if(nodeArray[i][j][k].grad > GRADMAX) 
							nodeArray[i][j][k].grad = GRADMAX-1;
					}
				}
			}
	
	
	
	
	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow() )
	// ask for color, double-buffering, and z-buffering:
	
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	
	
	// set the initial window configuration:
	
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( INIT_WINDOW_SIZE, INIT_WINDOW_SIZE );
	
	
	// open the window and set its title:
	
	MainWindow = glutCreateWindow( WINDOWTITLE );
	glutSetWindowTitle( WINDOWTITLE );
	
	
	// setup the clear values:
	
	glClearColor( BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3] );
	
	
	// setup the callback routines:
	
	
	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on
	
	glutSetWindow( MainWindow );
	glutDisplayFunc( Display );
	glutReshapeFunc( Resize );
	glutKeyboardFunc( Keyboard );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutPassiveMotionFunc( NULL );
	glutVisibilityFunc( Visibility );
	glutEntryFunc( NULL );
	glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( SBM );
	glutSpaceballRotateFunc( SBR );
	glutSpaceballButtonFunc( SBB );
	glutButtonBoxFunc( NULL );
	glutDialsFunc( NULL );
	glutTabletMotionFunc( NULL );
	glutTabletButtonFunc( NULL );
	glutMenuStateFunc( NULL );
	glutTimerFunc( 0, NULL, 0 );
	
	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	// glutIdleFunc( NULL );
	// let glui take care of it in InitGlui()
}

void
SBM( int tx, int ty, int tz )
{
	fprintf( stderr, "SBM: %5d, %5d, %5d\n", tx, ty, tz );
}

void
SBR( int rx, int ry, int rz )
{
	fprintf( stderr, "SBR: %5d, %5d, %5d\n", rx, ry, rz );
}

void
SBB( int button, int state )
{
	fprintf( stderr, "SBB: %5d, %5d\n", button, state );
}

//
// initialize the display lists that will not change:
//

void
InitLists( void )
{
	// create the axes:
	
	AxesList = glGenLists( 1 );
	glNewList( AxesList, GL_COMPILE );
	glLineWidth( AXES_WIDTH );
	Axes( 1.5 );
	glLineWidth( 1. );
	glEndList();
}



//
// the keyboard callback:
//
void
Keyboard( unsigned char c, int x, int y )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Keyboard: '%c' (0x%0x)\n", c, c );
	
	switch( c )
	{
		case 'o':
		case 'O':
			WhichProjection = ORTHO;
			break;
			
		case 'p':
		case 'P':
			WhichProjection = PERSP;
			break;
			
		case 'q':
		case 'Q':
		case ESCAPE:
			Buttons( QUIT );	// will not return here
			break;			// happy compiler
			
		case 'r':
		case 'R':
			LeftButton = ROTATE;
			break;
			
		case 's':
		case 'S':
			LeftButton = SCALE;
			break;
			
		default:
			fprintf( stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c );
	}
	
	
	// synchronize the GLUI display with the variables:
	
	Glui->sync_live();
	
	
	// force a call to Display():
	
	glutSetWindow( MainWindow );
	glutPostRedisplay();
}



//
// called when the mouse button transitions down or up:
//
void
MouseButton( int button, int state, int x, int y )
{
	int b;			// LEFT, MIDDLE, or RIGHT
	
	if( DebugOn != 0 )
		fprintf( stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y );
	
	
	// get the proper button bit mask:
	
	switch( button )
	{
		case GLUT_LEFT_BUTTON:
			b = LEFT;		break;
			
		case GLUT_MIDDLE_BUTTON:
			b = MIDDLE;		break;
			
		case GLUT_RIGHT_BUTTON:
			b = RIGHT;		break;
			
		default:
			b = 0;
			fprintf( stderr, "Unknown mouse button: %d\n", button );
	}
	
	
	// button down sets the bit, up clears the bit:
	
	if( state == GLUT_DOWN )
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}



//
// called when the mouse moves while a button is down:
//
void
MouseMotion( int x, int y )
{
	int dx, dy;		// change in mouse coordinates
	
	if( DebugOn != 0 )
		fprintf( stderr, "MouseMotion: %d, %d\n", x, y );
	
	
	dx = x - Xmouse;		// change in mouse coords
	dy = y - Ymouse;
	
	if( ( ActiveButton & LEFT ) != 0 )
	{
		switch( LeftButton )
		{
			case ROTATE:
				Xrot += ( ANGFACT*dy );
				Yrot += ( ANGFACT*dx );
				break;
				
			case SCALE:
				Scale += SCLFACT * (float) ( dx - dy );
				if( Scale < MINSCALE )
					Scale = MINSCALE;
				break;
		}
	}
	
	
	if( ( ActiveButton & MIDDLE ) != 0 )
	{
		Scale += SCLFACT * (float) ( dx - dy );
		
		// keep object from turning inside-out or disappearing:
		
		if( Scale < MINSCALE )
			Scale = MINSCALE;
	}
	
	Xmouse = x;			// new current position
	Ymouse = y;
	
	glutSetWindow( MainWindow );
	glutPostRedisplay();
}



//
// reset the transformations and the colors:
//
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
//
void
Reset( void )
{
	ActiveButton = 0;
	AxesOn = GLUITRUE;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	Scale  = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display()
	WhichColor = WHITE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;
	
	RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0]                   = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1]                   = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3]                   = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;
}



//
// called when user resizes the window:
//
void
Resize( int width, int height )
{
	if( DebugOn != 0 )
		fprintf( stderr, "ReSize: %d, %d\n", width, height );
	
	// don't really need to do anything since window size is
	// checked each time in Display():
	
	glutSetWindow( MainWindow );
	glutPostRedisplay();
}


//
// handle a change to the window's visibility:
//
void
Visibility ( int state )
{
	if( DebugOn != 0 )
		fprintf( stderr, "Visibility: %d\n", state );
	
	if( state == GLUT_VISIBLE )
	{
		glutSetWindow( MainWindow );
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




//////////////////////////////////////////  EXTRA HANDY UTILITIES:  /////////////////////////////

// size of wings as fraction of length:

#define WINGS	0.10


// axes:

#define X	1
#define Y	2
#define Z	3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void
Arrow( float tail[3], float head[3] )
{
	float u[3], v[3], w[3];		// arrow coordinate system
	float d;			// wing distance
	float x, y, z;			// point to plot
	float mag;			// magnitude of major direction
	float f;			// fabs of magnitude
	int axis;			// which axis is the major
	
	
	// set w direction in u-v-w coordinate system:
	
	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];
	
	
	// determine major direction:
	
	axis = X;
	mag = fabs( w[0] );
	if( (f=fabs(w[1]))  > mag )
	{
		axis = Y;
		mag = f;
	}
	if( (f=fabs(w[2]))  > mag )
	{
		axis = Z;
		mag = f;
	}
	
	
	// set size of wings and turn w into a Unit vector:
	
	d = WINGS * Unit( w, w );
	
	
	// draw the shaft of the arrow:
	
	glBegin( GL_LINE_STRIP );
	glVertex3fv( tail );
	glVertex3fv( head );
	glEnd();
	
	// draw two sets of wings in the non-major directions:
	
	if( axis != X )
	{
		Cross( w, axx, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd();
	}
	
	
	if( axis != Y )
	{
		Cross( w, ayy, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd();
	}
	
	
	
	if( axis != Z )
	{
		Cross( w, azz, v );
		(void) Unit( v, v );
		Cross( v, w, u  );
		x = head[0] + d * ( u[0] - w[0] );
		y = head[1] + d * ( u[1] - w[1] );
		z = head[2] + d * ( u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd();
		x = head[0] + d * ( -u[0] - w[0] );
		y = head[1] + d * ( -u[1] - w[1] );
		z = head[2] + d * ( -u[2] - w[2] );
		glBegin( GL_LINE_STRIP );
		glVertex3fv( head );
		glVertex3f( x, y, z );
		glEnd();
	}
}



float
Dot( float v1[3], float v2[3] )
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}



void
Cross( float v1[3], float v2[3], float vout[3] )
{
	float tmp[3];
	
	tmp[0] = v1[1]*v2[2] - v2[1]*v1[2];
	tmp[1] = v2[0]*v1[2] - v1[0]*v2[2];
	tmp[2] = v1[0]*v2[1] - v2[0]*v1[1];
	
	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
Unit( float vin[3], float vout[3] )
{
	float dist, f ;
	
	dist = vin[0]*vin[0] + vin[1]*vin[1] + vin[2]*vin[2];
	
	if( dist > 0.0 )
	{
		dist = sqrt( dist );
		f = 1. / dist;
		vout[0] = f * vin[0];
		vout[1] = f * vin[1];
		vout[2] = f * vin[2];
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}
	
	return dist;
}



// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
	0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
	-.5f, .5f, .5f, -.5f
};

static int xorder[] = {
	1, 2, -3, 4
};


static float yx[] = {
	0.f, 0.f, -.5f, .5f
};

static float yy[] = {
	0.f, .6f, 1.f, 1.f
};

static int yorder[] = {
	1, 2, 3, -2, 4
};


static float zx[] = {
	1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
	.5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[] = {
	1, 2, 3, 4, -5, 6
};


// fraction of the length to use as height of the characters:

const float LENFRAC = 0.10f;


// fraction of length to use as start location of the characters:

const float BASEFRAC = 1.10f;


//
//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)
//

void
Axes( float length )
{
	int i, j;			// counters
	float fact;			// character scale factor
	float base;			// character start location
	
	
	glBegin( GL_LINE_STRIP );
	glVertex3f( length, 0., 0. );
	glVertex3f( 0., 0., 0. );
	glVertex3f( 0., length, 0. );
	glEnd();
	glBegin( GL_LINE_STRIP );
	glVertex3f( 0., 0., 0. );
	glVertex3f( 0., 0., length );
	glEnd();
	
	fact = LENFRAC * length;
	base = BASEFRAC * length;
	
	glBegin( GL_LINE_STRIP );
	for( i = 0; i < 4; i++ )
	{
		j = xorder[i];
		if( j < 0 )
		{
			
			glEnd();
			glBegin( GL_LINE_STRIP );
			j = -j;
		}
		j--;
		glVertex3f( base + fact*xx[j], fact*xy[j], 0.0 );
	}
	glEnd();
	
	glBegin( GL_LINE_STRIP );
	for( i = 0; i < 5; i++ )
	{
		j = yorder[i];
		if( j < 0 )
		{
			
			glEnd();
			glBegin( GL_LINE_STRIP );
			j = -j;
		}
		j--;
		glVertex3f( fact*yx[j], base + fact*yy[j], 0.0 );
	}
	glEnd();
	
	glBegin( GL_LINE_STRIP );
	for( i = 0; i < 6; i++ )
	{
		j = zorder[i];
		if( j < 0 )
		{
			
			glEnd();
			glBegin( GL_LINE_STRIP );
			j = -j;
		}
		j--;
		glVertex3f( 0.0, fact*zy[j], base + fact*zx[j] );
	}
	glEnd();
	
}




//
// routine to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//		"Computer Graphics Principles and Practices,"
//		Additon-Wesley, 1990, pp592-593.


void
HsvRgb( float hsv[3], float rgb[3] )
{
	float h, s, v;			// hue, sat, value
	float r, g, b;			// red, green, blue
	float i, f, p, q, t;		// interim values
	
	
	// guarantee valid input:
	
	h = hsv[0] / 60.;
	while( h >= 6. )	h -= 6.;
	while( h <  0. ) 	h += 6.;
	
	s = hsv[1];
	if( s < 0. )
		s = 0.;
	if( s > 1. )
		s = 1.;
	
	v = hsv[2];
	if( v < 0. )
		v = 0.;
	if( v > 1. )
		v = 1.;
	
	
	// if sat==0, then is a gray:
	
	if( s == 0.0 )
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}
	
	
	// get an rgb from the hue itself:
	
	i = floor( h );
	f = h - i;
	p = v * ( 1. - s );
	q = v * ( 1. - s*f );
	t = v * ( 1. - ( s * (1.-f) ) );
	
	switch( (int) i )
	{
		case 0:
			r = v;	g = t;	b = p;
			break;
			
		case 1:
			r = q;	g = v;	b = p;
			break;
			
		case 2:
			r = p;	g = v;	b = t;
			break;
			
		case 3:
			r = p;	g = q;	b = v;
			break;
			
		case 4:
			r = t;	g = p;	b = v;
			break;
			
		case 5:
			r = v;	g = p;	b = q;
			break;
	}
	
	
	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}
