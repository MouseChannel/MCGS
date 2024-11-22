#pragma once
#include "Wrapper/ComputePass/ComputePass.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "vulkan/vulkan.hpp"
#include <memory>
namespace MCRT {
class Buffer;
class gpusort {
public:
    // struct PushConstant
    // {
    //     int pass;
    //     restrict ElementCount elementCountReference;
    //     restrict GlobalHistogram globalHistogramReference;
    //     restrict PartitionHistogram partitionHistogramReference;
    //     restrict Keys keysInReference;
    // };
    gpusort() = default;
    void Init(uint all_point_count, std::shared_ptr<Buffer> indirect_buffer);
    void sort(vk::CommandBuffer cmd, std::shared_ptr<Buffer> key_buffer, std::shared_ptr<Buffer> value_buffer);

    // private:
    std::shared_ptr<Buffer> storage_buffer;
    std::shared_ptr<Buffer> indirect_buffer;
    VkDeviceSize elementCountSize = sizeof(uint32_t);
    VkDeviceSize histogramSize;
    VkDeviceSize inoutSize;

    // std::shared_ptr<Buffer> key_buffer;
    // std::shared_ptr<Buffer> value_buffer;

    VkDeviceSize histogramOffset = sizeof(uint32_t);
    VkDeviceSize inoutOffset;
    uint32_t partitionCount;
    std::shared_ptr<Compute_Pipeline> upsweepPipeline;
    std::shared_ptr<Compute_Pipeline> spinePipeline;
    std::shared_ptr<Compute_Pipeline> downsweepKeyValuePipeline;

    std::shared_ptr<ComputePass<>> upsweepPass;
    std::shared_ptr<ComputePass<>> spinePass;
    std::shared_ptr<ComputePass<>> downsweepKeyValuePass;

    // std::shared_ptr<DescriptorSet> set;
    // std::shared_ptr<DescriptorPool> setpool;

    // std::shared_ptr<Compute_Pipeline> downsweepPipeline;
};
}