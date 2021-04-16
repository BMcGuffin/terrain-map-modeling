#version 330 core
in vec2 texCoord;
in vec4 fragCoord;
uniform sampler2D htmap;
uniform sampler2D terrain;

layout(location = 0) out vec4 color;
void main(){
   color = texture(terrain, texCoord);
   //color.y *= color.y;
   //color = fragCoord;	
}
