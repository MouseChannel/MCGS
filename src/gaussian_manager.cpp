#include "gaussian_manager.hpp"
// #include "Rendering/ComputePass.hpp"
// #include "Wrapper/Image.hpp"
#include "Helper/CommandManager.hpp"
#include "Helper/Model_Loader/ImageWriter.hpp"
#include "Wrapper/Buffer.hpp"
#include "Wrapper/Device.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "chrono"
#include "duplicateWithKeys_pass.hpp"
#include "identify_pass.hpp"
#include "ply_loader.hpp"
#include "precess_pass.hpp"
#include "raster_pass.hpp"
#include "shaders/push_contant.h"
#include "sort_pass.hpp"
#include "sum_pass.hpp"

#include "Helper/cameraManager.hpp"
#include <Wrapper/CommandBuffer.hpp>

namespace MCGS {
// using namespace MCRT;

void GaussianManager::Init()
{
    get_gaussian_raw_data();
    camera.reset(new CameraManager);
    camera->Init();
    precess_context.reset(new ProcessPass);
    // precess_context->set_address(address);
    precess_context->Init();

    sum_context.reset(new SumPass);
    // sum_context.set_address();
    sum_context->Init();
    // duplicate_context.reset(new duplicatePass(point_list_key, point_list_value));
    duplicate_context.reset(new duplicatePass);

    duplicate_context->Init();

    // sort_context.reset(new SortPass(point_list_key, point_list_value));
    // sort_context->Init();
    multi_sort_context.reset(new Multi_SortPass);
    multi_sort_context->Init();

    identify_content.reset(new IdentifyPass);
    identify_content->Init();
    render_content.reset(new RasterPass);
    render_content->Init();

   
    std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
    Tick();
    // Context::Get_Singleton()->get_device()->get_handle().waitIdle();

    // std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
    // auto cpuSortTime = (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end1 - begin1).count()) * std::pow(10, -3));

    // std::cout << "compute shader " << cpuSortTime << "[ms]." << std::endl;
    // //

    // // here
    ImageWriter::WriteImage(render_content->render_out);
    // std::vector<uint64_t> temp(1625771);
    // std::shared_ptr<Buffer> tempbuffer;
    // tempbuffer.reset(new Buffer(temp.size() * sizeof(temp[0]), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible));
    // Buffer::CopyBuffer(binning_state.point_list_key_buffer, tempbuffer);
    // auto temp1 = tempbuffer->Get_mapped_data(0);
    // std::memcpy(temp.data(), temp1.data(), temp.size() * sizeof(temp[0]));
    // for (auto u : temp) {
    //     if (u == 105922436u) {
    //         int asswd = 3;
    //     }
    // }
    // int r = 0;
    // std::sort(binning_state.point_list_key_d.begin(), binning_state.point_list_key_d.end());
    // auto& te = binning_state.point_list_key_d;
    // for (int i = 0; i < te.size(); i++) {
    //     if (te[i] != temp[i]) {
    //         std::cout << "failed" << std::endl;
    //         break;
    //     }
    // }
    // int r12 = 0;
    // std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
    //
    //
    // std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
    // auto cpuSortTime = (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end1 - begin1).count()) * std::pow(10, -3));
    //
    // std::cout << "compute shader " << cpuSortTime << "[ms]." << std::endl;
    //
}

void GaussianManager::Tick()
{
    auto context = precess_context->get_context();
    auto cmd = context->BeginFrame();
    precess_context->run_pass(cmd->get_handle());
    sum_context->run_pass(cmd->get_handle());
    duplicate_context->run_pass(cmd->get_handle());
    multi_sort_context->run_pass(cmd->get_handle());
    identify_content->run_pass(cmd->get_handle());
    render_content->run_pass(cmd->get_handle());
    camera->Tick();
    context->Submit();
}

GaussianManager::GeometryState::GeometryState(int size)
{
    depth_d.resize(size);
    clamped_d.resize(size * 3);
    radii_d.resize(size);
    mean2d_d.resize(size * 2);
    cov3d_d.resize(size * 6);
    conic_opacity_d.resize(size * 4);
    rgb_d.resize(size * 3);
    tiles_touched_d.resize(size);
    point_offsets_d.resize(size);

    auto flag = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
    depth_buffer = Buffer::CreateDeviceBuffer(depth_d.data(),
                                              depth_d.size() * sizeof(depth_d[0]),
                                              flag);
    clamped_buffer = Buffer::CreateDeviceBuffer(clamped_d.data(),
                                                clamped_d.size() * sizeof(clamped_d[0]),
                                                flag);
    radii_buffer = Buffer::CreateDeviceBuffer(radii_d.data(),
                                              radii_d.size() * sizeof(radii_d[0]),
                                              flag);
    mean2d_buffer = Buffer::CreateDeviceBuffer(mean2d_d.data(),
                                               mean2d_d.size() * sizeof(mean2d_d[0]),
                                               flag);
    conv3d_buffer = Buffer::CreateDeviceBuffer(cov3d_d.data(),
                                               cov3d_d.size() * sizeof(cov3d_d[0]),
                                               flag);
    conic_opacity_buffer = Buffer::CreateDeviceBuffer(conic_opacity_d.data(),
                                                      conic_opacity_d.size() * sizeof(conic_opacity_d[0]),
                                                      flag);
    rgb_buffer = Buffer::CreateDeviceBuffer(rgb_d.data(),
                                            rgb_d.size() * sizeof(rgb_d[0]),
                                            flag);
    tiles_touched_buffer = Buffer::CreateDeviceBuffer(tiles_touched_d.data(),
                                                      tiles_touched_d.size() * sizeof(tiles_touched_d[0]),
                                                      flag);
    point_offsets_buffer = Buffer::CreateDeviceBuffer(point_offsets_d.data(),
                                                      point_offsets_d.size() * sizeof(point_offsets_d[0]),
                                                      flag);
}

GaussianManager::BinningState::BinningState(int size)
{
    point_list_d.resize(size);
    point_list_key_d.resize(size);
    point_list_pingpong_d.resize(size);
    point_list_key_pingpong_d.resize(size);
    histograms_d.resize(size);
    auto flag = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eTransferSrc;
    point_list_buffer = Buffer::CreateDeviceBuffer(point_list_d.data(),
                                                   point_list_d.size() * sizeof(point_list_d[0]),
                                                   flag);
    point_list_key_buffer = Buffer::CreateDeviceBuffer(point_list_key_d.data(),
                                                       point_list_key_d.size() * sizeof(point_list_key_d[0]),
                                                       flag);
    point_list_pingpong_buffer = Buffer::CreateDeviceBuffer(point_list_pingpong_d.data(),
                                                            point_list_pingpong_d.size() * sizeof(point_list_pingpong_d[0]),
                                                            flag);
    point_list_key_pingpong_buffer = Buffer::CreateDeviceBuffer(point_list_key_pingpong_d.data(),
                                                                point_list_key_pingpong_d.size() * sizeof(point_list_key_pingpong_d[0]),
                                                                flag);
    histograms_buffer = Buffer::CreateDeviceBuffer(histograms_d.data(),
                                                   histograms_d.size() * sizeof(histograms_d[0]),
                                                   flag);
}

GaussianManager::ImageState::ImageState(int size)
{
    ranges_d.resize(size);
    n_contrib_d.resize(size);
    accum_alpha_d.resize(size);
    auto flag = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR | vk::BufferUsageFlagBits::eTransferSrc;
    ranges_buffer = Buffer::CreateDeviceBuffer(ranges_d.data(),
                                               ranges_d.size() * sizeof(ranges_d[0]),
                                               flag);
    n_contrib_buffer = Buffer::CreateDeviceBuffer(n_contrib_d.data(),
                                                  n_contrib_d.size() * sizeof(n_contrib_d[0]),
                                                  flag);
    accum_alpha_buffer = Buffer::CreateDeviceBuffer(accum_alpha_d.data(),
                                                    accum_alpha_d.size() * sizeof(accum_alpha_d[0]),
                                                    flag);
}

void GaussianManager::get_gaussian_raw_data()
{
    auto gs_data = MCGS::load_ply("point_cloud.ply");
    auto xyz_d = MCGS::get_xyz(gs_data);
    for (int i = 0; i < xyz_d.size() / 3; i++) {

        // glm::make_vec3({ xyz_d[i * 3], xyz_d[i * 3 + 1], xyz_d[i * 3 + 2] });
    }

    std::vector<glm::vec3> xyz_3 {
        xyz_d.begin(),
        xyz_d.end() - 3
    };
    auto scale_d = MCGS::get_scale(gs_data);
    auto dc_012 = MCGS::get_dc_012(gs_data);
    auto dc_rest = MCGS::get_dc_rest(gs_data);

    auto opacity_d = MCGS::get_opacity(gs_data);
    auto rotations_d = MCGS::get_rotation(gs_data);

    std::vector<float> feature_d(dc_012.size() + dc_rest.size());
    for (int i = 0; i < dc_012.size() / 3; i++) {
        for (int j = 0; j < 3; j++) {

            feature_d[i * 48 + j] = dc_012[i * 3 + j];
        }
        for (int j = 0; j < 45; j++) {
            feature_d[i * 48 + 3 + j] = dc_rest[i * 45 + j];
        }
    }
    point_num = opacity_d.size();

    auto flag = vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
    xyz_buffer = Buffer::CreateDeviceBuffer(xyz_d.data(),
                                            xyz_d.size() * sizeof(xyz_d[0]),
                                            flag);
    opacity_buffer = Buffer::CreateDeviceBuffer(opacity_d.data(),
                                                opacity_d.size() * sizeof(opacity_d[0]),
                                                flag);
    scale_buffer = Buffer::CreateDeviceBuffer(scale_d.data(),
                                              scale_d.size() * sizeof(scale_d[0]),
                                              flag);
    rotation_buffer = Buffer::CreateDeviceBuffer(rotations_d.data(),
                                                 rotations_d.size() * sizeof(rotations_d[0]),
                                                 flag);
    feature_buffer = Buffer::CreateDeviceBuffer(feature_d.data(),
                                                feature_d.size() * sizeof(feature_d[0]),
                                                flag);

    geometry_state = GeometryState(point_num);
    // binning_state = BinningState(xyz_d.size() * 10);

    binning_state = BinningState(1625771);
    image_state = ImageState(800 * 800);
    auto addr = GS_Address {
        .xyz_address = xyz_buffer->get_address(),
        .scale_address = scale_buffer->get_address(),
        .feature_address = feature_buffer->get_address(),
        .opacity_address = opacity_buffer->get_address(),
        .rotation_address = rotation_buffer->get_address(),
        .depth_address = geometry_state.depth_buffer->get_address(),
        .clamped_address = geometry_state.clamped_buffer->get_address(),
        .radii_address = geometry_state.radii_buffer->get_address(),
        .mean2d_address = geometry_state.mean2d_buffer->get_address(),
        .conv3d_address = geometry_state.conv3d_buffer->get_address(),
        .conic_opacity_address = geometry_state.conic_opacity_buffer->get_address(),
        .rgb_address = geometry_state.rgb_buffer->get_address(),
        .tiles_touched_address = geometry_state.tiles_touched_buffer->get_address(),
        .point_offsets_address = geometry_state.point_offsets_buffer->get_address(),
        .point_list_keys_address = binning_state.point_list_key_buffer->get_address(),
        .point_list_keys_pingpong_address = binning_state.point_list_key_pingpong_buffer->get_address(),
        .point_list_address = binning_state.point_list_buffer->get_address(),
        .point_list_pingpong_address = binning_state.point_list_pingpong_buffer->get_address(),
        // Image
        .ranges_address = image_state.ranges_buffer->get_address(),
        .n_contrib_address = image_state.n_contrib_buffer->get_address(),
        .accum_alpha_address = image_state.accum_alpha_buffer->get_address(),
        .histograms_address = binning_state.histograms_buffer->get_address()
    };
    address = UniformManager::make_uniform({ addr }, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
}
}