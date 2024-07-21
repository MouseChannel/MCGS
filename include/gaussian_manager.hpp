#pragma once
#include "Rendering/ComputePass.hpp"
#include "Wrapper/Image.hpp"
#include "shaders/push_contant.h"
#include "sort/multi_sort_pass.hpp"
#include <Helper/Instance_base.hpp>
namespace MCGS {
using namespace MCRT;
// class Image;
class ProcessPass;
class SumPass;
class duplicatePass;
class SortPass;
class IdentifyPass;
class RasterPass;
class CameraManager;
class GaussianManager : public Instance_base<GaussianManager> {
public:
    void Init();
    void Tick();
    auto get_buffer_addr()
    {
        return address;
    }
    auto get_point_num()
    {
        return point_num;
    }
    auto get_camera(){
        return  camera;
    }

    std::shared_ptr<ProcessPass> precess_context;
    std::shared_ptr<CameraManager> camera;
    std::shared_ptr<SumPass>
        sum_context;

    std::shared_ptr<duplicatePass> duplicate_context;
    std::shared_ptr<SortPass> sort_context;
    std::shared_ptr<Multi_SortPass> multi_sort_context;
    ;

    std::shared_ptr<IdentifyPass> identify_content;
    std::shared_ptr<RasterPass> render_content;

private:
    void get_gaussian_raw_data();
    // std::shared_ptr<Image> render_out;
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
    uint point_num;

    // std::shared_ptr<Uniform_Stuff<int>> radii;
    struct GeometryState {
        GeometryState() = default;
        GeometryState(int size);
        int scan_size;
        std::vector<float> depth_d;
        std::vector<int> clamped_d;
        std::vector<int> radii_d;
        std::vector<float> mean2d_d;
        std::vector<float> cov3d_d;
        std::vector<float> conic_opacity_d;
        std::vector<float> rgb_d;
        std::vector<uint> tiles_touched_d;
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
    } geometry_state;

    struct BinningState {
        BinningState() = default;
        BinningState(int size);
        std::vector<uint64_t> point_list_key_d;
        std::vector<uint64_t> point_list_d;
        std::vector<uint64_t> point_list_key_pingpong_d;
        std::vector<uint64_t> point_list_pingpong_d;
        std::vector<uint32_t> histograms_d;

        std::shared_ptr<Buffer> point_list_key_buffer;
        std::shared_ptr<Buffer> point_list_buffer;

        std::shared_ptr<Buffer> point_list_key_pingpong_buffer;
        std::shared_ptr<Buffer> point_list_pingpong_buffer;
        std::shared_ptr<Buffer> histograms_buffer;

    } binning_state;
    struct ImageState {
        ImageState() = default;
        ImageState(int size);
        std::vector<uint32_t> ranges_d;
        std::vector<uint32_t> n_contrib_d;
        std::vector<float> accum_alpha_d;
        std::shared_ptr<Buffer> ranges_buffer;

        std::shared_ptr<Buffer> n_contrib_buffer;

        std::shared_ptr<Buffer> accum_alpha_buffer;

    } image_state;

    // GeometryState geometry_state;

    // std::vector<uint64_t> point_list_keyd;

    // std::vector<uint64_t> point_list_valued;

    // std::shared_ptr<Uniform_Stuff<uint64_t>>
    //     point_list_key;

    // std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_value;
    // conv3d = UniformManager::make_uniform(cov3d_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
};
}
