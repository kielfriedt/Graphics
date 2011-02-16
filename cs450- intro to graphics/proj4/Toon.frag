// Toon fragment shader
//credit for details on how to create this goes to http://www.lighthouse3d.com/opengl/glsl/

varying vec3 normal;
	
	void main()
	{
		float intensity;
		vec4 color;
		vec3 n = normalize(normal);
		
		intensity = dot(vec3(gl_LightSource[0].position),n);
		
		if (intensity > 0.95)
			color = gl_FrontLightProduct[0].diffuse * .98;
		else if (intensity > 0.5)
			color = gl_FrontLightProduct[0].diffuse * .75;
		else if (intensity > 0.25)
			color = gl_FrontLightProduct[0].diffuse * .50;
		else
			color = gl_FrontLightProduct[0].diffuse * .25;
		
		gl_FragColor = color;
	} 

		
	
		
	


