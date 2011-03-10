#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996) //This one is annoying
#endif

#include <ctype.h>
#define _USE_MATH_DEFINES
#include <stdio.h>
#include <math.h>

#ifdef WIN32
#include "gl/glee.h"
#include <gl/glut.h>
#else
#include <GLUT/glut.h>
#endif

#include <map>

void            CheckGlErrors( const char* );

#ifdef NOTDEF
#ifndef INLINE_OSU
#define INLINE_OSU
inline void glGetShaderivOSU( GLuint shader, GLenum what, int *value )
{
	glGetObjectParameterivARB( shader, what, value );
}

inline void glGetShaderInfoLogOSU( GLuint shader, int len, GLsizei *ptr, GLchar * log )
{
	glGetInfoLogARB( shader, len, ptr, log );
}

inline void glGetProgramivOSU( GLuint Program, GLenum what, int *value )
{
	glGetObjectParameterivARB( Program, what, value );
}

inline void glGetProgramInfoLogOSU( GLuint Program, int len, GLsizei *ptr, GLchar * log )
{
	glGetInfoLogARB( Program, len, ptr, log );
}
#endif
#endif

inline int GetOSU( int flag )
{
	int i;
	glGetIntegerv( flag, &i );
	return i;
}


void	CheckGlErrors( const char* );




class GLSLProgram
{
  private:
	std::map<char *, int>	AttributeLocs;
	char *					Ffile;
	unsigned int			Fshader;
	char *					Gfile;
	unsigned int			Gshader;
	GLenum					InputTopology;
	GLenum					OutputTopology;
	int						Program;
	char *					Vfile;
	unsigned int			Vshader;
	std::map<char *, int>	UniformLocs;
	bool					Verbose;

	static int				CurrentProgram;

	void	AttachShader( GLuint );
	bool	CanDoFragmentShader;
	bool	CanDoGeometryShader;
	bool	CanDoVertexShader;
	int		CompileShader( GLuint );
	void	Create( char *, char *, char * );
	int		GetAttributeLocation( char * );
	int		GetUniformLocation( char * );
	int		LinkProgram( );
	GLuint	LoadFragmentShader( char * );
	GLuint	LoadGeometryShader( char * );
	GLuint	LoadVertexShader( char * );
	int		LoadShader( const char *, GLuint );

  public:
	GLSLProgram( );
	GLSLProgram( char *, char *, char * );
	GLSLProgram( char *, char * );

	bool		IsExtensionSupported( const char * );
	void		SetAttribute( char *, int );
	void		SetAttribute( char *, float );
	void		SetInputTopology( GLenum );
	void		SetOutputTopology( GLenum );
	void		SetUniform( char *, int );
	void		SetUniform( char *, float );
	void		SetVerbose( bool );
	void		Use( );
	void		Use( int );

	static void	UseFixedFunction( );
};
