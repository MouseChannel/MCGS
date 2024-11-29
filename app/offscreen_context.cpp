// #include "Wrapper/SubPass/ToneMapSubpass.hpp"
#include "offscreen_context.hpp"
#include "Imgui/imgui.h"
#include "Wrapper/GraphicPass/UiPass.hpp"
#include "Wrapper/Pipeline/Graphic_Pipeline.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Helper/Camera.hpp"
#include "Helper/CommandManager.hpp"

#include "Helper/Model_Loader/gltf_loader.hpp"
#include "Rendering/ComputeContext.hpp"
#include "Rendering/GraphicContext.hpp"
#include "Rendering/Model.hpp"
#include "Wrapper/CommandBuffer.hpp"
#include "Wrapper/GraphicPass/GraphicPass.hpp"
// #include "Wrapper/Skybox.hpp"
#include "Wrapper/Texture.hpp"

#include "Helper/DescriptorSetTarget/BufferDescriptorTarget.hpp"
#include "Helper/DescriptorSetTarget/ImageDescriptorTarget.hpp"

#include "GSContext.hpp"
#include "Rendering/PBR/IBL_Manager.hpp"
#include <Helper/Model_Loader/ImageWriter.hpp>
namespace MCRT {
std::unique_ptr<Context> Context::_instance { new MCRT::offscreen_context };
float offscreen_context::light_pos_x = 0, offscreen_context::light_pos_y = 0, offscreen_context::light_pos_z = 5, offscreen_context::gamma = 2.2f;
bool offscreen_context::use_normal_map = false, offscreen_context::use_r_rm_map = false, offscreen_context::use_ao = false;
int irradiance_size = 512;

enum DescriptorSetsIndex {
    MAIN,
    DESCRIPTORSET_COUNT
};
enum PASS {
    eMainPass,
    eUIPass,
    PassCount
};
enum PIPELINE {
    eMainPipeline,
    // eUIPipeline,
    PipelineCount
};

offscreen_context::offscreen_context()
{
}

offscreen_context::~offscreen_context()
{
}

void offscreen_context::prepare(std::shared_ptr<Window> window)
{
    raster_context::prepare(window);

    std::vector<Vertex> vertexs {
        { .pos { 1, 1, 0 }, .texCoord { 1, 1 } },
        { .pos { 1, -3, 0 }, .texCoord { 1, -1 } },
        { .pos { -3, 1, 0 }, .texCoord { -1, 1 } }
    };
    std::vector<uint32_t> faces { 0, 1, 2 };
    offscreen_mesh.reset(new Mesh("offscreen", vertexs, faces, {}));

    PASS.resize(2);
    {
        PASS[1] = std::make_shared<GSContext>("C:/Users/moche/project/gs/gaussian-splatting/output/80127b2a-4/point_cloud/iteration_30000/point_cloud.ply");
        PASS[1] = std::make_shared<GSContext>("point_cloud.ply");

        PASS[1]->prepare();
    }
    {
        PASS[Pass_index::Graphic] = std::shared_ptr<GraphicContext> { new GraphicContext(m_device) };

        auto graphic_context = std::reinterpret_pointer_cast<GraphicContext>(PASS[Graphic]);
        if (graphic_context == nullptr) {
            throw std::runtime_error("not graphic context");
        }

        graphic_context->prepare();

        {
            auto swapchain_renderTarget = graphic_context->AddSwapchainRenderTarget();
            auto depth_renderTarget = graphic_context->AddDepthRenderTarget();

            {
                graphic_context->descriptorSets.resize(DESCRIPTORSET_COUNT);
                graphic_context->descriptorSetPools.resize(DESCRIPTORSET_COUNT);
                graphic_context->descriptorSets[MAIN] = std::make_shared<DescriptorSet>();
            }
            {
                graphic_context->graphicPass.resize(PassCount);

                for (int i = 0; i < PassCount; i++) {

                    graphic_context->graphicPass[i] = std::make_shared<GraphicPass>(graphic_context.get());
                    graphic_context->graphicPass[i]->set_subpass_index(i);
                }
            }


            {
                graphic_context->descriptorSets[MAIN]->AddBufferDescriptorTarget(std::reinterpret_pointer_cast<GSContext>(PASS[1])->instance_buffer,
                                                                                 1,
                                                                                 vk::ShaderStageFlagBits::eVertex,
                                                                                 vk::DescriptorType::eStorageBuffer);
                // if (graphic_context->descriptorSets[MAIN]->check_dirty()) {
                graphic_context->descriptorSetPools[MAIN].reset(new DescriptorPool({ graphic_context->descriptorSets[MAIN] }, graphic_context->get_frame_count()));
                graphic_context->descriptorSets[MAIN]->build(graphic_context->descriptorSetPools[MAIN], graphic_context->get_frame_count());
                // }
            }
            graphic_context->m_pipelines.resize(PipelineCount);
            {
                auto mainPass = graphic_context->graphicPass[eMainPass];
                mainPass->link_renderTarget({ swapchain_renderTarget },
                                            {depth_renderTarget},
                                            {},
                                            {});
                auto uiPass = graphic_context->graphicPass[eUIPass];
                uiPass->link_renderTarget({ swapchain_renderTarget },
                                          {},
                                          {},
                                          {});
            }
            {
                graphic_context->AddSubPassDependency(vk::SubpassDependency()
                                                          .setSrcSubpass(VK_SUBPASS_EXTERNAL)
                                                          .setDstSubpass(eMainPass)
                                                          .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                                          .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                                                          .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                                          .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite));
                graphic_context->AddSubPassDependency(vk::SubpassDependency()
                                                          .setSrcSubpass(eUIPass - 1)
                                                          .setDstSubpass(eUIPass)
                                                          .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                                          .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                                                          .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
                                                          .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite));
            }
            {
                auto gbufferPass = graphic_context->graphicPass[eMainPass];

                graphic_context->post_prepare();

                auto uiPass = std::reinterpret_pointer_cast<UIPass>(graphic_context->graphicPass[eUIPass]);
                uiPass->Init();
            }
            {
                auto rr = std::reinterpret_pointer_cast<GSContext>(PASS[1])->set;
                graphic_context->m_pipelines[eMainPipeline]
                    .reset(
                        new Graphic_Pipeline(graphic_context->Get_render_pass(),
                                             "include/shaders/splat.vert.spv",
                                             "include/shaders/splat.frag.spv",
                                             vk::CullModeFlagBits::eBack,
                                             true,
                                             false,
                                             vk::SampleCountFlagBits::e1,
                                             graphic_context->graphicPass[eMainPass]->get_subpass_index(),
                                             //  { rr },
                                             { graphic_context->descriptorSets[MAIN] },

                                             4,
                                             vk::ShaderStageFlagBits::eFragment,
                                             graphic_context->graphicPass[eMainPass]->color_references.size(),
                                             vk::PipelineColorBlendAttachmentState()
                                                 .setBlendEnable(true)
                                                 .setSrcColorBlendFactor(vk::BlendFactor ::eOne)
                                                 .setSrcAlphaBlendFactor(vk::BlendFactor ::eOne)
                                                 .setColorBlendOp(vk::BlendOp ::eAdd)
                                                 .setDstColorBlendFactor(vk::BlendFactor ::eOneMinusSrcAlpha)
                                                 .setDstAlphaBlendFactor(vk::BlendFactor ::eOneMinusSrcAlpha)
                                                 .setAlphaBlendOp(vk::BlendOp ::eAdd)
                                                 .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)));
            }
        }
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
    auto gs_context = std::reinterpret_pointer_cast<GSContext>(PASS[1]);
    auto render_context = std::reinterpret_pointer_cast<GraphicContext>(PASS[Graphic]);

    std::shared_ptr<CommandBuffer> command = render_context->BeginFrame();
    // std::shared_ptr<CommandBuffer> command = gs_context->BeginFrame();
    // gs_context->Submit();
    // Context::Get_Singleton()->get_device()->Get_Graphic_queue().waitIdle();

    gs_context->tick(command);

    render_context->Begin_RenderPass(command);
    {

        {
            auto cmd = command->get_handle();

            cmd.setViewport(0,
                            vk::Viewport()
                                .setHeight(extent2d.height)
                                .setWidth(extent2d.width)
                                .setMinDepth(0)
                                .setMaxDepth(1)
                                .setX(0)
                                .setY(0));
            cmd.setScissor(0,
                           vk::Rect2D()
                               .setExtent(extent2d)
                               .setOffset(vk::Offset2D()
                                              .setX(0)
                                              .setY(0)));

            {
                cmd.bindPipeline(vk::PipelineBindPoint ::eGraphics, render_context->m_pipelines[eMainPipeline]->get_handle());

                cmd.bindDescriptorSets(
                    vk::PipelineBindPoint ::eGraphics,
                    render_context->m_pipelines[eMainPipeline]->get_layout(),
                    0,
                    { render_context->descriptorSets[MAIN]->get_handle()[render_context->get_cur_index()] },
                    {});

                // cmd.bindVertexBuffers(0, offscreen_mesh->get_vertex_buffer()->get_handle(), { 0 });
                cmd.bindIndexBuffer(gs_context->index_buffer->get_handle(), 0, vk::IndexType ::eUint32);
                // cmd.draw(3, 1, 0, 0);
                cmd.drawIndexedIndirect(gs_context->indirct_cmd_buffer->get_handle(), 0, 1, 0);
            }

            {
                cmd.nextSubpass(vk::SubpassContents ::eInline);
                auto uiPass = std::reinterpret_pointer_cast<UIPass>(render_context->graphicPass[eUIPass]);

                uiPass->DrawUI(cmd, []() {
                    ImGui::Text("move:[W A S D Q E]");
                    ImGui::Text("Hold left Mouse Button To Rotate!!");
                    ImGui::SliderFloat("move-sensitivity", &Context::Get_Singleton()->get_camera()->m_sensitivity, 1e-2f, 1e-1f);

                    ImGui::Checkbox("use_normal_map", &offscreen_context::use_normal_map);
                    ImGui::Checkbox("rm", &use_r_rm_map);
                    ImGui::Checkbox("AO", &offscreen_context::use_ao);
                    ImGui::Text("fps       : %7.3f", ImGui::GetIO().Framerate);
                });
            }
        }
    }
    return command;
}
void offscreen_context::EndGraphicFrame()
{
    auto& m_render_context = PASS[Graphic];
    m_render_context->Submit();
    m_render_context->EndFrame();
}

}