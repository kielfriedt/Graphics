/*
 *	Copyright (C) 2013  Kiel Friedt
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
 #include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include "glui/glui.h"
#include "glui/glut.h"


//	Author: Kiel Friedt


inline float SQR( float x )
{
	return x * x;
}

// title of these windows:

const char *WINDOWTITLE = { "Project 9 -- KIEL FRIEDT" };
const char *GLUITITLE   = { "GL User Interface Window" };



// what the glui package defines as true and false:

const int GLUITRUE  = { true  };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b

// Longitude is west-to-east (x)
// Latitude is south-to-north (-z)

#define NUMLNGS		201
#define NUMLATS		105

// initial window size:

const int INIT_WINDOW_SIZE = { 600 };


const float LNGMIN = { -289.6f   };
const float LNGMAX = {  289.6f   };
const float LATMIN = { -197.5f   };
const float LATMAX = {  211.2f   };
const float HGTMIN = {    0.0f   };
const float HGTMAX = {    2.891f };




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

struct LngLatHgt
	{
		float lng, lat, hgt;
	};

struct LngLatHgt Points[NUMLATS][NUMLNGS];


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE     = { GL_LINEAR };
const GLfloat FOGDENSITY  = { 0.30f };
const GLfloat FOGSTART    = { 1.5 };
const GLfloat FOGEND      = { 4. };



//
// non-constant global variables:
//

double TimeStep = .01;

int	ActiveButton;		// current button that is down
GLuint	AxesList;		// list to hold the axes
GLuint TerrainList;
int	AxesOn;			// != 0 means to draw the axes
int Cloud;
int	DebugOn;			// != 0 means to print debugging info
int	DepthCueOn;		// != 0 means to use intensity depth cueing
GLUI *	Glui;			// instance of glui window
int	GluiWindow;		// the glut id for the glui window
int	LeftButton;		// either ROTATE or SCALE
GLuint	PointCloud;		// object display list
int	MainWindow;		// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichTexture;		// index into 
int	WhichFilter;		// index into 
int	WhichColor;		// index into Colors[]
int	WhichProjection;	// ORTHO or PERSP
int	Xmouse, Ymouse;		// mouse values
float	Xrot, Yrot;		// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets
bool Paused;
//
// function prototypes:
//
void Vector( float x, float y, float z,   float *vxp, float *vyp, float *vzp );
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
void	Axes( float );
void	Cross( float [3], float [3], float [3] );
float	Dot( float [3], float [3] );
float	Unit( float [3], float [3] );
void	Buttons( int );
unsigned char *BmpToTexture( char *filename, int *widthp, int *heightp );
void Terrain();
unsigned char *BmpToTexture( char *, int *, int * );
int	ReadInt( FILE * );
short	ReadShort( FILE * );

void	SBM( int, int, int );
void	SBR( int, int, int );
void	SBB( int, int );
void CloudAnimate();

static float ypoz = 0, zpoz = 0;



struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
} FileHeader;

struct bmih
{
	int biSize;
	int biWidth;
	int biHeight;
	short biPlanes;
	short biBitCount;
	int biCompression;
	int biSizeImage;
	int biXPelsPerMeter;
	int biYPelsPerMeter;
	int biClrUsed;
	int biClrImportant;
} InfoHeader;

const int birgb = { 0 };

//
// main program:
//

int
	main( int argc, char *argv[] )
{
	// Set up node structure

	// Allocate memory for the 3d node array



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

glutIdleFunc(CloudAnimate);
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

void
	Sliders( int id )
{
	switch( id )
	{	

	}

	glutSetWindow( MainWindow );
	glutPostRedisplay( );
}





/**
 ** read a BMP file into a Texture:
 **/

