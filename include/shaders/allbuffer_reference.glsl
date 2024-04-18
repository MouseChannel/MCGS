layout(buffer_reference, scalar) buffer _xyz
{
    vec3 xyz[];
};
layout(buffer_reference, scalar) readonly buffer _opacity
{
    float opacity[];
};
layout(buffer_reference, scalar) readonly buffer _scale
{
    // float scale[];
    vec3 scale[];
};
layout(buffer_reference, scalar) readonly buffer _rotation
{
    // float rotation[];
    vec4 rotation[];
};
layout(buffer_reference, scalar) readonly buffer _feature
{
    // float rotation[];
    vec3 feature[];
};
layout(buffer_reference, scalar) buffer _clamped
{
    // float rotation[];
    int clamped[];
};
layout(buffer_reference, scalar) buffer _rgb
{
    // float rotation[];
    vec3 rgb[];
};
layout(buffer_reference, scalar) buffer _depth
{

    float depth[];
};
layout(buffer_reference, scalar) buffer _depth_uint
{

    uint depth[];
};

layout(buffer_reference, scalar) buffer _radii
{
    int radii[];
};
layout(buffer_reference, scalar) buffer _mean2d
{
    // float rotation[];
    vec2 mean2d[];
};
layout(buffer_reference, scalar) buffer _conic_opacity
{
    // float rotation[];
    vec4 conic_opacity[];
};

layout(buffer_reference, scalar)
    buffer _tiles_touched
{
    // float rotation[];
    uint tiles_touched[];
};
layout(buffer_reference, scalar)
    buffer _point_list
{

    uint64_t point_list[];
};
layout(buffer_reference, scalar)
buffer _histograms
{

    uint histograms[];
};
layout(buffer_reference, scalar)
    buffer _point_list_pingpong
{
    uint64_t point_list_pingpong[];
};
layout(buffer_reference, scalar)
    buffer _point_list_keys
{
    uint64_t point_list_keys[];
};
layout(buffer_reference, scalar)
    buffer _point_list_keys_pingpong
{
    uint64_t point_list_keys_pingpong[];
};

// image

layout(buffer_reference, scalar)
    buffer _ranges
{
    uvec2 ranges[];
};

layout(buffer_reference, scalar)
    buffer _n_contrib
{
    uint n_contrib[];
};

layout(buffer_reference, scalar)
    buffer _accum_alpha
{
    float accum_alpha[];
};