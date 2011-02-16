// Toon vertex shader
//credit for details on how to create this goes to http://www.lighthouse3d.com/opengl/glsl/

varying vec3 normal;
	
	void main()
	{

		normal = gl_NormalMatrix * gl_Normal;
	
		gl_Position = ftransform();
	} 
