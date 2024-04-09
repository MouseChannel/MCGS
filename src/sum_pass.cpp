#include "sum_pass.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "gaussian_manager.hpp"
#include "shaders/push_contant.h"
#include <chrono>
namespace MCGS {

void SumPass::prepare_shader_pc()
{
    shader_module.reset(
        new ShaderModule("/home/mocheng/project/MCGS/include/shaders/inclusiveSum.comp.spv"));
    pc_size = sizeof(PushContant_Sum);
}
void SumPass::prepare_buffer()
{

    element_in.resize(num_element, 1);

    element_in_data = UniformManager::make_uniform(element_in,
                                                   vk::ShaderStageFlagBits::eCompute,
                                                   vk::DescriptorType::eStorageBuffer,
                                                   vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
}
void SumPass::prepare_descriptorset()
{
    content->prepare_descriptorset([&]() {
        auto descriptor_manager = content->get_descriptor_manager();

        descriptor_manager->Make_DescriptorSet(element_in_data,
                                               0,
                                               DescriptorManager::Compute);

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
        .g_num_elements = GaussianManager::Get_Singleton()->get_point_num()

        // .g_num_elements = num_element

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

                                   //    cmd.dispatch(ceil(num_element / 1024.f), 1, 1);
                               });
}
void SumPass::execute_second()
{
    PushContant_Sum pc {
        .step = 1024,
        .g_num_elements = GaussianManager::Get_Singleton()->get_point_num()
        // .g_num_elements = num_element

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
        .g_num_elements = GaussianManager::Get_Singleton()->get_point_num()
        // .g_num_elements = num_element

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

                                   //    cmd.dispatch(ceil(num_element / 1024.f), 1, 1);
                               });
}
void SumPass::execute_test()
{
    PushContant_Sum pc {
        .step = 1,
        .g_num_elements = GaussianManager::Get_Singleton()->get_point_num()
        // .g_num_elements = num_element

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
                                   // second
                                   pc.step = 1024;
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
                                   // last
                                   pc.step = -1;
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
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    // execute_first();
    // execute_second();
    // execute_last();
    execute_test();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double gpuSortTime = (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) * std::pow(10, -3));
    std::cout << "GPU sum finished in " << gpuSortTime << "[ms]." << std::endl;

    // GaussianManager::Get_Singleton()->get_buffer_addr();
    // std::vector<uint32_t>
    //     data1(GaussianManager::Get_Singleton()->get_point_num());

    // std::shared_ptr<Buffer> tempbuffer;
    // tempbuffer.reset(new Buffer(data1.size() * 4, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible));
    // // Buffer::CopyBuffer(element_in_data->buffer, tempbuffer);

    // Buffer::CopyBuffer(, tempbuffer);

    // // Buffer::CopyBuffer(element_in_data->buffer, tempbuffer);

    // auto temp1 = tempbuffer->Get_mapped_data(0);

    // std::memcpy(data1.data(), temp1.data(), temp1.size());

    // auto cpuarr = element_in;
    // for (int i = 1; i < cpuarr.size(); i++) {
    //     cpuarr[i] += cpuarr[i - 1];
    // }
    // for (int i = 0; i < cpuarr.size(); i++) {
    //     if (cpuarr[i] != data1[i]) {
    //         int s = 0;
    //     }
    // }
    // // int r = data1[168385];
    // int rr = 0;
}
}