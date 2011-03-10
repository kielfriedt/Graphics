/************************************************************************/
/*						 CS 450 / 550 Assignment4						*/
/*							 Helper Functions							*/
/*							by Christophe Torne							*/
/*								11/11/2010								*/
/************************************************************************/

#include <time.h>
#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>

using namespace std;

struct bmfh
{
	short bfType;
	int bfSize;
	short bfReserved1;
	short bfReserved2;
	int bfOffBits;
};

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
};

struct bmfh FileHeader;
struct bmih InfoHeader;
const int birgb = { 0 };

extern const unsigned WIN_WIDTH;
extern const unsigned WIN_HEIGHT;

//RAM

typedef unsigned char byte;

// Copy the contents of the framebuffer to disk using the BMP file format
int snapshot ()
{
	int windowWidth = WIN_WIDTH, windowHeight = WIN_HEIGHT;
	time_t rawtime;
	struct tm * timeinfo;
	char filename [80];

	time ( &rawtime );
	timeinfo = localtime ( &rawtime );

	strftime (filename, 80, "%A %B %d %Y %Hh%Mm%Ss.bmp",timeinfo);

	byte* bmpBuffer = (byte*)malloc(windowWidth*windowHeight*3);
	if (!bmpBuffer)
		return cout << "Can't allocate bitmap buffer of size" << windowWidth * windowHeight * 3 << endl, system("pause");

	glReadPixels((GLint)0, (GLint)0, (GLint)windowWidth-1, (GLint)windowHeight-1, GL_BGR, GL_UNSIGNED_BYTE, bmpBuffer);

	FILE *filePtr = fopen(filename, "wb");
	if (!filePtr)
		return cout << "Can't open " << filename << endl, system("pause");

	//BITMAPFILEHEADER bitmapFileHeader;
	struct bmfh	bitmapFileHeader;
	
	//BITMAPINFOHEADER bitmapInfoHeader;
	struct bmih bitmapInfoHeader;
	
	bitmapFileHeader.bfType = 0x4D42; //"BM"
	bitmapFileHeader.bfSize = windowWidth*windowHeight*3;
	bitmapFileHeader.bfReserved1 = 0;
	bitmapFileHeader.bfReserved2 = 0;
	bitmapFileHeader.bfOffBits =
		sizeof(struct bmfh) + sizeof(struct bmih);

	bitmapInfoHeader.biSize = sizeof(struct bmih);
	bitmapInfoHeader.biWidth = windowWidth-1;
	bitmapInfoHeader.biHeight = windowHeight-1;
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = 24;
	bitmapInfoHeader.biCompression = birgb;
	bitmapInfoHeader.biSizeImage = 0;
	bitmapInfoHeader.biXPelsPerMeter = 0; // ?
	bitmapInfoHeader.biYPelsPerMeter = 0; // ?
	bitmapInfoHeader.biClrUsed = 0;
	bitmapInfoHeader.biClrImportant = 0;

	fwrite(&bitmapFileHeader, sizeof(struct bmfh), 1, filePtr);
	fwrite(&bitmapInfoHeader, sizeof(struct bmih), 1, filePtr);
	fwrite(bmpBuffer, windowWidth*windowHeight*3, 1, filePtr);
	fclose(filePtr);

	free(bmpBuffer);
	return EXIT_SUCCESS;
}

int ReadInt( FILE *fp )
{
	unsigned char b3, b2, b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	b2 = fgetc( fp );
	b3 = fgetc( fp );
	return ( b3 << 24 )  |  ( b2 << 16 )  |  ( b1 << 8 )  |  b0;
}

short ReadShort( FILE *fp )
{
	unsigned char b1, b0;
	b0 = fgetc( fp );
	b1 = fgetc( fp );
	return ( b1 << 8 )  |  b0;
}

// read a BMP file into a Texture:
unsigned char * BmpToTexture( char *filename, int *width, int *height )
{
	int s, t, e;		// counters
	int numextra;		// # extra bytes each line in the file is padded with
	FILE *fp;
	unsigned char *texture;
	int nums, numt;
	unsigned char *tp;
	//RAM  &fp
	//fopen (fp, filename, "rb" );
	fp = fopen (filename, "rb");
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
	//texture = new unsigned char[ 4 * nums * numt ];
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
			//for( s = 0; s < nums; s++, tp += 4 )
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

void normalize (float *v)
{
	float norm = sqrtf (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	if (norm > 0.f)
	{
		v[0] /= norm;
		v[1] /= norm;
		v[2] /= norm;
	}
}

void cross (float *v1, float *v2, float *res)
{
	res[0] = v1[1] * v2[2] - v1[2] * v2[1];
	res[1] = v1[2] * v2[0] - v1[0] * v2[2];
	res[2] = v1[0] * v2[1] - v1[1] * v2[0];
	normalize(res);
}