#include "rasterPass.hpp"
#include "Helper/DescriptorSetTarget/ImageDescriptorTarget.hpp"
namespace MCGS {
Raster_Pass::Raster_Pass(std::weak_ptr<GaussianContext> computeContext)
    : ComputePass<GaussianContext>(computeContext)
{
    Init();
}
void Raster_Pass::Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z)
{

    cmd.pushConstants<uint32_t>(
        get_pipeline()
            ->get_layout(),
        vk::ShaderStageFlagBits::eCompute,
        0,
        1);

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
    cmd.dispatch(50, 50, 1);
}
void Raster_Pass::Init()
{
    SetShaderModule("include/shaders/render.comp.spv");
    Prepare_DescriptorSet([&]() {
        AddDescriptorTarget(std::make_shared<BufferDescriptorTarget>(
            m_computeContext.lock()->get_address_buffer(),
            (int)Gaussian_Binding_Index::eAddress,
            vk::ShaderStageFlagBits::eCompute,
            vk::DescriptorType::eStorageBuffer,
            get_DescriptorSet()));
        AddDescriptorTarget(std::make_shared<ImageDescriptorTarget>(
            m_computeContext.lock()->get_out_image(),
            (int)Gaussian_Binding_Index::render_out_index,
            vk::ShaderStageFlagBits::eCompute,
            vk::DescriptorType::eStorageImage,
            get_DescriptorSet()));

        //        descriptor_manager->Make_DescriptorSet(render_out,
        //                                               DescriptorManager::Compute,
        //                                               (int)Gaussian_Data_Index::render_out_index,
        //                                               vk::DescriptorType::eStorageImage,
        //                                               vk::ShaderStageFlagBits::eCompute);
    });
    prepare_pipeline(sizeof(uint32_t));
}
}