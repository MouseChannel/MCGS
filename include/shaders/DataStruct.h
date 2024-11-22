#ifdef __cplusplus
#pragma once
#include "glm/glm.hpp"

using vec3 = glm::vec3;

using vec4 = glm::vec4;
using vec2 = glm::vec2;
using uint = uint32_t;
using mat4x2 = glm::mat4x2;
using mat4 = glm::mat4;
using uvec2 = glm::uvec2;
using lowp_fvec4 = glm::lowp_fvec4;
#else
#extension GL_EXT_shader_16bit_storage : require
#define lowp_fvec4 = lowp vec4
#endif

#define e_gaussian_raw_point 0
#define e_instance_point 1
#define e_indir_cmd 2
#define e_point_count 3
#define e_camera 4
#define e_instance_key 5
#define e_instance_value 6
#define e_inverse_index 7
// #define e_instance_data 8
struct PointCount {
    uint all_count;
};

struct

    GaussianPoint {
    vec3 pos;
    float deldete1;
    vec3 scale;
    float deldete2;

    vec4 rot;
    // glm::vec4 dd[12];
    vec4 sh[12];
    // lowp_fvec4 sh[12];

    // f16vec4 sh[12];

    // mat2x4 conv3d;
    // mat4x2 conv3d;
    vec4 conv3d[2];

    //
    // float sh[48];

    float opacity;
    float pad1;
    float pad2;
    float pad3;
};

struct InstancePoint {
    vec3 ndc_position;
    float pad0;
    vec2 scale;
    float theta;
    float pad1;
    // lowp_fvec4 sh;
    vec4 color;
};

struct IndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
};
struct CameraInfo {
    mat4 projection;
    mat4 view;
    vec3 camera_position;
    float pad;
    uvec2 screen_size;
};

// struct PushConstants {
//     uint32_t pass;
//     uint64_t elementCountReference;
//     uint64_t globalHistogramReference;
//     uint64_t partitionHistogramReference;
//     uint64_t keysInReference;
//     uint64_t keysOutReference;
//     uint64_t valuesInReference;
//     uint64_t valuesOutReference;
// };