#version  330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 MV;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;
uniform vec3 lightPos;
uniform sampler2D htmap;
uniform float heightscale;

out vec2 texCoord;
out vec4 fragCoord;

void main()
{
   vec3 dummy2 = vertNor * MatAmb * MatDif * MatSpec * shine * lightPos;
	texCoord = vec2(vertPos.x + 1, vertPos.y + 1) / 2.0;
   fragCoord = texture(htmap, texCoord);
   vec4 KMV = vec4(vertPos, 1);
//   KMV.x += fragCoord.x; 
//   KMV.y += fragCoord.y;
   KMV.z = heightscale * fragCoord.z;
   gl_Position = P * MV * KMV;
}
