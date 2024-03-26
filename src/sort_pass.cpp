#include "sort_pass.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/CommandBuffer.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "shaders/push_contant.h"
#include <chrono>
#include <random>
namespace MCGS {
SortPass::SortPass()
{
    prepare_buffer();

    sort_content.reset(new ComputePass);
    sort_content->set_constants_size(sizeof(PushContant_Sort));
    sort_content->prepare();
    sort_content->prepare_descriptorset([&]() {
        auto descriptor_manager = sort_content->get_descriptor_manager();

        descriptor_manager->Make_DescriptorSet(element_in_data,
                                               0,
                                               DescriptorManager::Compute);
        // descriptor_manager->Make_DescriptorSet(zero_data,
        //                                        1,
        //                                        DescriptorManager::Compute);
        descriptor_manager->Make_DescriptorSet(ping_pong_data,
                                               1,
                                               DescriptorManager::Compute);
    });
    std::shared_ptr<ShaderModule>
        sort_compute_shader {
            new ShaderModule("/home/mocheng/project/MCGS/include/shaders/sort/single_radixsort.comp.spv")
        };
    sort_content->prepare_pipeline({ sort_compute_shader },

                                   { sort_content->get_descriptor_manager()->get_DescriptorSet(DescriptorManager::Compute) },
                                   sizeof(PushContant_Sort));
    sort_content->post_prepare();

    // low_radixsort.reset(new ComputePass);
    // low_radixsort->set_constants_size(sizeof(PushContant_Sort));
    // low_radixsort->prepare();
    // low_radixsort->prepare_descriptorset([&]() {
    //     auto descriptor_manager = low_radixsort->get_descriptor_manager();

    //     descriptor_manager->Make_DescriptorSet(element_in_data,
    //                                            0,
    //                                            DescriptorManager::Compute);
    //     // descriptor_manager->Make_DescriptorSet(zero_data,
    //     //                                        1,
    //     //                                        DescriptorManager::Compute);
    //     descriptor_manager->Make_DescriptorSet(histograms_data,
    //                                            1,
    //                                            DescriptorManager::Compute);
    // });
    // std::shared_ptr<ShaderModule>
    //     low_radixsort_compute_shader {
    //         new ShaderModule("/home/mocheng/project/MCGS/include/shaders/sort/multi_radixsort_histograms.comp.spv")
    //     };
    // low_radixsort->prepare_pipeline({ low_radixsort_compute_shader },

    //                                 { low_radixsort->get_descriptor_manager()->get_DescriptorSet(DescriptorManager::Compute) },
    //                                 sizeof(PushContant_Sort));
    // low_radixsort->post_prepare();

    // high_radixsort.reset(new ComputePass);
    // high_radixsort->set_constants_size(sizeof(PushContant_Sort));
    // high_radixsort->prepare();
    // high_radixsort->prepare_descriptorset([&]() {
    //     auto descriptor_manager = high_radixsort->get_descriptor_manager();

    //     descriptor_manager->Make_DescriptorSet(element_in_data,
    //                                            0,
    //                                            DescriptorManager::Compute);
    //     descriptor_manager->Make_DescriptorSet(zero_data,
    //                                            1,
    //                                            DescriptorManager::Compute);
    //     descriptor_manager->Make_DescriptorSet(histograms_data,
    //                                            2,
    //                                            DescriptorManager::Compute);
    // });

    // std::shared_ptr<ShaderModule>
    //     high_radixsort_compute_shader {
    //         new ShaderModule("/home/mocheng/project/MCGS/include/shaders/sort/multi_radixsort.comp.spv")
    //     };
    // high_radixsort->prepare_pipeline({ high_radixsort_compute_shader },

    //                                  { high_radixsort->get_descriptor_manager()->get_DescriptorSet(DescriptorManager::Compute) },
    //                                  sizeof(PushContant_Sort));
    // high_radixsort->post_prepare();
}

void SortPass::prepare_buffer()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 0x0FFFFFFF);
    element_in.resize(num_element);
    for (int i = 0; i < num_element; i++) {
        element_in[i] = distrib(gen);
    }
    ping_pong.resize(num_element);
    element_in_data = UniformManager::make_uniform(element_in,
                                                   vk::ShaderStageFlagBits::eCompute,
                                                   vk::DescriptorType::eStorageBuffer,
                                                   vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    ping_pong_data = UniformManager::make_uniform(ping_pong,
                                                  vk::ShaderStageFlagBits::eCompute,
                                                  vk::DescriptorType::eStorageBuffer,
                                                  vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);

    // zeros.resize(num_element);
    // element_in_data = UniformManager::make_uniform(element_in,
    //                                                vk::ShaderStageFlagBits::eCompute,
    //                                                vk::DescriptorType::eStorageBuffer,
    //                                                vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    // zero_data = UniformManager::make_uniform(zeros,
    //                                          vk::ShaderStageFlagBits::eCompute,
    //                                          vk::DescriptorType::eStorageBuffer,
    //                                          vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    // histograms_data = UniformManager::make_uniform(zeros,
    //                                                vk::ShaderStageFlagBits::eCompute,
    //                                                vk::DescriptorType::eStorageBuffer,
    //                                                vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
}

