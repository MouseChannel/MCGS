#include "duplicateWithKeys_pass.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "gaussian_manager.hpp"
namespace MCGS {
duplicatePass::duplicatePass(std::shared_ptr<Uniform_Stuff<uint64_t>> _point_list_key, std::shared_ptr<Uniform_Stuff<uint64_t>> _point_list_value)
    : GSPassBase()
{
    point_list_value = _point_list_value;
    point_list_key = _point_list_key;
}

void duplicatePass::prepare_buffer()
{
}

void duplicatePass::prepare_shader_pc()
{
    shader_module.reset(
        new ShaderModule("/home/mocheng/project/MCGS/include/shaders/duplicate.comp.spv"));
    pc_size = sizeof(PushContant_Dup);
}
void duplicatePass::prepare_descriptorset()
{
    content->prepare_descriptorset([&]() {
        auto descriptor_manager = content->get_descriptor_manager();

        descriptor_manager->Make_DescriptorSet(point_list_key,
                                               0,
                                               DescriptorManager::Compute);
        descriptor_manager->Make_DescriptorSet(point_list_value,
                                               1,
                                               DescriptorManager::Compute);

        descriptor_manager->Make_DescriptorSet(GaussianManager::Get_Singleton()->get_buffer_addr(),
                                               (int)Gaussian_Data_Index::eAddress,
                                               DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(zero_data,
        //                                        1,
        //                                        DescriptorManager::Compute);
    });
}
void duplicatePass::Execute()
{
    PushContant_Dup pc {
        .g_num_elements = GaussianManager::Get_Singleton()->get_point_num()
        // .second_step = 1024
    };
    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Compute_queue(),
                               // CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),

                               [&](vk::CommandBuffer& cmd) {
                                   cmd.pushConstants<PushContant_Dup>(
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

                                   cmd.pipelineBarrier2(vk::DependencyInfo()
                                                            .setMemoryBarriers(
                                                                vk::MemoryBarrier2()
                                                                    .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                                                    .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
                                                                    .setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                                                    .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)));

                                   cmd.dispatch(ceil(GaussianManager::Get_Singleton()->get_point_num() / 1024.f), 1, 1);
                               });
}
}