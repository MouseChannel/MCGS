// #include "Wrapper/SubPass/ToneMapSubpass.hpp"
#include "lib/imgui/imgui.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Helper/Camera.hpp"
#include "Helper/CommandManager.hpp"
#include "offscreen_context.hpp"

#include "Helper/Model_Loader/gltf_loader.hpp"
#include "Rendering/ComputeContext.hpp"
#include "Rendering/GraphicContext.hpp"
#include "Rendering/Model.hpp"
#include "Wrapper/CommandBuffer.hpp"

// #include "Wrapper/Skybox.hpp"
#include "Wrapper/Texture.hpp"

#include "shaders/PBR/IBL/binding.h"

#include "Wrapper/SubPass/IMGUISubPass.hpp"

#include "shader/Binding.h"
#include "shaders/PBR/IBL/push_constants.h"

#include "example/raster/shader/Constants.h"

#include "Helper/DescriptorSetTarget/BufferDescriptorTarget.hpp"
#include "Helper/DescriptorSetTarget/ImageDescriptorTarget.hpp"

#include "Rendering/PBR/IBL_Manager.hpp"
#include "Wrapper/SubPass/OffscreenSubpass.hpp"
#include "gaussian_context.hpp"
#include <Helper/Model_Loader/ImageWriter.hpp>
// template class MCRT::ComputePass<MCGS::GaussianContext>;
namespace MCRT {
std::unique_ptr<Context> Context::_instance { new MCRT::offscreen_context };
float offscreen_context::light_pos_x = 0, offscreen_context::light_pos_y = 0, offscreen_context::light_pos_z = 5, offscreen_context::gamma = 2.2f;
bool offscreen_context::use_normal_map = false, offscreen_context::use_r_rm_map = false, offscreen_context::use_ao = false;
int irradiance_size = 512;

offscreen_context::offscreen_context()
{
}

offscreen_context::~offscreen_context()
{
}

void offscreen_context::prepare(std::shared_ptr<Window> window)
{
    raster_context::prepare(window);

    //    auto skybox_mesh = GLTF_Loader::load_skybox("assets/skybox.gltf");

    // skybox_mesh = Mesh::LoadFromFile("assets/skybox.gltf");
    // Mesh::LoadFromFile("/home/mousechannel/project/MCRT/assets/pbr/gun.gltf");

    // Mesh::LoadFromFile("/home/mousechannel/Downloads/korean_fire_extinguisher_01_4k.gltf/korean_fire_extinguisher_01_4k.gltf");

    // Mesh::LoadFromFile("assets/pbr/korean_fire_extinguisher_01_4k/korean_fire_extinguisher_01_4k.gltf");
    // Mesh::LoadFromFile("C://Users//moche//Downloads//plunger_1k.gltf//plunger_1k.gltf");
    //    Mesh::LoadFromFile("assets/pbr/korean_fire_extinguisher_01_4k/korean_fire_extinguisher_01_4k.gltf");
    //    IBLManager::Get_Singleton()->Init("assets/Cubemap/hospital_room_2_8k.hdr");
    // IBLManager::Get_Singleton()->Init("assets//Cubemap//victoria_sunset_1k.hdr");

    PASS.resize(2);

    PASS[Pass_index::Compute] = std::shared_ptr<MCGS::GaussianContext> { new MCGS::GaussianContext };
    auto compute_context = std::reinterpret_pointer_cast<MCGS::GaussianContext>(PASS[Compute]);
    compute_context->prepare();

    {
        PASS[Pass_index::Graphic] = std::shared_ptr<GraphicContext> { new GraphicContext(m_device) };
        // Context::Get_Singleton()
        //     ->get_graphic_context()
        //     ->set_constants_size(sizeof(PC_Raster));
        auto graphic_context = std::reinterpret_pointer_cast<GraphicContext>(PASS[Graphic]);
        if (graphic_context == nullptr) {
            throw std::runtime_error("not graphic context");
        }

        graphic_context->prepare();

        {
            auto swapchain_renderTarget = graphic_context->AddSwapchainRenderTarget();
            auto color_renderTarget = graphic_context->AddColorRenderTarget();
            auto depth_renderTarget = graphic_context->AddDepthRenderTarget();
            auto resolve_renderTarget = graphic_context->AddResolveRenderTarget();
            // auto gbuffer_color_renderTarget = graphic_context->AddGbufferRenderTarget(vk::Format::eR8G8B8A8Unorm);
            // auto gbuffer_depth_renderTarget = graphic_context->AddGbufferRenderTarget(vk::Format::eR8G8B8A8Unorm);
            // auto gbuffer_normal_renderTarget = graphic_context->AddGbufferRenderTarget(vk::Format::eR8G8B8A8Unorm);
            std::shared_ptr<Texture> target_texture { new Texture("/home/mousechannel/project/MCRT/assets/icon.png") };

            graphic_context->subpasses.resize(SubPass_index::SubPassCount);
            {

                {
                    graphic_context->subpasses[OffscreenSubPassIndex].reset(new OffscreenSubPass(graphic_context, OffscreenSubPassIndex));
                    auto& offscreenSubPass = graphic_context->subpasses[OffscreenSubPassIndex];
                    offscreenSubPass->link_renderTarget({ swapchain_renderTarget },
                                                        {},
                                                        {},
                                                        {});
                    offscreenSubPass->prepare_vert_shader_module("app/shader/offscreen.vert.spv");
                    offscreenSubPass->prepare_frag_shader_module("app/shader/offscreen.frag.spv");
                    offscreenSubPass->Prepare_DescriptorSet(
                        [&]() {
                            for (int i = 0; i < offscreenSubPass->get_DescriptorSetCount(); i++) {

                                offscreenSubPass->AddDescriptorTarget(std::make_shared<ImageDescriptorTarget>(
                                    compute_context->get_out_image(),
                                    //                                target_texture->get_image(),
                                    (int)Graphic_Binding::e_offscreen,
                                    vk::ShaderStageFlagBits::eFragment,
                                    vk::DescriptorType::eCombinedImageSampler,
                                    offscreenSubPass->get_DescriptorSet(),
                                    i));
                            }
                        }

                    );
                    offscreenSubPass->prepare_pipeline(sizeof(PC_Raster));
                    graphic_context->AddSubPassDependency(vk::SubpassDependency()
                                                              .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                                                              .setDstSubpass(OffscreenSubPassIndex)
                                                              .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                                              .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                                                              .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
                                                              .setDstAccessMask(vk::AccessFlagBits::eShaderRead));
                }
                {
                    graphic_context->subpasses[IMGUISubPassIndex].reset(new IMGUISubPass(graphic_context, IMGUISubPassIndex));
                    auto& imguiSubPass = graphic_context->subpasses[IMGUISubPassIndex];
                    // graphic_context->AddSubPass(imguiSubPass);
                    imguiSubPass->link_renderTarget({ swapchain_renderTarget }, {}, {}, {});
                    graphic_context->AddSubPassDependency(vk::SubpassDependency()
                                                              .setSrcSubpass(IMGUISubPassIndex - 1)
                                                              .setDstSubpass(IMGUISubPassIndex)
                                                              .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                                              .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                                                              .setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
                                                              .setDstAccessMask(vk::AccessFlagBits::eShaderRead));
                }
            }
        }

        graphic_context->post_prepare();
    }
}

std::shared_ptr<CommandBuffer> offscreen_context::Begin_Frame()
{

    return raster_context::Begin_Frame();
}

void offscreen_context::EndFrame()
{
    raster_context::EndFrame();
}

std::shared_ptr<CommandBuffer> offscreen_context::BeginGraphicFrame()
{
    // get_device()->get_handle().waitIdle();

    auto gaussian_context = std::reinterpret_pointer_cast<MCGS::GaussianContext>(PASS[Compute]);
    //    auto cmd = gaussian_context->BeginFrame();

    auto render_context = std::reinterpret_pointer_cast<GraphicContext>(PASS[Graphic]);
    std::shared_ptr<CommandBuffer> cmd = render_context->BeginFrame();
    {
        gaussian_context->Tick(cmd->get_handle());

        cmd->get_handle().updateBuffer<Camera_matrix>(camera_matrix->buffer->get_handle(),
                                                      0,
                                                      Camera_matrix {
                                                          .view { m_camera->Get_v_matrix() },
                                                          .project { m_camera->Get_p_matrix() },

                                                          .camera_pos {
                                                              m_camera->get_pos() } });
    }
    render_context->Begin_RenderPass(cmd);
    {

        {

            {
                cmd->get_handle().setViewport(0,
                                              vk::Viewport()
                                                  .setHeight(extent2d.height)
                                                  .setWidth(extent2d.width)
                                                  .setMinDepth(0)
                                                  .setMaxDepth(1)
                                                  .setX(0)
                                                  .setY(0));
                cmd->get_handle().setScissor(0,
                                             vk::Rect2D()
                                                 .setExtent(extent2d)
                                                 .setOffset(vk::Offset2D()
                                                                .setX(0)
                                                                .setY(0)));
                auto offscreenSubpass = std::reinterpret_pointer_cast<OffscreenSubPass>(render_context->get_subpasses()[OffscreenSubPassIndex]);

                cmd->get_handle().bindPipeline(vk::PipelineBindPoint::eGraphics,
                                               offscreenSubpass->get_pipeline()->get_handle());
                cmd->get_handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                                     offscreenSubpass->get_pipeline()->get_layout(),
                                                     0,
                                                     offscreenSubpass->get_DescriptorSet()->get_handle()
                                                    //  [0],
                                                      [get_graphic_context()->get_cur_index()],
                                                     //  tonemapSubpass->get_DescriptorSet()->get_handle()[0],

                                                     {});

                {
                    cmd->get_handle().bindIndexBuffer(offscreenSubpass->offscreen_mesh->get_indices_buffer()->get_handle(),
                                                      0,
                                                      vk::IndexType::eUint32);

                    cmd->get_handle().bindVertexBuffers(0,
                                                        { offscreenSubpass->offscreen_mesh->get_vertex_buffer()->get_handle() },
                                                        { 0 });
                }

                cmd->get_handle()
                    .draw(3,
                          1,
                          0,
                          0);
            }
            {
                cmd->get_handle().nextSubpass(vk::SubpassContents::eInline);
                auto imguiSubpass = std::reinterpret_pointer_cast<IMGUISubPass>(render_context->get_subpasses()[IMGUISubPassIndex]);
                imguiSubpass->drawUI(cmd, []() {
                    // ImVec4 color = ImVec4(1, 1, 1, 1);
                    ImGui::Text("move:[W A S D Q E]");
                    ImGui::Text("Hold left Mouse Button To Rotate!!");

                    ImGui::SliderFloat("move-sensitivity", &Context::Get_Singleton()->get_camera()->m_sensitivity, 1e-2f, 1e-1f);
                    // ImGui::SliderFloat("light_pos_y", &offscreen_context::light_pos_y, -2.0f, 2.0f);
                    // ImGui::SliderFloat("light_pos_z", &offscreen_context::light_pos_z, 0.f, 20.0f);
                    ImGui::Checkbox("use_normal_map", &offscreen_context::use_normal_map);
                    ImGui::Checkbox("rm", &use_r_rm_map);
                    ImGui::Checkbox("AO", &offscreen_context::use_ao);
                    // ImGui::SliderFloat("gamma", &offscreen_context::gamma, 1.0f, 4.4f);
                });
            }

            //            cmd->get_handle().endRenderPass();
            return cmd;
        }
    }
}
void offscreen_context::EndGraphicFrame()
{
    auto& m_render_context = PASS[Graphic];
    m_render_context->Submit();
    m_render_context->EndFrame();

    //    auto gaussian_context = PASS[Compute];
    //    gaussian_context->Submit();
}

}