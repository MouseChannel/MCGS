#include "sort_pass.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/CommandBuffer.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "gaussian_manager.hpp"
#include "shaders/push_contant.h"
#include <chrono>
#include <random>
namespace MCGS {
SortPass::SortPass(std::shared_ptr<Uniform_Stuff<uint64_t>> _point_list_key, std::shared_ptr<Uniform_Stuff<uint64_t>> _point_list_value)
{
    point_list_key = _point_list_key;
    ping_pong_temp.resize(1625771);
    // point_list_pingpong = UniformManager::make_uniform(ping_pong_temp,
    //                                                    vk::ShaderStageFlagBits::eCompute,
    //                                                    vk::DescriptorType::eStorageBuffer);
    point_list_value = _point_list_value;
}
void SortPass::prepare_shader_pc()
{
    // shader_module.reset(
    //     new ShaderModule("/home/mocheng/project/MCGS/include/shaders/sort/temp.comp.spv"));
    shader_module.reset(
        new ShaderModule("/home/mocheng/project/MCGS/include/shaders/sort/sort.comp.spv"));
    pc_size = sizeof(PushContant_Sort);
}
void SortPass::prepare_descriptorset()
{
    content->prepare_descriptorset([&]() {
        auto descriptor_manager = content->get_descriptor_manager();

        // descriptor_manager->Make_DescriptorSet(element_in_data,
        //                                        0,
        //                                        DescriptorManager::Compute);

        // descriptor_manager->Make_DescriptorSet(ping_pong_data,
        //                                        1,
        //                                        DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(element_value_in_data,
        //                                        2,
        //                                        DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(ping_pong_value_data,
        //                                        3,
        //                                        DescriptorManager::Compute);
        descriptor_manager->Make_DescriptorSet(GaussianManager::Get_Singleton()->get_buffer_addr(),
                                               (int)Gaussian_Data_Index::eAddress,
                                               DescriptorManager::Compute);
    });
}
void SortPass::prepare_buffer()
{
    std::random_device rd;
    std::mt19937 gen(123);
    std::uniform_int_distribution<uint32_t> distrib(0, 0x0FFFFFFFF);
    // element_in.resize(GaussianManager::Get_Singleton()->get_point_num());
    element_in.resize(1625771);
    element_value_in.resize(1625771);

    for (int i = 0; i < element_in.size(); i++) {

        element_in[i] = distrib(gen);
        element_in[i] <<= 32;
        // if (i < 66) {
        //     element_in[i] *= -1;
        // }
        element_value_in[i] = element_in[i];
        // element_in[i] <<= 12;
    }
    // ping_pong.resize(GaussianManager::Get_Singleton()->get_point_num());
    ping_pong.resize(1625771);

    ping_pong_value.resize(num_element);
    histograms.resize(num_element);

    // element_in_data = UniformManager::make_uniform(element_in,
    //                                                vk::ShaderStageFlagBits::eCompute,
    //                                                vk::DescriptorType::eStorageBuffer,
    //                                                vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    // element_value_in_data = UniformManager::make_uniform(element_value_in,
    //                                                      vk::ShaderStageFlagBits::eCompute,
    //                                                      vk::DescriptorType::eStorageBuffer,
    //                                                      vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    // ping_pong_data = UniformManager::make_uniform(ping_pong,
    //                                               vk::ShaderStageFlagBits::eCompute,
    //                                               vk::DescriptorType::eStorageBuffer,
    //                                               vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    // ping_pong_value_data = UniformManager::make_uniform(ping_pong_value,
    //                                                     vk::ShaderStageFlagBits::eCompute,
    //                                                     vk::DescriptorType::eStorageBuffer,
    //                                                     vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);

    // histograms_data = UniformManager::make_uniform(histograms,vk::ShaderStageFlagBits::eCompute,
    //                                                     vk::DescriptorType::eStorageBuffer,
    //                                                     vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
}

void SortPass::run_pass(vk::CommandBuffer& cmd)
{

    PushContant_Sort pc {
        .g_num_elements = 1625771,
    };

    cmd.pushConstants<PushContant_Sort>(
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
}

void SortPass::execute(uint offset)
{

    PushContant_Sort pc {

        .g_num_elements = 1625771,

    };

    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Compute_queue(),
                               // CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),

                               [&](vk::CommandBuffer& cmd) {
                                   cmd.pushConstants<PushContant_Sort>(
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
}