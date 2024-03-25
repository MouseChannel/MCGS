#pragma once
#include "Rendering/ComputePass.hpp"
#include "Wrapper/Image.hpp"
#include "shaders/push_contant.h"
#include <Helper/Instance_base.hpp>
namespace MCGS {
using namespace MCRT;
// class Image;
// class ComputePass;

class GaussianManager : public Instance_base<GaussianManager> {
public:
    void Init();
    // void pre_compute_irradiance();
    // void pre_compute_LUT();
    // void Write_LUT_image();
    // void Write_irradiance_image();

    // auto get_LUT()
    // {
    //     return LUT;
    // }

    // auto get_irradiance()
    // {
    //     return irradiance;
    // }

private:
    void get_gaussian_raw_data();
    std::shared_ptr<Image> render_out;
    std::shared_ptr<Uniform_Stuff<float>> xyz;
    std::shared_ptr<Uniform_Stuff<float>> scale;
    std::shared_ptr<Uniform_Stuff<float>> feature;
    std::shared_ptr<Uniform_Stuff<float>> opacity;
    std::shared_ptr<Uniform_Stuff<float>> rotation;

    std::shared_ptr<Buffer> xyz_buffer;
    std::shared_ptr<Buffer> opacity_buffer;
    std::shared_ptr<Buffer> scale_buffer;
    std::shared_ptr<Buffer> rotation_buffer;
    std::shared_ptr<Buffer> feature_buffer;

    std::shared_ptr<Uniform_Stuff<GS_Address>> address;

    // std::shared_ptr<Uniform_Stuff<int>> radii;
    struct GeometryState {
        GeometryState() = default;
        GeometryState(int size);
        int scan_size;
        std::vector<float> depth_d;
        std::vector<int> clamped_d;
        std::vector<float> radii_d;
        std::vector<float> mean2d_d;
        std::vector<float> cov3d_d;
        std::vector<float> conic_opacity_d;
        std::vector<float> rgb_d;
        std::vector<float> tiles_touched_d;
        std::vector<float> scanning_space_d;
        std::vector<float> point_offsets_d;
        std::shared_ptr<Buffer> depth_buffer;
        std::shared_ptr<Buffer> clamped_buffer;
        std::shared_ptr<Buffer> radii_buffer;
        std::shared_ptr<Buffer> mean2d_buffer;
        std::shared_ptr<Buffer> conv3d_buffer;
        std::shared_ptr<Buffer> conic_opacity_buffer;
        std::shared_ptr<Buffer> rgb_buffer;
        std::shared_ptr<Buffer> tiles_touched_buffer;
        std::shared_ptr<Buffer> point_offsets_buffer;

        // std::shared_ptr<Uniform_Stuff<float>> depth;
        // std::shared_ptr<Uniform_Stuff<float>> clamped;
        // std::shared_ptr<Uniform_Stuff<float>> radii;
        // std::shared_ptr<Uniform_Stuff<float>> mean2d;
        // std::shared_ptr<Uniform_Stuff<float>> conv3d;
        // std::shared_ptr<Uniform_Stuff<float>> conic_opacity;
        // std::shared_ptr<Uniform_Stuff<float>> rgb;
        // std::shared_ptr<Uniform_Stuff<float>> tiles_touched;
        // std::shared_ptr<Uniform_Stuff<float>> point_offsets;
    };
    GeometryState geometry_state;
    std::shared_ptr<ComputePass>
        context;
};
}