void SortPass::execute()
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
    std::sort(element_in.begin(), element_in.end());
    std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
    auto cpuSortTime = (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end1 - begin1).count()) * std::pow(10, -3));

    std::cout << "CPU sort finished in " << cpuSortTime << "[ms]." << std::endl;
    // std::vector<uint32_t> data(num_element);
    // auto buffer = Buffer::create_buffer(data.data(), data.size() * sizeof(data[0]), vk::BufferUsageFlagBits::eTransferDst);
    // Buffer::CopyBuffer(element_in_data->buffer, buffer);

    // // auto temp = buffer->Get_mapped_data(0);
    // auto temp = ping_pong_data->buffer->Get_mapped_data(0);

    // std::memcpy(data.data(), temp.data(), temp.size());
    std::vector<uint32_t>
        data1(num_element);
    auto temp1 = element_in_data->buffer->Get_mapped_data(0);

    std::memcpy(data1.data(), temp1.data(), temp1.size());

    auto re = element_in;
    int a = 0;
}
// void SortPass::record_command()
// {

//     cmd.reset(new CommandBuffer);
//     PushContant_Sort pc {
//         .g_num_elements = num_element,
//         // .g_shift = offset,
//         // .g_num_workgroups = size_x,
//         // .g_num_blocks_per_workgroup = num_blocks_per_workgroup
//     };
//     cmd->get_handle().begin(vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
//     cmd->get_handle()
//         .pushConstants<PushContant_Sort>(
//             sort_content
//                 ->get_pipeline()
//                 ->get_layout(),
//             vk::ShaderStageFlagBits::eCompute,
//             0,
//             pc);
//     cmd->get_handle().bindDescriptorSets(vk::PipelineBindPoint::eCompute,
//                                          sort_content->get_pipeline()->get_layout(),
//                                          0,
//                                          sort_content->get_pipeline()->get_descriptor_sets(),
//                                          {});
//     cmd->get_handle().bindPipeline(vk::PipelineBindPoint::eCompute,
//                                    sort_content->get_pipeline()->get_handle());

//     cmd->get_handle().pipelineBarrier2(vk::DependencyInfo()
//                                            .setMemoryBarriers(
//                                                vk::MemoryBarrier2()
//                                                    .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
//                                                    .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
//                                                    .setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
//                                                    .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)));
//     cmd->get_handle().dispatch(1, 1, 1);

//     cmd->get_handle().end();
// }
void SortPass::execute(uint offset)
{
    // uint32_t globalInvocationSize = num_element / num_blocks_per_workgroup;
    // uint32_t remainder = num_element % num_blocks_per_workgroup;
    // globalInvocationSize += remainder > 0 ? 1 : 0;
    // uint size_x = std::ceil((float)globalInvocationSize / (float)WORKGROUP_SIZE);

    PushContant_Sort pc {
        .g_num_elements = num_element,
        // .g_shift = offset,
        // .g_num_workgroups = size_x,
        // .g_num_blocks_per_workgroup = num_blocks_per_workgroup
    };
    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Compute_queue(),
                               // CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),

                               [&](vk::CommandBuffer& cmd) {
                                   cmd.pushConstants<PushContant_Sort>(
                                       sort_content
                                           ->get_pipeline()
                                           ->get_layout(),
                                       vk::ShaderStageFlagBits::eCompute,
                                       0,
                                       pc);
                                   cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                                          sort_content->get_pipeline()->get_layout(),
                                                          0,
                                                          sort_content->get_pipeline()->get_descriptor_sets(),
                                                          {});
                                   cmd.bindPipeline(vk::PipelineBindPoint::eCompute,
                                                    sort_content->get_pipeline()->get_handle());

                                   cmd.pipelineBarrier2(vk::DependencyInfo()
                                                            .setMemoryBarriers(
                                                                vk::MemoryBarrier2()
                                                                    .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                                                    .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
                                                                    .setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                                                    .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)));
                                   //    cmd.pushConstants<PushContant_Sort>(
                                   //        low_radixsort
                                   //            ->get_pipeline()
                                   //            ->get_layout(),
                                   //        vk::ShaderStageFlagBits::eCompute,
                                   //        0,
                                   //        pc);
                                   //    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                   //                           low_radixsort->get_pipeline()->get_layout(),
                                   //                           0,
                                   //                           low_radixsort->get_pipeline()->get_descriptor_sets(),
                                   //                           {});
                                   //    cmd.bindPipeline(vk::PipelineBindPoint::eCompute,
                                   //                     low_radixsort->get_pipeline()->get_handle());

                                   //    cmd.pipelineBarrier2(vk::DependencyInfo()
                                   //                             .setMemoryBarriers(
                                   //                                 vk::MemoryBarrier2()
                                   //                                     .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                   //                                     .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
                                   //                                     .setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                   //                                     .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)));

                                   //    cmd.pushConstants<PushContant_Sort>(
                                   //        high_radixsort
                                   //            ->get_pipeline()
                                   //            ->get_layout(),
                                   //        vk::ShaderStageFlagBits::eCompute,
                                   //        0,
                                   //        pc);
                                   //    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                                   //                           high_radixsort->get_pipeline()->get_layout(),
                                   //                           0,
                                   //                           high_radixsort->get_pipeline()->get_descriptor_sets(),
                                   //                           {});
                                   //    cmd.bindPipeline(vk::PipelineBindPoint::eCompute,
                                   //                     high_radixsort->get_pipeline()->get_handle());

                                   //    cmd.pipelineBarrier2(vk::DependencyInfo()
                                   //                             .setMemoryBarriers(
                                   //                                 vk::MemoryBarrier2()
                                   //                                     .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                   //                                     .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
                                   //                                     .setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                   //                                     .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)));

                                   cmd.dispatch(1, 1, 1);
                               });
}
}