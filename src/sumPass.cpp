

#include "sumPass.hpp"

namespace MCGS {

sum_Pass::sum_Pass(std::weak_ptr<GaussianContext> context)
    : ComputePass<GaussianContext>(context)
{
    Init();
}
void sum_Pass::Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z)
{
    auto contextp = m_computeContext.lock();
    PushContant_Sum pc {
        .step = 1,
        .g_num_elements = contextp->get_point_num()
        // .g_num_elements = num_element

    };
    cmd.pushConstants<PushContant_Sum>(
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

    cmd.dispatch(ceil(contextp->get_point_num() / 1024.f), 1, 1);
    // second
    pc.step = 1024;
    cmd.pushConstants<PushContant_Sum>(
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

    cmd.dispatch(1, 1, 1);
    // last
    pc.step = -1;
    cmd.pushConstants<PushContant_Sum>(
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

    cmd.dispatch(ceil(contextp->get_point_num() / 1024.f), 1, 1);
}
void sum_Pass::Init()
{
    ComputePass<GaussianContext>::    SetShaderModule("include/shaders/inclusiveSum.comp.spv");
    ComputePass<GaussianContext>:: Prepare_DescriptorSet([&]() {
        AddDescriptorTarget(std::make_shared<BufferDescriptorTarget>(
            m_computeContext.lock()->get_address_buffer(),
            (int)Gaussian_Binding_Index::eAddress,
            vk::ShaderStageFlagBits::eCompute,
            vk::DescriptorType::eStorageBuffer,
            get_DescriptorSet()));
    });
    ComputePass<GaussianContext>:: prepare_pipeline(sizeof(PushContant_Sum));
}
}
