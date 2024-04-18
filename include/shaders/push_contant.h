#ifdef __cplusplus
#pragma once
#include "glm/glm.hpp"
using mat4 = glm::mat4;
using vec3 = glm::vec3;
#endif

#ifdef __cplusplus
#define BEGIN_ENUM(a) enum class a {
#define END_ENUM() }
#else
#define BEGIN_ENUM(a) const uint
#define END_ENUM()
#endif

BEGIN_ENUM(Gaussian_Data_Index)
    xyz_index = 0,
    scale_index = 1,
    feature_index = 2,
    opacity_index = 3,
    rotation_index = 4,

    depth_index = 11,
    clamped_index = 12,
    radii_index = 13,
    mean2d_index = 14,
    conv3d_index = 15,
    conic_opacity = 16,
    rgb_index = 17,
    tiles_touched_index = 18,
    point_offsets_index = 19,
    eAddress = 22,

    render_out_index = 10 END_ENUM();

struct PushContant_GS
{
    mat4 viewMatrix;
    mat4 projMatrix;
    vec3 campos;
    float tanfov;
    int point_num;
};

struct GS_Address
{
    uint64_t xyz_address;
    uint64_t scale_address;
    uint64_t feature_address;
    uint64_t opacity_address;
    uint64_t rotation_address;
    uint64_t depth_address;
    uint64_t clamped_address;
    uint64_t radii_address;
    uint64_t mean2d_address;
    uint64_t conv3d_address;
    uint64_t conic_opacity_address;
    uint64_t rgb_address;
    uint64_t tiles_touched_address;
    uint64_t point_offsets_address;
    // BinningState
    uint64_t point_list_keys_address;
    uint64_t point_list_keys_pingpong_address;
    uint64_t point_list_address;
    uint64_t point_list_pingpong_address;
    // ImageState
    uint64_t ranges_address;
    uint64_t n_contrib_address;
    uint64_t accum_alpha_address;

    uint64_t histograms_address;
};

struct PushContant_Sort
{
    uint g_num_elements;
};
struct PushContant_SortHisgram
{
    uint g_num_elements;
    uint g_shift;
    uint g_num_workgroups;
    uint g_num_blocks_per_workgroup;
};

struct PushContant_Sum
{
    int step;
    uint g_num_elements;
};

struct PushContant_Dup
{
    uint g_num_elements;
};

// struct TT {
//     int a[];
// };