unsigned char *
BmpToTexture( char *filename, int *width, int *height )
{

	int s, t, e;		// counters
	int numextra;		// # extra bytes each line in the file is padded with
	FILE *fp;
	unsigned char *texture;
	int nums, numt;
	unsigned char *tp;


	fp = fopen( filename, "rb" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open Bmp file '%s'\n", filename );
		return NULL;
	}

	FileHeader.bfType = ReadShort( fp );


	// if bfType is not 0x4d42, the file is not a bmp:

	if( FileHeader.bfType != 0x4d42 )
	{
		fprintf( stderr, "Wrong type of file: 0x%0x\n", FileHeader.bfType );
		fclose( fp );
		return NULL;
	}


	FileHeader.bfSize = ReadInt( fp );
	FileHeader.bfReserved1 = ReadShort( fp );
	FileHeader.bfReserved2 = ReadShort( fp );
	FileHeader.bfOffBits = ReadInt( fp );


	InfoHeader.biSize = ReadInt( fp );
	InfoHeader.biWidth = ReadInt( fp );
	InfoHeader.biHeight = ReadInt( fp );

	nums = InfoHeader.biWidth;
	numt = InfoHeader.biHeight;

	InfoHeader.biPlanes = ReadShort( fp );
	InfoHeader.biBitCount = ReadShort( fp );
	InfoHeader.biCompression = ReadInt( fp );
	InfoHeader.biSizeImage = ReadInt( fp );
	InfoHeader.biXPelsPerMeter = ReadInt( fp );
	InfoHeader.biYPelsPerMeter = ReadInt( fp );
	InfoHeader.biClrUsed = ReadInt( fp );
	InfoHeader.biClrImportant = ReadInt( fp );


	// fprintf( stderr, "Image size found: %d x %d\n", ImageWidth, ImageHeight );


	texture = new unsigned char[ 3 * nums * numt ];
	if( texture == NULL )
	{
		fprintf( stderr, "Cannot allocate the texture array!\b" );
		return NULL;
	}


	// extra padding bytes:

	numextra =  4*(( (3*InfoHeader.biWidth)+3)/4) - 3*InfoHeader.biWidth;


	// we do not support compression:

	if( InfoHeader.biCompression != birgb )
	{
		fprintf( stderr, "Wrong type of image compression: %d\n", InfoHeader.biCompression );
		fclose( fp );
		return NULL;
	}
	


	rewind( fp );
	fseek( fp, 14+40, SEEK_SET );

	if( InfoHeader.biBitCount == 24 )
	{
		for( t = 0, tp = texture; t < numt; t++ )
		{
			for( s = 0; s < nums; s++, tp += 3 )
			{
				*(tp+2) = fgetc( fp );		// b
				*(tp+1) = fgetc( fp );		// g
				*(tp+0) = fgetc( fp );		// r
			}

			for( e = 0; e < numextra; e++ )
			{
				fgetc( fp );
			}
		}
	}

	fclose( fp );

	*width = nums;
	*height = numt;
	return texture;
}



int
ReadInt( FILE *fp )
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	b2 = fgetc( fp );
	b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}


short
ReadShort( FILE *fp )
{
	unsigned char b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}


