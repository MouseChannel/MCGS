#pragma once
#include "shaders/DataStruct.h"

#include "sort.hpp"
#include <Rendering/ComputeContext.hpp>
#include <Wrapper/ComputePass/ComputePass.hpp>

namespace MCRT {

class Buffer;
class GSContext : public ComputeContext {
public:
    GSContext(std::string path);

    void prepare() override;
    std::shared_ptr<CommandBuffer> BeginFrame() override;
    void tick(std::shared_ptr<CommandBuffer> command);

    std::shared_ptr<Buffer>
        raw_data;


    std::shared_ptr<Buffer> instance_buffer;
    std::shared_ptr<Buffer> indirct_cmd_buffer;
    std::shared_ptr<Buffer> point_count_buffer;
    std::shared_ptr<Buffer> visiable_count_buffer;

    std::shared_ptr<Buffer> key_buffer;
    std::shared_ptr<Buffer> value_buffer;
    std::shared_ptr<Buffer> inverse_index_buffer;
    std::shared_ptr<Buffer> index_buffer;
    std::shared_ptr<Buffer> camera_buffer;

    std::unique_ptr<ComputePass<>> pre_process_pass;
    std::unique_ptr<ComputePass<>> rank_pass;
    std::unique_ptr<ComputePass<>> inverse_pass;
    std::unique_ptr<ComputePass<>> projection_pass;

    int all_point_count;
    // std::unique_ptr<ComputePass<>> duplicatePass;

    // std::vector<DescriptorSet> sets;
    std::shared_ptr<gpusort> m_gpu_sort;

    std::shared_ptr<Compute_Pipeline> rank_pipeline;

    std::shared_ptr<DescriptorSet> set;
    std::shared_ptr<DescriptorPool> setpool;
};
}