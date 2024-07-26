#version 460
#extension GL_EXT_debug_printf : enable
#extension GL_GOOGLE_include_directive : enable
vec3[3] positions = vec3[3](vec3(1.0, 1.0, 0.0), vec3(1.0, -3.0, 0.0), vec3(-3.0, 1.0, 0.0));
vec2[3] uvs = vec2[3](vec2(1.0, 1.0), vec2(1.0, -1.0), vec2(-1.0, 1.0));
#include "shaders/Data_struct.h"


layout(location = e_pos) in vec3 inpos;
layout(location = e_texCoord) in vec2 in_texCoord;


layout(location = e_texCoord) out vec2 out_texCoord;
 void rr(){
     
 }
void main()
{

    // out_texCoord = uvs[gl_VertexIndex];
    // gl_Position = vec4(positions[gl_VertexIndex], 1);
    out_texCoord = in_texCoord;
    gl_Position = vec4(inpos,1);
}