void CloudAnimate()
{
	ypoz+=0.5;
if (ypoz>360) ypoz=0;

// Normally openGL doesn't continuously draw frames. It puts one in place and waits for you to tell him what to do next.
// Calling glutPostRedisplay() forces a redraw with the new angle
glutPostRedisplay();
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

// possibly draw the axes:

	if( AxesOn != 0 )
	{
		glColor3fv( &Colors[WhichColor][0] );
		glCallList( AxesList );
	}



	

	

	

glCallList(TerrainList);


//float normx,normy,normz;
if(Cloud != 0){
//glLoadIdentity();

glTranslatef(0,2,0);
glTranslatef(NUMLATS/2, 0,NUMLNGS/2);
// Lets do some rotations here
// ypoz gets changed by the animation function and zpoz only by keyboard.
// remember you can press 'y' and 'z' to play with these
// The angle is in degrees (0..360)


glRotatef(ypoz,0.0f,1.0f,0.0f);


// I'm going to draw something simple here as we haven't yet discussed loading models
glColor3f(1,0,0);
glBegin(GL_TRIANGLES);

    glVertex3f(0,0,3);
    glVertex3f(3,0,0);
    glVertex3f(-3,0,0);

glEnd();

}






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

	Glui->add_checkbox( (char*)"Axes", &AxesOn );
	Glui->add_checkbox((char*) "Perspective", &WhichProjection );
	Glui->add_checkbox( (char*)"Intensity Depth Cue", &DepthCueOn );
	Glui->add_checkbox( (char*)"Cloud", &Cloud );


	Glui->add_separator();

	panel = Glui->add_panel( (char*) "Texture Filtering" );
	group = Glui->add_radiogroup_to_panel( panel, &WhichTexture );
	Glui->add_radiobutton_to_group( group,(char*) "GL_NEAREST" );
	Glui->add_radiobutton_to_group( group,(char*) "GL_LINEAR" );

	panel = Glui->add_panel( (char*) "Texture Environment" );
	group = Glui->add_radiogroup_to_panel( panel, &WhichFilter );
	Glui->add_radiobutton_to_group( group,(char*) "No Texture" );
	Glui->add_radiobutton_to_group( group,(char*) "GL_REPLACE" );
	Glui->add_radiobutton_to_group( group,(char*) "RGL_MODULATE" );
	
	

	Glui->add_separator();
	panel = Glui->add_panel((char *) "Object Transformation" );
	rot = Glui->add_rotation_to_panel( panel,(char *) "Rotation", (float *) RotMatrix );

	rot->set_spin( 1.0 );

	Glui->add_column_to_panel( panel, GLUIFALSE );
	scale = Glui->add_translation_to_panel( panel,(char *) "Scale",  GLUI_TRANSLATION_Y , &Scale2 );
	scale->set_speed( 0.005f );

	Glui->add_column_to_panel( panel, GLUIFALSE );
	trans = Glui->add_translation_to_panel( panel,(char *) "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0] );
	trans->set_speed( 0.05f );

	Glui->add_column_to_panel( panel, GLUIFALSE );
	trans = Glui->add_translation_to_panel( panel, (char *)"Trans Z",  GLUI_TRANSLATION_Z , &TransXYZ[2] );
	trans->set_speed( 0.05f );


	Glui->add_checkbox( (char *)"Debug", &DebugOn );


	panel = Glui->add_panel((char *) "", GLUIFALSE );

	Glui->add_button_to_panel( panel, (char *)"Reset", RESET, (GLUI_Update_CB) Buttons );

	Glui->add_column_to_panel( panel, GLUIFALSE );

	Glui->add_button_to_panel( panel, (char *)"Quit", QUIT, (GLUI_Update_CB) Buttons );


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
	
	FILE *fp = fopen( "or.hgt", "r" );
	if( fp == NULL )
	{
		fprintf( stderr, "Cannot open 'or.hgt'\n" );
		exit( 1 );
	}

	float lat, lng;
	float dlat, dlng;
	int z, x;

	dlng = (abs(LNGMIN)+ abs(LNGMAX))/NUMLNGS;
	dlat = (abs(LATMIN)+ abs(LATMAX))/NUMLATS;

	for( z = 0, lat = LATMAX; z < NUMLATS; z++, lat -= dlat )
		for( x = 0, lng = LNGMIN; x < NUMLNGS; x++, lng += dlng )
		{
			Points[z][x].lng = lng;
			Points[z][x].lat = lat;
			fscanf( fp, "%f", &Points[z][x].hgt );
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
	//glutPassiveMotionFunc( NULL );
	//glutVisibilityFunc( Visibility );
	//glutEntryFunc( NULL );
	//glutSpecialFunc( NULL );
	glutSpaceballMotionFunc( SBM );
	glutSpaceballRotateFunc( SBR );
	glutSpaceballButtonFunc( SBB );
	//glutButtonBoxFunc( NULL );
	//glutDialsFunc( NULL );
	//glutTabletMotionFunc( NULL );
	//glutTabletButtonFunc( NULL );
	//glutMenuStateFunc( NULL );
	//glutTimerFunc( 0, NULL, 0 );

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
TerrainList = glGenLists(1);
	glNewList(TerrainList,GL_COMPILE);
	Terrain();
glEndList();
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
		    Paused = ! Paused;
        if( Paused )
                GLUI_Master.set_glutIdleFunc( NULL );
        else
                GLUI_Master.set_glutIdleFunc( Animate );
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
	AxesOn=GLUIFALSE;
	ActiveButton = 0;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	Cloud = GLUITRUE;
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


void Arrow( float tail[3], float head[3] )
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


void Terrain()
{
	
	for(int z = 0; z < NUMLATS-1; z++ )
		for( int x = 0; x < NUMLNGS; x++){
				glBegin(GL_TRIANGLE_STRIP);
glColor3fv( &Colors[WHITE][0] );			
glVertex3f(z, Points[z][x].hgt, x);
glColor3fv( &Colors[GREEN][0] );
glVertex3f(z, Points[z][x + 1].hgt, x+1);
glColor3fv( &Colors[WHITE][0] );
glVertex3f(z+1, Points[z + 1][x].hgt, x);
glColor3fv( &Colors[GREEN][0] );
glVertex3f(z+1, Points[z + 1][x + 1].hgt, x+1);
glEnd();	
}
}

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
