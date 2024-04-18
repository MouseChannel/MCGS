#include "sort_multi_pass2.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/CommandBuffer.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "gaussian_manager.hpp"
#include "shaders/push_contant.h"
#include <chrono>
#include <cmath>
#include <random>

namespace MCGS {

SortMultiPass2::SortMultiPass2(std::shared_ptr<Uniform_Stuff<uint64_t>> _element_in_data,
                               std::shared_ptr<Uniform_Stuff<uint64_t>> _pingpong_data,
                               std::shared_ptr<Uniform_Stuff<uint32_t>> _histograms_data)
{
    element_in_data = _element_in_data;
    histograms_data = _histograms_data;
    ping_pong_data = _pingpong_data;
}

void SortMultiPass2::run_pass(vk::CommandBuffer& cmd)
{
    // PushContant_SortHisgram pc {
    //     .g_num_elements = 1625771,
    //     .g_shift = 0,
    //     .g_num_blocks_per_workgroup = 32,
    //     .g_num_workgroups = uint(ceil((float)num_element / 256.f)),
    // };

    cmd.pushConstants<PushContant_SortHisgram>(
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
    cmd.dispatch(ceil((float)num_element / 256.f / 32.f), 1, 1);
}

void SortMultiPass2::prepare_buffer()
{
    // ping_pong.resize(num_element);
    // // histograms.resize(num_element);
    // ping_pong_data = UniformManager::make_uniform(ping_pong,
    //                                               vk::ShaderStageFlagBits::eCompute,
    //                                               vk::DescriptorType::eStorageBuffer,
    //                                               vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    // histograms_data = UniformManager::make_uniform(histograms,vk::ShaderStageFlagBits::eCompute,
    //                                                    vk::DescriptorType::eStorageBuffer,
    //                                                    vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
}

void SortMultiPass2::prepare_shader_pc()
{
    shader_module.reset(
        new ShaderModule("/home/mocheng/project/MCGS/include/shaders/sort/multi_radixsort.comp.spv"));
    pc_size = sizeof(PushContant_SortHisgram);
}

void SortMultiPass2::prepare_descriptorset()
{
    content->prepare_descriptorset([&]() {
        auto descriptor_manager = content->get_descriptor_manager();

        descriptor_manager->Make_DescriptorSet(element_in_data,
                                               1,
                                               DescriptorManager::Compute);

        descriptor_manager->Make_DescriptorSet(ping_pong_data,
                                               0,
                                               DescriptorManager::Compute);


        descriptor_manager->Make_DescriptorSet(histograms_data,
                                               4,
                                               DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(element_value_in_data,
        //                                        2,
        //                                        DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(ping_pong_value_data,
        //                                        3,
        //                                        DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(GaussianManager::Get_Singleton()->get_buffer_addr(),
        //                                        (int)Gaussian_Data_Index::eAddress,
        //                                        DescriptorManager::Compute);
    });
}
}