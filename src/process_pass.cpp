#include "Helper/CommandManager.hpp"
#include "Helper/cameraManager.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "gaussian_manager.hpp"
#include "precess_pass.hpp"
#include "shaders/push_contant.h"
namespace MCGS {
void ProcessPass::prepare_shader_pc()
{
    shader_module.reset(
        new ShaderModule("include/shaders/process.comp.spv"));
    pc_size = sizeof(PushContant_GS);
}
void ProcessPass::prepare_buffer()
{
}
void ProcessPass::prepare_descriptorset()
{
    content->prepare_descriptorset([&]() {
        auto descriptor_manager = content->get_descriptor_manager();
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
        // descriptor_manager->Make_DescriptorSet(render_out,
        //                                        DescriptorManager::Compute,
        //                                        (int)Gaussian_Data_Index::render_out_index,
        //                                        vk::DescriptorType::eStorageImage,
        //                                        vk::ShaderStageFlagBits::eCompute);

        descriptor_manager->Make_DescriptorSet(GaussianManager::Get_Singleton()->get_buffer_addr(),
                                               (int)Gaussian_Data_Index::eAddress,
                                               DescriptorManager::Compute);
    });
}

void ProcessPass::run_pass(vk::CommandBuffer& cmd)
{

    auto camera = GaussianManager::Get_Singleton()->get_camera();
    camera->Get_p_matrix();

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
        .tanfov = 0.36f,
        .point_num = static_cast<int>(GaussianManager::Get_Singleton()->get_point_num())
    };

    auto fov = 60.f / 180.0f * float(M_PI);
    auto tfov = tan(fov * 0.5f);
    pc = PushContant_GS {
        .viewMatrix = camera->Get_v_matrix(),
        .projMatrix = camera->Get_p_matrix() *camera->Get_v_matrix(),
        .campos = camera->get_pos(),
        .tanfov = float(tfov),
//        .tanfov = 0.36f,

        .point_num = static_cast<int>(GaussianManager::Get_Singleton()->get_point_num())
    };

    cmd.pushConstants<PushContant_GS>(
        content
            ->get_pipeline()
            ->get_layout(),
        vk::ShaderStageFlagBits::eCompute,
        0,
        pc);
    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                           content->get_pipeline()->get_layout(),
                           0,
                           content->get_pipeline()->get_descriptor_sets(),
                           {});
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute,
                     content->get_pipeline()->get_handle());

    cmd.dispatch(ceil(float(GaussianManager::Get_Singleton()->get_point_num()) / 256), 1, 1);
}

}