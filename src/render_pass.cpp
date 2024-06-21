#include "Helper/CommandManager.hpp"
#include "Helper/Model_Loader/ImageWriter.hpp"
#include "Wrapper/Image.hpp"
#include "Wrapper/Pipeline/Compute_Pipeline.hpp"
#include "Wrapper/Shader_module.hpp"
#include "gaussian_manager.hpp"
#include "raster_pass.hpp"
namespace MCGS {

void RasterPass::run_pass(vk::CommandBuffer& cmd)
{
    cmd.pushConstants<uint32_t>(
        content
            ->get_pipeline()
            ->get_layout(),
        vk::ShaderStageFlagBits::eCompute,
        0,
        1);

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
    cmd.dispatch(50, 50, 1);
    // ImageWriter::WriteImage(render_out);
    // std::cout<<"tick"<<std::endl;
}

void RasterPass::prepare_buffer()
{
    render_out.reset(new Image(800,
                               800,
                               // vk::Format::eR32G32B32A32Sfloat,
                               vk::Format::eR8G8B8A8Unorm,
                               vk::ImageType::e2D,
                               vk::ImageTiling::eOptimal,
                               vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
                               vk::ImageAspectFlagBits::eColor,
                               vk::SampleCountFlagBits::e1));
    render_out->SetImageLayout(vk::ImageLayout::eGeneral,
                               vk::AccessFlagBits::eNone,
                               vk::AccessFlagBits::eNone,
                               vk::PipelineStageFlagBits::eTopOfPipe,
                               vk::PipelineStageFlagBits::eBottomOfPipe);
}

void RasterPass::prepare_shader_pc()
{
    shader_module.reset(new ShaderModule("include/shaders/render.comp.spv"));
    pc_size = sizeof(uint32_t);
}
void RasterPass::prepare_descriptorset()
{
    content->prepare_descriptorset([&]() {
        auto descriptor_manager = content->get_descriptor_manager();

        descriptor_manager->Make_DescriptorSet(render_out,
                                               DescriptorManager::Compute,
                                               (int)Gaussian_Data_Index::render_out_index,
                                               vk::DescriptorType::eStorageImage,
                                               vk::ShaderStageFlagBits::eCompute);
        descriptor_manager->Make_DescriptorSet(GaussianManager::Get_Singleton()->get_buffer_addr(),
                                               (int)Gaussian_Data_Index::eAddress,
                                               DescriptorManager::Compute);
    });
}

}