#include "sum_pass.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "gaussian_manager.hpp"
#include "shaders/push_contant.h"

namespace MCGS {

void SumPass::prepare_shader_pc()
{
    shader_module.reset(
        new ShaderModule("/home/mocheng/project/MCGS/include/shaders/inclusiveSum.comp.spv"));
    pc_size = sizeof(PushContant_Sum);
}
void SumPass::prepare_buffer()
{

    // element_in.resize(num_element, 1);

    // element_in_data = UniformManager::make_uniform(element_in,
    //                                                vk::ShaderStageFlagBits::eCompute,
    //                                                vk::DescriptorType::eStorageBuffer,
    //                                                vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
}
void SumPass::prepare_descriptorset()
{
    content->prepare_descriptorset([&]() {
        auto descriptor_manager = content->get_descriptor_manager();

        // descriptor_manager->Make_DescriptorSet(element_in_data,
        //                                        0,
        //                                        DescriptorManager::Compute);

        descriptor_manager->Make_DescriptorSet(GaussianManager::Get_Singleton()->get_buffer_addr(),
                                               (int)Gaussian_Data_Index::eAddress,
                                               DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(zero_data,
        //                                        1,
        //                                        DescriptorManager::Compute);
    });
}
void SumPass::execute_first()
{
    PushContant_Sum pc {
        .step = 1,
        // .second_step = 1024
    };
    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Compute_queue(),
                               // CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),

                               [&](vk::CommandBuffer& cmd) {
                                   cmd.pushConstants<PushContant_Sum>(
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
void SumPass::execute_second()
{
    PushContant_Sum pc {
        .step = 1024,
        // .second_step = 1024
    };
    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Compute_queue(),
                               // CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),

                               [&](vk::CommandBuffer& cmd) {
                                   cmd.pushConstants<PushContant_Sum>(
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

                                   cmd.dispatch(1, 1, 1);
                               });
}
void SumPass::execute_last()
{
    PushContant_Sum pc {
        .step = -1,
        // .second_step = 1024
    };
    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Compute_queue(),
                               // CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),

                               [&](vk::CommandBuffer& cmd) {
                                   cmd.pushConstants<PushContant_Sum>(
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

void SumPass::Execute()
{
    execute_first();
    execute_second();
    execute_last();

    // GaussianManager::Get_Singleton()->get_buffer_addr();
    //     std::vector<uint32_t>
    //         data1(168386);
    // auto temp1 = element_in_data->buffer->Get_mapped_data(0);

    // std::memcpy(data1.data(), temp1.data(), temp1.size());
    // int r = data1[168385];
    // int rr = 0;
}
}