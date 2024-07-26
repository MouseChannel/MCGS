#include "sort/sortHistogramPass.hpp"

namespace MCGS {
SortHistogram_Pass::SortHistogram_Pass(std::weak_ptr<GaussianContext> context)
    : ComputePass<GaussianContext>(context)
{
    Init();
}
void SortHistogram_Pass::Init()
{
    SetShaderModule("include/shaders/sort/multi_radixsort_histograms.comp.spv");
    Prepare_DescriptorSet([&]() {
        AddDescriptorTarget(std::make_shared<BufferDescriptorTarget>(
            m_computeContext.lock()->get_address_buffer(),
            (int)Gaussian_Binding_Index::eAddress,
            vk::ShaderStageFlagBits::eCompute,
            vk::DescriptorType::eStorageBuffer,
            get_DescriptorSet()));
    });
    prepare_pipeline(sizeof(PushContant_SortHisgram));
}
void SortHistogram_Pass::Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z)
{
    cmd.pushConstants<PushContant_SortHisgram>(
        get_pipeline()
            ->get_layout(),
        vk::ShaderStageFlagBits::eCompute,
        0,
        pc);

    cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute,
                           get_pipeline()->get_layout(),
                           0,
                           get_DescriptorSet()->get_handle(),
                           {});
    cmd.bindPipeline(vk::PipelineBindPoint::eCompute,
                     get_pipeline()->get_handle());

    cmd.pipelineBarrier2(vk::DependencyInfo()
                             .setMemoryBarriers(
                                 vk::MemoryBarrier2()
                                     .setSrcStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                     .setSrcAccessMask(vk::AccessFlagBits2::eShaderWrite)
                                     .setDstStageMask(vk::PipelineStageFlagBits2::eComputeShader)
                                     .setDstAccessMask(vk::AccessFlagBits2::eShaderRead)));
    cmd.dispatch(ceil((float)1625771 / 256.f / 32.f), 1, 1);
}

}