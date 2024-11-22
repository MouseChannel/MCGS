#version 450 core
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_debug_printf : enable

#include "Binding.h"

#include "Shader/Data_struct.h"

layout(location = 0) out vec4 outColor;
layout(location = e_texCoord) in vec2 in_texCoord1;
layout(set = e_graphic, binding = e_offscreen) uniform sampler2D img;
void main()
{
    // debugPrintfEXT("Hewe\n");

    outColor = texture(img, in_texCoord1).rgba;
    // outColor = vec4(in_texCoord1,0,1.f);
}
