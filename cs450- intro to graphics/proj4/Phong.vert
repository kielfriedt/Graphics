// Phong vertex shader
//credit for details on how to create this goes to http://www.lighthouse3d.com/opengl/glsl/


varying vec4 diffuse, ambient;
varying vec3 normal, lightDir, halfVector;

void main()
{
   normal = normalize(gl_NormalMatrix * gl_Normal);
   lightDir = normalize(vec3(gl_LightSource[0].position));
   halfVector = normalize(gl_LightSource[0].halfVector.xyz);
   diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
   ambient = gl_FrontMaterial.ambient * (gl_LightSource[0].ambient + gl_LightModel.ambient);
   gl_Position = ftransform();
}