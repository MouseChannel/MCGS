#include "sort/sort_manager.hpp"
// #include "sort
#include "Helper/CommandManager.hpp"
#include "Wrapper/Buffer.hpp"
#include "Wrapper/CommandBuffer.hpp"
#include "Wrapper/Device.hpp"
#include <chrono>
#include <random>
namespace MCGS {

void SortManager::Init()
{
    prepare_buffer();
    sort_histogram_pass.reset(new SortHistogramPass(element_in_data, ping_pong_data, histograms_data, test_data));
    // sort_histogram_pass2.reset(new SortHistogramPass(ping_pong_data, histograms_data));
    sort_multi_pass.reset(new SortMultiPass(element_in_data, ping_pong_data, histograms_data, test_data));
    // sort_multi_pass2.reset(new SortMultiPass(ping_pong_data, element_in_data, histograms_data));

    sort_histogram_pass->Init();
    sort_multi_pass->Init();
    // sort_histogram_pass2->Init();
    // sort_multi_pass2->Init();
    semaphores.resize(2);
    fences.resize(2);
    auto device = Context::Get_Singleton()->get_device();
    // device->get_handle().resetFences(fences[0].get_handle());
    // device->get_handle().resetFences(fences[1].get_handle());
}
void SortManager::prepare_buffer()
{
    std::random_device rd;
    std::mt19937 gen(123);
    std::uniform_int_distribution<uint64_t> distrib(0, 0x0FFFFFFFFFFF);
    // element_in.resize(GaussianManager::Get_Singleton()->get_point_num());
    element_in.resize(num_element);
    // element_value_in.resize(1625771);

    for (int i = 0; i < element_in.size(); i++) {

        element_in[i] = distrib(gen);
    }

    histograms.resize(ceil((float)num_element / 256.f / 32.f) * 256);
    ping_pong.resize(num_element);
    // element_in_data = UniformManager::make_uniform(element_in,
    //                                                vk::ShaderStageFlagBits::eCompute,
    //                                                vk::DescriptorType::eStorageBuffer,
    //                                                vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    // histograms_data = UniformManager::make_uniform(histograms,
    //                                                vk::ShaderStageFlagBits::eCompute,
    //                                                vk::DescriptorType::eStorageBuffer,
    //                                                vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    // ping_pong_data = UniformManager::make_uniform(ping_pong,
    //                                               vk::ShaderStageFlagBits::eCompute,
    //                                               vk::DescriptorType::eStorageBuffer,
    //                                               vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst);
    element_in_buffer = MCRT::Buffer::CreateDeviceBuffer(element_in.data(),
                                                         element_in.size() * sizeof(element_in[0]),
                                                         vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress);

    pingpong_buffer = MCRT::Buffer::CreateDeviceBuffer(ping_pong.data(),
                                                       ping_pong.size() * sizeof(ping_pong[0]),
                                                       vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress);
    histograms_buffer = MCRT::Buffer::CreateDeviceBuffer(histograms.data(),
                                                         histograms.size() * sizeof(histograms[0]),
                                                         vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eShaderDeviceAddress);
    testaddr.element_in_address = element_in_buffer->get_address();
    testaddr.pinigpong_address = pingpong_buffer->get_address();
    testaddr.histograms_address = histograms_buffer->get_address();
    test_data = UniformManager::make_uniform({ testaddr }, vk::ShaderStageFlagBits::eCompute, vk::DescriptorType::eStorageBuffer);
}
void SortManager::run_pass(vk::CommandBuffer& cmd)
{

    // std::vector<std::shared_ptr<CommandBuffer>> command(2);
    // command[0].reset(new CommandBuffer);
    // command[1].reset(new CommandBuffer);

    auto device = Context::Get_Singleton()->get_device();
    auto queue = device->Get_Compute_queue();
    PushContant_SortHisgram pc {
        .g_num_elements = num_element,
        // .g_shift = i * 8,
        .g_num_workgroups = uint(ceil((float)num_element / 256.f / 32.f)),
        .g_num_blocks_per_workgroup = 32,
        // .pass = cur_fight
    };

    // for (uint i = 0; i < 8; i++) {
    //     auto cur_fight = i % 2;

    //     pc.g_shift = i * 8;
    //     pc.pass = cur_fight;

    //     sort_histogram_pass->pc = pc;
    //     sort_multi_pass->pc = pc;

    //     sort_histogram_pass->run_pass(cmd);
    //     sort_multi_pass->run_pass(cmd);
    // }
    // return;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    CommandManager::ExecuteCmd(device->Get_Compute_queue(), [&](vk::CommandBuffer& cmd) {
        for (uint i = 0; i < 8; i++) {
            auto cur_fight = i % 2;

            pc.g_shift = i * 8;
            pc.pass = cur_fight;

            sort_histogram_pass->pc = pc;
            sort_multi_pass->pc = pc;

            sort_histogram_pass->run_pass(cmd);
            sort_multi_pass->run_pass(cmd);
        }
    });

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    auto cpuSortTime = (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()) * std::pow(10, -3));

    std::cout << "cost0  :" << cpuSortTime << std::endl;
    // std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
    // for (uint i = 0; i < 8; i++) {
    //     auto cur_fight = i % 2;
    //     // cmd = command[cur_fight]->get_handle();
    //     cmd = command[cur_fight]->get_handle();

    //     auto res = device->get_handle().waitForFences(fences[cur_fight].get_handle(), true, UINT64_MAX);
    //     device->get_handle().resetFences(fences[cur_fight].get_handle());
    //     // cmd.reset(vk::CommandBufferResetFlagBits(0));
    //     // command->Begin(vk::CommandBufferUsageFlagBits(0));
    //     cmd.reset();
    //     cmd.begin(vk::CommandBufferBeginInfo());
    //     pc.g_shift = i * 8;
    //     pc.pass = cur_fight;

    //     sort_histogram_pass->pc = pc;
    //     sort_multi_pass->pc = pc;
    //     // sort_histogram_pass2->pc = pc;
    //     // sort_multi_pass2->pc = pc;

    //     sort_histogram_pass->run_pass(cmd);
    //     sort_multi_pass->run_pass(cmd);

    //     // if (i % 2 == 0) {
    //     //     sort_histogram_pass->run_pass(cmd);
    //     //     sort_multi_pass->run_pass(cmd);
    //     // } else {
    //     //     sort_histogram_pass2->run_pass(cmd);
    //     //     sort_multi_pass2->run_pass(cmd);
    //     // }
    //     // command->End();
    //     cmd.end();
    //     std::vector<vk::PipelineStageFlags>
    //         waitflag { vk::PipelineStageFlagBits::eComputeShader };
    //     auto submit_info = vk::SubmitInfo()
    //                            .setCommandBuffers(cmd)

    //                            .setSignalSemaphores(semaphores[cur_fight].get_handle());
    //     if (i != 0) {
    //         submit_info
    //             .setWaitSemaphores(semaphores[1 - cur_fight].get_handle())
    //             .setWaitDstStageMask(waitflag);
    //     }
    //     queue.submit(submit_info, fences[cur_fight].get_handle());
    //     // queue.submit(submit_info);
    // }
    // queue.waitIdle();
    // std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
    // auto cpuSortTime1 = (static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(end1 - begin1).count()) * std::pow(10, -3));

    // std::cout << "cost  :" << cpuSortTime1 << std::endl;
}
}