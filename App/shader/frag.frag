#version 460

#extension GL_EXT_debug_printf : enable
#extension GL_GOOGLE_include_directive : enable
//#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
//#include "shaders/Data_struct.h"

//layout(location = e_nrm) in vec3 in_nrm;

layout(location = 0) in vec2 in_texCoord;

layout(binding = 1) uniform sampler2D Sampler;
// layout(binding = 3) uniform sampler2D test;
layout(location = 0) out vec4 outColor;
// layout(binding = 4, rgba32f) uniform image2D LUT;
// layout(binding = 5, rgba8_snorm) uniform imageCube irr;

void main()
{

    outColor = pow(texture(Sampler, in_texCoord).rgba, vec4(1. / 1));
}