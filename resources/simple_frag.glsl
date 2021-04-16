#version 330 core 
in vec3 fragNor;
in vec3 WPos;
uniform vec3 MatAmb;
uniform mat4 P;
uniform mat4 MV;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;
uniform vec3 lightPos;

layout(location = 0) out vec4 color;

void main()
{
   vec3 viewVec = normalize(-WPos);

   vec3 lightMV = (MV * vec4(lightPos, 0.0)).xyz;
//   vec3 lightMV = lightPos;

   float dist = distance(lightMV, WPos);

   vec3 lightVec = normalize(lightMV - WPos);

   float diffuse = max(dot(fragNor, lightVec), 0);

   vec3 refVec = normalize(2 * diffuse * fragNor - lightVec);

   float specular = pow(dot(refVec, viewVec), shine);

   color = vec4(MatAmb + (MatDif * diffuse) + (MatSpec * specular), 1.0);
}
