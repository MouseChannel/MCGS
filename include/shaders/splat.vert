#version 450
#extension GL_EXT_debug_printf : enable

#include "DataStruct.h"
layout(std430, binding = e_instance_point) readonly buffer Instances
{
    Instance instances[]; // (N, 10). 3 for ndc position, 3 for scale rot, 4 for color
};
layout(location = 0) out vec4 out_color;
layout(location = 1) out vec2 out_position;

void main()
{
    // index [0,1,2,2,1,3], 4 vertices for a splat.
    int index = gl_VertexIndex / 4;
    vec3 ndc_position = instances[index].ndc_position;
    // if(index == 0){
    //     debugPrintfEXT("message %f %f %f\n", ndc_position.x, ndc_position.y, ndc_position.z);
    // }
    vec2 scale = instances[index].scale;
    float theta = instances[index].theta;
    vec4 color = instances[index].color;

    // quad positions (-1, -1), (-1, 1), (1, -1), (1, 1), ccw in screen space.
    int vert_index = gl_VertexIndex % 4;
    vec2 position = vec2(vert_index / 2, vert_index % 2) * 2.f - 1.f;

    mat2 rot = mat2(cos(theta), sin(theta), -sin(theta), cos(theta));

    float confidence_radius = 3.f;

    gl_Position = vec4(ndc_position + vec3(rot * (scale * position) * confidence_radius, 0.f), 1.f);
    out_color = color;
    out_position = position * confidence_radius;
}