#include "duplicateWithKeysPass.hpp"
namespace MCGS {
duplicate_Pass::duplicate_Pass(std::weak_ptr<GaussianContext> context)
    : ComputePass<GaussianContext>(context)
{
    Init();
}
void duplicate_Pass::Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z)
{
    auto contextp = m_computeContext.lock();
    PushContant_Dup pc {
        .g_num_elements = contextp->get_point_num()
        // .second_step = 1024
    };
    cmd.pushConstants<PushContant_Dup>(
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
void duplicate_Pass::Init()
{
    SetShaderModule("include/shaders/duplicate.comp.spv");
    Prepare_DescriptorSet([&]() {
        AddDescriptorTarget(std::make_shared<BufferDescriptorTarget>(
            m_computeContext.lock()->get_address_buffer(),
            (int)Gaussian_Binding_Index::eAddress,
            vk::ShaderStageFlagBits::eCompute,
            vk::DescriptorType::eStorageBuffer,
            get_DescriptorSet()));
    });
    ComputePass<GaussianContext>::prepare_pipeline(sizeof(PushContant_GS));
}
}