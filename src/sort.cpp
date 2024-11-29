#include "sort.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/Buffer.hpp"
#include "Wrapper/ComputePass/ComputePass.hpp"
constexpr uint32_t RADIX = 256;
constexpr int WORKGROUP_SIZE = 512;
constexpr int PARTITION_DIVISION = 8;
constexpr int PARTITION_SIZE = PARTITION_DIVISION * WORKGROUP_SIZE;
static constexpr uint32_t MAX_SPLAT_COUNT = 1 << 23;
namespace MCRT {
struct PushConstants {
    uint32_t pass;
    VkDeviceAddress elementCountReference;
    VkDeviceAddress globalHistogramReference;
    VkDeviceAddress partitionHistogramReference;
    VkDeviceAddress keysInReference;
    VkDeviceAddress keysOutReference;
    VkDeviceAddress valuesInReference;
    VkDeviceAddress valuesOutReference;
};
uint32_t RoundUp(uint32_t a, uint32_t b)
{
    return (a + b - 1) / b;
}
VkDeviceSize HistogramSize(uint32_t elementCount)
{
    return (1 + 4 * RADIX + RoundUp(elementCount, PARTITION_SIZE) * RADIX) *
        sizeof(uint32_t);
}

VkDeviceSize InoutSize(uint32_t elementCount)
{
    return elementCount * sizeof(uint32_t);
}
// void LookDeviceBuffer(vk::Buffer device_buffer, int size)
// {
//     Context::Get_Singleton()->get_device()->Get_Graphic_queue().waitIdle();
//     auto host_buffer = Buffer::create_buffer(nullptr, size, vk::BufferUsageFlagBits::eTransferDst);
//     CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),
//                                [&](vk::CommandBuffer& cmd) {
//                                    cmd.copyBuffer(
//                                        device_buffer,
//                                        host_buffer->get_handle(),
//                                        vk::BufferCopy()
//                                            .setDstOffset(0)
//                                            .setSrcOffset(0)
//                                            .setSize(size));
//                                });
//     auto cpu_raw_data = host_buffer->Get_mapped_data();
//     std::vector<uint> data(size / 4);
//     std::memcpy(data.data(), cpu_raw_data.data(), sizeof(data));
//     int r = 0;
// }
void gpusort::Init(uint all_point_count, std::shared_ptr<Buffer> _visiable_buffer)
{
    visiable_count_buffer = _visiable_buffer;

    elementCountSize = sizeof(uint32_t);
    histogramSize = HistogramSize(all_point_count);
    inoutSize = InoutSize(all_point_count);

    histogramOffset = elementCountSize;
    inoutOffset = histogramOffset + histogramSize;
    // 2x for key value
    VkDeviceSize storageSize = inoutOffset + 2 * inoutSize;

    storage_buffer = Buffer::CreateDeviceBuffer(
        nullptr,
        storageSize,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eTransferSrc |
            vk::BufferUsageFlagBits::eShaderDeviceAddress);
    partitionCount =
        RoundUp(all_point_count, PARTITION_SIZE);

    auto upsweep_shader = std::make_shared<ShaderModule>("include/shaders/gpusort/upsweep.comp.spv");

    auto spineshader = std::make_shared<ShaderModule>("include/shaders/gpusort/spine.comp.spv");
        auto downsweep_shader = std::make_shared<ShaderModule>("include/shaders/gpusort/downsweep.comp.spv");


    upsweepPass.reset(new ComputePass({}, sizeof(PushConstants), upsweep_shader));
    spinePass.reset(new ComputePass({}, sizeof(PushConstants), spineshader));
    downsweepKeyValuePass.reset(new ComputePass({}, sizeof(PushConstants), downsweep_shader));
}

void gpusort::sort(vk::CommandBuffer cmd, std::shared_ptr<Buffer> key_buffer, std::shared_ptr<Buffer> value_buffer)
{

    {
        cmd.copyBuffer(
            visiable_count_buffer->get_handle(),
            storage_buffer->get_handle(),
            vk::BufferCopy()
                .setSize(sizeof(uint32_t))
                .setDstOffset(0)
                .setSrcOffset(0));
    }
    // reset global histogram. partition histogram is set by shader.
    cmd.fillBuffer(
        storage_buffer->get_handle(),
        sizeof(uint32_t),
        4 * RADIX * sizeof(uint32_t),
        0);

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                        vk::PipelineStageFlagBits::eComputeShader,
                        vk::DependencyFlagBits::eDeviceGroup,
                        // 0,
                        vk::MemoryBarrier()
                            .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
                        {},
                        {});

    auto storageAddress = storage_buffer->get_address();
    auto keysAddress = key_buffer->get_address();
    auto valuesAddress = value_buffer->get_address();
    for (int i = 0; i < 4; ++i) {
        // pushConstants.pass = i;
        auto native_keysInReference = keysAddress;
        auto native_keysOutReference = storageAddress + inoutOffset;
        auto native_valuesInReference = valuesAddress;
        auto native_valuesOutReference = storageAddress + inoutOffset + inoutSize;

        cmd.pushConstants<PushConstants>(
            upsweepPass->get_pipeline()->get_layout(),
            vk::ShaderStageFlagBits::eCompute,
            0,
            PushConstants {
                .pass = static_cast<uint32_t>(i),
                .elementCountReference = storageAddress,
                .globalHistogramReference = storageAddress + sizeof(uint32_t),
                .partitionHistogramReference = storageAddress + sizeof(uint32_t) + sizeof(uint32_t) * 4 * RADIX,

                .keysInReference = i == 0 || i == 2 ? native_keysInReference : native_keysOutReference,
                .keysOutReference = i == 0 || i == 2 ? native_keysOutReference : native_keysInReference,
                .valuesInReference = i == 0 || i == 2 ? native_valuesInReference : native_valuesOutReference,
                .valuesOutReference = i == 0 || i == 2 ? native_valuesOutReference : native_valuesInReference

            });

        cmd.bindPipeline(vk::PipelineBindPoint ::eCompute, upsweepPass->get_pipeline()->get_handle());

        cmd.dispatch(partitionCount, 1, 1);

        //
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlagBits::eByRegion,
            vk::MemoryBarrier()
                .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
            {},
            {});
        {
            cmd.bindPipeline(
                vk::PipelineBindPoint ::eCompute,
                spinePass->get_pipeline()->get_handle());

            cmd.dispatch(RADIX, 1, 1);
        }
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlagBits::eByRegion,
            vk::MemoryBarrier()
                .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
            {},
            {});
        {
            cmd.bindPipeline(
                vk::PipelineBindPoint ::eCompute,
                downsweepKeyValuePass->get_pipeline()->get_handle());
            cmd.dispatch(partitionCount, 1, 1);
        }
        if (i < 3) {

            cmd.pipelineBarrier(
                vk::PipelineStageFlagBits::eComputeShader,
                vk::PipelineStageFlagBits::eComputeShader,
                vk::DependencyFlagBits::eByRegion,
                vk::MemoryBarrier()
                    .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                    .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
                {},
                {});
        }
    }
}

}