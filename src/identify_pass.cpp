#include "identify_pass.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "gaussian_manager.hpp"
namespace MCGS {

void IdentifyPass::prepare_descriptorset()
{
    content->prepare_descriptorset([&]() {
        auto descriptor_manager = content->get_descriptor_manager();
        descriptor_manager->Make_DescriptorSet(GaussianManager::Get_Singleton()->get_buffer_addr(),
                                               (int)Gaussian_Data_Index::eAddress,
                                               DescriptorManager::Compute);
    });
}
void IdentifyPass::prepare_shader_pc()
{
    shader_module.reset(new ShaderModule("/home/mocheng/project/MCGS/include/shaders/identify.comp.spv"));
    pc_size = sizeof(uint32_t);
}

void IdentifyPass::prepare_buffer()
{
}

void IdentifyPass::Execute()
{
    auto num_render = 1625771;
    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Compute_queue(),
                               // CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),

                               [&](vk::CommandBuffer& cmd) {
                                   cmd.pushConstants<uint32_t>(
                                       content
                                           ->get_pipeline()
                                           ->get_layout(),
                                       vk::ShaderStageFlagBits::eCompute,
                                       0,
                                       num_render);

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
                                   cmd.dispatch(ceil((float)num_render / 1024.f), 1, 1);
                               });
}

}