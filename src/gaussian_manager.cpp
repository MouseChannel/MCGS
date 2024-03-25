#include "gaussian_manager.hpp"
// #include "Rendering/ComputePass.hpp"
// #include "Wrapper/Image.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/Buffer.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "ply_loader.hpp"
#include "shaders/push_contant.h"
namespace MCGS {
// using namespace MCRT;

void GaussianManager::Init()
{
    get_gaussian_raw_data();
    render_out.reset(new Image(800,
                               800,
                               // vk::Format::eR32G32B32A32Sfloat,
                               vk::Format::eR8G8B8A8Unorm,
                               vk::ImageType::e2D,
                               vk::ImageTiling::eOptimal,
                               vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
                               vk::ImageAspectFlagBits::eColor,
                               vk::SampleCountFlagBits::e1));
    render_out->SetImageLayout(vk::ImageLayout::eGeneral,
                               vk::AccessFlagBits::eNone,
                               vk::AccessFlagBits::eNone,
                               vk::PipelineStageFlagBits::eTopOfPipe,
                               vk::PipelineStageFlagBits::eBottomOfPipe);

    context.reset(new ComputePass);
    context->set_constants_size(sizeof(PushContant_GS));
    context->prepare();
    context->prepare_descriptorset([&]() {
        auto descriptor_manager = context->get_descriptor_manager();
        // descriptor_manager->Make_DescriptorSet(xyz,
        //                                        (int)Gaussian_Data_Index::xyz_index,
        //                                        DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(scale,
        //                                        (int)Gaussian_Data_Index::scale_index,
        //                                        DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(feature,
        //                                        (int)Gaussian_Data_Index::feature_index,
        //                                        DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(opacity,
        //                                        (int)Gaussian_Data_Index::opacity_index,
        //                                        DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(rotation,
        //                                        (int)Gaussian_Data_Index::rotation_index,
        //                                        DescriptorManager::Compute);
        descriptor_manager->Make_DescriptorSet(render_out,
                                               DescriptorManager::Compute,
                                               (int)Gaussian_Data_Index::render_out_index,
                                               vk::DescriptorType::eStorageImage,
                                               vk::ShaderStageFlagBits::eCompute);

        descriptor_manager->Make_DescriptorSet(address,
                                               (int)Gaussian_Data_Index::eAddress,
                                               DescriptorManager::Compute);
    });

    std::shared_ptr<ShaderModule>
        compute_shader {
            new ShaderModule("/home/mocheng/project/MCGS/include/shaders/process.comp.spv")
        };
    context->prepare_pipeline({ compute_shader },

                              { context->get_descriptor_manager()->get_DescriptorSet(DescriptorManager::Compute) },
                              sizeof(PushContant_GS));
    context->post_prepare();
    PushContant_GS pc {
        .viewMatrix = { -.993989f,
                        .1083f,
                        -.021122f,
                        0.f,
                        .11034f,
                        .97551f,
                        -.19026f,
                        0.f,
                        0.f,
                        -.19143f,
                        -.98151f,
                        0.f,
                        0.f,
                        0.f,
                        4.0311f,
                        1.f },
        .projMatrix = { -2.760816f,
                        .300833f,
                        -0.021124f,
                        -0.021122f,
                        .306501f,
                        2.70976f,
                        -0.190277f,
                        -0.190258f,
                        -0.f,
                        -0.531742f,
                        -0.981605f,
                        -0.981507f,
                        -0.f,
                        -0.f,
                        4.021532f,
                        4.031129f },
        .campos = { 0.0851f,
                    0.7670f,
                    0.39566f },
        .tanfov = 0.36f
    };

    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),
                               [&](vk::CommandBuffer& cmd) {
                                   cmd
                                       .pushConstants<PushContant_GS>(
                                           context
                                               ->get_pipeline()
                                               ->get_layout(),
                                           vk::ShaderStageFlagBits::eCompute,
                                           0,
                                           pc);
                                   cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                                          context->get_pipeline()->get_layout(),
                                                          0,
                                                          context->get_pipeline()->get_descriptor_sets(),
                                                          {});
                                   cmd.bindPipeline(vk::PipelineBindPoint::eCompute,
                                                    context->get_pipeline()->get_handle());

                                   cmd.dispatch(3, 2, 1);
                               });
    std::cout << "here" << std::endl;
}

GaussianManager::GeometryState::GeometryState(int size)
{
    depth_d.resize(size);
    clamped_d.resize(size * 3);
    radii_d.resize(size);
    mean2d_d.resize(size);
    cov3d_d.resize(size * 6);
    conic_opacity_d.resize(size);
    rgb_d.resize(size * 3);
    tiles_touched_d.resize(size);
    point_offsets_d.resize(size);

    // depth = UniformManager::make_uniform(depth_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);

    // clamped = UniformManager::make_uniform(clamped_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
    // radii = UniformManager::make_uniform(radii_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
    // mean2d = UniformManager::make_uniform(mean2d_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
    // conv3d = UniformManager::make_uniform(cov3d_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
    // conic_opacity = UniformManager::make_uniform(conic_opacity_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
    // rgb = UniformManager::make_uniform(rgb_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
    // tiles_touched = UniformManager::make_uniform(tiles_touched_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
    // point_offsets = UniformManager::make_uniform(point_offsets_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
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

void GaussianManager::get_gaussian_raw_data()
{
    auto gs_data = MCGS::load_ply("/home/mocheng/project/MCGS/assets/point_cloud.ply");
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

    // xyz = UniformManager::make_uniform(xyz_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eUniformBuffer);
    // scale = UniformManager::make_uniform(scale_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eUniformBuffer);
    // feature = UniformManager::make_uniform(feature_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eUniformBuffer);
    // opacity = UniformManager::make_uniform(opacity_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eUniformBuffer);
    // rotation = UniformManager::make_uniform(rotations_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eUniformBuffer);
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

    geometry_state = GeometryState(xyz_d.size());
    auto addr = GS_Address {
        .xyz_address = xyz_buffer->get_address(),
        .scale_address = scale_buffer->get_address(),
        .feature_address = feature_buffer->get_address(),
        .opacity_address = xyz_buffer->get_address(),
        .rotation_address = rotation_buffer->get_address(),
        .depth_address = geometry_state.depth_buffer->get_address(),
        .clamped_address = geometry_state.clamped_buffer->get_address(),
        .radii_address = geometry_state.clamped_buffer->get_address(),
        .mean2d_address = geometry_state.mean2d_buffer->get_address(),
        .conv3d_address = geometry_state.conv3d_buffer->get_address(),
        .conic_opacity_address = geometry_state.conic_opacity_buffer->get_address(),
        .rgb_address = geometry_state.rgb_buffer->get_address(),
        .tiles_touched_address = geometry_state.tiles_touched_buffer->get_address(),
        .point_offsets_address = geometry_state.point_offsets_buffer->get_address()

    };
    address = UniformManager::make_uniform({ addr }, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
    // geometry_state.depth = UniformManager::make_uniform(geometry_state.depth_d, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
}
}