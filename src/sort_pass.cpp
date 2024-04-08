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
    point_list_pingpong = UniformManager::make_uniform(ping_pong_temp,
                                                       vk::ShaderStageFlagBits::eCompute,
                                                       vk::DescriptorType::eStorageBuffer);
    point_list_value = _point_list_value;
}
void SortPass::prepare_shader_pc()
{
    shader_module.reset(
        new ShaderModule("/home/mocheng/project/MCGS/include/shaders/sort/temp.comp.spv"));
    shader_module.reset(
        new ShaderModule("/home/mocheng/project/MCGS/include/shaders/sort/temp.comp.spv"));
    pc_size = sizeof(PushContant_Sort);
}
void SortPass::prepare_descriptorset()
{
    content->prepare_descriptorset([&]() {
        auto descriptor_manager = content->get_descriptor_manager();

        descriptor_manager->Make_DescriptorSet(point_list_key,
                                               0,
                                               DescriptorManager::Compute);

        descriptor_manager->Make_DescriptorSet(point_list_pingpong,
                                               1,
                                               DescriptorManager::Compute);
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

    element_in_data = UniformManager::make_uniform(element_in,
                                                   vk::ShaderStageFlagBits::eCompute,
                                                   vk::DescriptorType::eStorageBuffer,
                                                   vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    element_value_in_data = UniformManager::make_uniform(element_value_in,
                                                         vk::ShaderStageFlagBits::eCompute,
                                                         vk::DescriptorType::eStorageBuffer,
                                                         vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    ping_pong_data = UniformManager::make_uniform(ping_pong,
                                                  vk::ShaderStageFlagBits::eCompute,
                                                  vk::DescriptorType::eStorageBuffer,
                                                  vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    ping_pong_value_data = UniformManager::make_uniform(ping_pong_value,
                                                        vk::ShaderStageFlagBits::eCompute,
                                                        vk::DescriptorType::eStorageBuffer,
                                                        vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
}

void SortPass::Execute()
{
    // for (int i = 0; i < 4; i++) {
    //     execute(8 * i);
    // }

    // record_command();
    // std::chrono::steady_clock::time_point begin2 = std::chrono::steady_clock::now();
    // CommandManager::submit_Cmd(Context::Get_Singleton()->get_device()->Get_Compute_queue(), cmd);

    // std::chrono::steady_clock::time_point end2 = std::chrono::steady_clock::now();
    // double gpuSortTime2 = (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end2 - begin2).count()) * std::pow(10, -3));
    // std::cout << "GPU sort2 finished in " << gpuSortTime2 << "[ms]." << std::endl;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    execute(0);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    double gpuSortTime = (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) * std::pow(10, -3));
    std::cout << "GPU sort finished in " << gpuSortTime << "[ms]." << std::endl;

    std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
    // std::sort(element_in.begin(), element_in.end());
    std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
    auto cpuSortTime = (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end1 - begin1).count()) * std::pow(10, -3));

    std::cout << "CPU sort finished in " << cpuSortTime << "[ms]." << std::endl;

    // std::vector<uint64_t> data1(num_element);
    // std::vector<uint32_t> datatemp(num_element);
    // // std::vector<uint64_t> data2(num_element);

    // std::shared_ptr<Buffer> tempbuffer;
    // tempbuffer.reset(new Buffer(element_in.size() * 8, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible));
    // // Buffer::CopyBuffer(element_in_data->buffer, tempbuffer);

    // Buffer::CopyBuffer(element_in_data->buffer, tempbuffer);

    // auto temp1 = tempbuffer->Get_mapped_data(0);

    // // auto temp2 = element_value_in_data->buffer->Get_mapped_data(0);

    // std::memcpy(data1.data(), temp1.data(), temp1.size());
    // // std::memcpy(data2.data(), temp2.data(), temp2.size());
    // for (int i = 0; i < data1.size(); i++) {
    //     datatemp[i] = data1[i];
    // }

    // int r = data1[];
    int rr = 0;
}

void SortPass::execute(uint offset)
{

    PushContant_Sort pc {
        // .g_num_elements = GaussianManager::Get_Singleton()->get_point_num() * 10,
        .g_num_elements = 1625771,

        // .g_num_elements = num_element,

        // .g_shift = offset,
        // .g_num_workgroups = size_x,
        // .g_num_blocks_per_workgroup = num_blocks_per_workgroup
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