#version 460
#extension GL_EXT_debug_printf : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_GOOGLE_include_directive : enable

//#include "shaders/Data_struct.h"
// #include "Data_struct.h"
layout(location = 0) in vec3 in_pos;
//layout(location = e_nrm) in vec3 in_nrm;
//layout(location = e_color) in vec3 in_color;
layout(location = 3) in vec2 in_texCoord;

//layout(location = e_nrm) out vec3 out_nrm;
layout(location = 0) out vec2 out_texCoord;
// layout(push_constant) uniform _PushContant
// {
//     PushContant pc;
// };

void main()
{
    // debugPrintfEXT("message:%d  %f %f %f \n", gl_VertexIndex, inposition.x, inposition.y, inposition.z);
    gl_Position = vec4(in_pos, 1.);
    //    out_nrm = in_nrm;
    out_texCoord = in_texCoord;
}