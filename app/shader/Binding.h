#ifdef __cplusplus
#include <glm/glm.hpp>
using mat4 = glm::mat4;

#define BEGIN_ENUM(a) enum class a {
#define END_ENUM() }

#else
#define BEGIN_ENUM(a) const uint
#define END_ENUM()
#endif
// #include "example/base/shaders/raster/Data_struct.h"

BEGIN_ENUM(Graphic_Set)
e_graphic = 0,
    // e_graphic_global = 1,
    graphic_count = 1 END_ENUM();

BEGIN_ENUM(Graphic_Binding)
e_camera_matrix = 0,
e_offscreen = 1
    // e_textures = 1,
    // e_skybox = 2,
    // e_irradiance_image = 3,
    // e_LUT_image = 4,
    // e_albedo_image = 5,
    // e_normal_image = 6,
    // e_m_r_image = 7,
    // e_tonemap_input = 8
    // e_albedo_texture = 9,
    // e_nrm_texture = 10,
    // e_arm_texture = 11
    // e_roughness = 11


    END_ENUM();

// const int e_out_uv = 5;
