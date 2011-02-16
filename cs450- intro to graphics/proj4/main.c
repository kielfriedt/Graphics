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
// Header Files
#include <stdio.h>
/* glut.h includes gl.h and glu.h*/
#include <OpenGL/gl.h>		// Header File For The OpenGL32 Library
#include <OpenGL/glu.h>		// Header File For The GLu32 Library
#include <GLUT/glut.h>		// Header File For The GLut Library
#include <stdlib.h>
#include <string.h>

// Function prototypes
void display();
void initGL();
void HSVtoRGB(float hsv[], float rgb[]);

// global variables
float *grid=NULL;
int indexs[2];

void display() 
{
	float rgb[3], hsv[3] = {0, 1, 1};
	float w, h, temp, x, y = 0; 
	int i, j = 0;
	if(grid != NULL)  	
	{
		glClear(GL_COLOR_BUFFER_BIT);
		w = .98/indexs[0];
		h = .98/indexs[1];
		for(i = 0; i < indexs[1]; i++) 
			for(j = 0; j < indexs[0]; j++)
			{
				temp = grid[i * indexs[0] + j];
				x = j * w -.49;		
				y = i * h - .49;
				if(temp == -1)//white
					glColor3f(255, 255, 255); 
				else
				{
					hsv[0] = 240 - (temp * (240/(20))); 
					HSVtoRGB(hsv, rgb);
					glColor3f(rgb[0], rgb[1], rgb[2]);
				}
				glBegin(GL_POLYGON);	
				glVertex2f(x, y);
				glVertex2f(x+w, y);
				glVertex2f(x+w, y+h);
				glVertex2f(x, y+h);
				glEnd();
				if(j > 0 && (temp != grid[i * indexs[0] + (j-1)]))//side black line
				{
					glColor3f(0, 0, 0);
					glBegin(GL_LINES);
					glVertex3f(x, y, .5);
					glVertex3f(x, y + h, .5);
					glEnd();
				}
				if(i > 0 && (temp != grid[(i-1) * indexs[0] + j]))//bottom black line
				{
					glColor3f(0, 0, 0);
					glBegin(GL_LINES);
					glVertex3f(x, y, .5);
					glVertex3f(x + w, y, .5);
					glEnd();
				}
			}
		glFlush();
	}
}

void initGL() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-0.5, 0.5, -0.5, 0.5);
	glMatrixMode(GL_MODELVIEW);
}

/* 
 * the HSV color model will be as follows
 * h : [0 - 360]
 * s : [0 - 1]
 * v : [0 - 1]
 * If you want it differently (in a 2 * pi scale, 256 instead of 1, etc, 
 * you'll have to change it yourself.
 * rgb is returned in 0-1 scale (ready for color3f)
 */
void HSVtoRGB(float hsv[3], float rgb[3]) {
	float tmp1 = hsv[2] * (1-hsv[1]);
	float tmp2 = hsv[2] * (1-hsv[1] * (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) ));
	float tmp3 = hsv[2] * (1-hsv[1] * (1 - (hsv[0] / 60.0f - (int) (hsv[0]/60.0f) )));
	switch((int)(hsv[0] / 60)) {
		case 0:
			rgb[0] = hsv[2] ;
			rgb[1] = tmp3 ;
			rgb[2] = tmp1 ;
			break;
		case 1:
			rgb[0] = tmp2 ;
			rgb[1] = hsv[2] ;
			rgb[2] = tmp1 ;
			break;
		case 2:
			rgb[0] = tmp1 ;
			rgb[1] = hsv[2] ;
			rgb[2] = tmp3 ;
			break;
		case 3:
			rgb[0] = tmp1 ;
			rgb[1] = tmp2 ;
			rgb[2] = hsv[2] ;
			break;
		case 4:
			rgb[0] = tmp3 ;
			rgb[1] = tmp1 ;
			rgb[2] = hsv[2] ;
			break;
		case 5:
			rgb[0] = hsv[2] ;
			rgb[1] = tmp1 ;
			rgb[2] = tmp2 ;
			break;
		default:
			printf("What!? Inconceivable!\n"); 
	}
}

int main(int argc, char ** argv) {	
	char buff[4092];
	int x,j=0;
	float temp;
	float min,max = 0;
	FILE *output_file;
	if (argc < 2) 
		fprintf(stderr,"usage %s filename\n", argv[0]);
	if ((output_file = fopen(argv[1], "rt")) == NULL)
		fprintf(stderr, "Cannot open %s\n", "output_file");
	else{
		fgets(buff,sizeof buff ,output_file);
		strtok(buff," ");
		indexs[0]= atoi(strtok(NULL," "));
		indexs[1]= atoi(strtok(NULL," "));
		grid =  (float*)malloc(indexs[0]*indexs[1]*sizeof(float));
		for(x = 0; x < indexs[1]; x++) // For each row
		{
			fgets(buff,sizeof buff ,output_file);
			for(j = 0; j < indexs[0]; j++)	 // For each column
			{
				if(j == 0) 
					grid[x*indexs[0]] = atof(strtok(buff, " ")); 
				else
					grid[x * indexs[0] + j] = atof(strtok(NULL, " ")); 
				temp = grid[x * indexs[0] + j];
				if(temp != 0){
					min = min < temp ? min : temp;	// Update min and max as we 
					max = max > temp ? max : temp;	// read through the data
				}
			}
		}
		for(x = 0; x < indexs[1]; x++) // For each row
			for(j = 0; j < indexs[0]; j++)	 // For each column
			{
				temp = grid[x * indexs[0] + j];
				if(temp != 0){
					grid[x * indexs[0] + j]= (int)(((temp-min)/(max-min) * (20 - 1)  + 1.5 ));
					//printf("%d",(int)grid[x * indexs[0] + j]);
				}
				else if (temp == 0.000000){
					grid[x * indexs[0] + j]= -1;
				}
			}
		glutInit(&argc, argv);
		glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
		glutInitWindowSize(500,500);
		glutInitWindowPosition(0,0);
		glutCreateWindow("CS 450/550 Program Skeleton");
		glutDisplayFunc(display);
		initGL();
		glutMainLoop();
		fclose(output_file);
		//free up the malloced memory
		free(grid);
	}
	return 0;
}




