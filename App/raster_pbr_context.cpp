#include "raster_pbr_context.hpp"
#include "Helper/Camera.hpp"
#include "Helper/CommandManager.hpp"
#include "Helper/DescriptorManager.hpp"
#include "Helper/Model_Loader/Obj_Loader.hpp"
#include "Helper/Model_Loader/gltf_loader.hpp"
#include "Rendering/ComputePass.hpp"
#include "Rendering/GraphicPass.hpp"

#include "Rendering/Model.hpp"
#include "Rendering/RaytracingPass.hpp"
#include "Wrapper/DescriptorSet.hpp"
#include "Wrapper/Pipeline/Graphic_Pipeline.hpp"
#include "Wrapper/Pipeline/RT_pipeline.hpp"
#include "Wrapper/Ray_Tracing/AS_Builder.hpp"
#include "Wrapper/Shader_module.hpp"
#include "Wrapper/Skybox.hpp"
#include "Wrapper/Texture.hpp"
#include "iostream"

#include "shaders/PBR/IBL/binding.h"

#include "example/raster/shader/Binding.h"
#include "shaders/PBR/IBL/push_constants.h"

#include "example/raster/shader/Constants.h"

#include "Rendering/AntiAliasing/TAA/TAA_Manager.hpp"
#include "Rendering/PBR/IBL_Manager.hpp"
#include "gaussian_manager.hpp"
#include "sort_pass.hpp"
#include "sum_pass.hpp"

#include <duplicateWithKeys_pass.hpp>
#include <Helper/Model_Loader/ImageWriter.hpp>
#include <execution>
#include <identify_pass.hpp>
#include <precess_pass.hpp>
#include <raster_pass.hpp>

namespace MCRT {
std::unique_ptr<Context> Context::_instance{ new MCRT::raster_context_pbr };
// float raster_context_pbr::light_pos_x = 0, raster_context_pbr::light_pos_y = 0, raster_context_pbr::light_pos_z = 5, raster_context_pbr::gamma = 2.2f;
// bool raster_context_pbr::use_normal_map = false, raster_context_pbr::use_r_rm_map = false, raster_context_pbr::use_ao = false;
// int irradiance_size = 512;

raster_context_pbr::raster_context_pbr()
{

    // std::vector<int> a{ 1, 2, 3, 4, 5, 6 };
    // std::for_each(std::execution::par,a.begin(),
    //               a.end(),
    //               [](auto& it) {
    //                   std::cout << it << std::endl;
    //               });
    // int dd = 0;
}

raster_context_pbr::~raster_context_pbr()
{
}

void raster_context_pbr::prepare(std::shared_ptr<Window> window)
{
    raster_context::prepare(window);
    // m_camera->m_position.z = 3;
    // sky_box.reset(new Skybox("/home/mocheng/project/MCRT/assets/Cubemap/kart_club_4k.hdr"));
    // skybox_mesh = GLTF_Loader::load_skybox("/home/mocheng/project/MCRT/assets/cube.gltf");

    // GLTF_Loader::load_model("/home/mocheng/project/MCRT/assets/Drill_01_4k.gltf/Drill_01_4k.gltf");
    // GLTF_Loader::load_model("/home/mocheng/project/MCRT/assets/pbr/temp/untitl1.gltf");
    // Obj_loader::load_model("/home/mocheng/project/MCRT/assets/untitled.obj");
    // IBLManager::Get_Singleton()->Init(sky_box);
    MCGS::GaussianManager::Get_Singleton()->Init();
    // NoiseManager::Get_Singleton()->InitPerlinNoise();

    // std::shared_ptr<MCGS::SortPass> sort_pass;
    // sort_pass.reset(new MCGS::SortPass);
    // sort_pass->Init();
    // sort_pass->Execute();

    // std::shared_ptr<MCGS::SumPass> sum_pass;
    // sum_pass.reset(new MCGS::SumPass);
    // sum_pass->Init();
    // sum_pass->Execute();

    // MCGS::SortPass::Get_Singleton()->execute();

    std::vector<uint32_t> indices{
        0,
        1,
        2,
        0,
        3,
        1

    };

    index_buffer = Buffer::CreateDeviceBuffer(indices.data(), indices.size() * sizeof(indices[0]), vk::BufferUsageFlagBits::eIndexBuffer);

    std::vector<Vertex> vertex{
        Vertex{
            .pos{ 1.0f, -1.0f, 0 },
            .texCoord{ 1.0f, 1.0f } },
        Vertex{
            .pos{ -1.0f, 1.0f, 0 },
            .texCoord{ 0.0f, 0.0f } },
        Vertex{
            .pos{ -1.0f, -1.0f, 0 },
            .texCoord{ 0.0f, 1.0f } },
        Vertex{
            .pos{ 1.0f, 1.0f, 0 },
            .texCoord{ 1.0f, 0.0f } },
    };
    m_vertex_buffer = Buffer::CreateDeviceBuffer(vertex.data(), vertex.size() * sizeof(vertex[0]), vk::BufferUsageFlagBits::eVertexBuffer);

    PASS.resize(1);

    {

        PASS[Pass_index::Graphic] = std::shared_ptr<GraphicPass>{ new GraphicPass(m_device) };
        Context::Get_Singleton()
            ->get_graphic_context()
            ->set_constants_size(sizeof(PC_Raster));
        auto graphic_context = std::reinterpret_pointer_cast<GraphicPass>(PASS[Graphic]);
        if (graphic_context == nullptr) {
            throw std::runtime_error("not graphic context");
        }

        std::vector<std::shared_ptr<ShaderModule>> graphic_shader_modules(Graphic_Pipeline::shader_stage_count);
        graphic_shader_modules[Graphic_Pipeline::Main_VERT].reset(new ShaderModule("/home/mocheng/project/MCGS/App/shader/vert.vert.spv"));
        graphic_shader_modules[Graphic_Pipeline::Main_FRAG].reset(new ShaderModule("/home/mocheng/project/MCGS/App/shader/frag.frag.spv"));
        // graphic_shader_modules[Graphic_Pipeline::Skybox_VERT].reset(new ShaderModule("/home/mocheng/project/MCRT/example/raster/shader/skybox.vert.spv"));
        // graphic_shader_modules[Graphic_Pipeline::Skybox_FRAG].reset(new ShaderModule("/home/mocheng/project/MCRT/example/raster/shader/skybox.frag.spv"));
        graphic_context->prepare();
        graphic_context->prepare_descriptorset([&]() {
            auto descriptor_manager = graphic_context->get_descriptor_manager();

          
            descriptor_manager->Make_DescriptorSet(std::vector{ MCGS::GaussianManager::Get_Singleton()->render_content->render_out },
                                                   DescriptorManager::Graphic,
                                                   1,
                                                   vk::DescriptorType::eCombinedImageSampler,
                                                   vk::ShaderStageFlagBits::eCompute|vk::ShaderStageFlagBits::eFragment);

        });
        graphic_context->prepare_pipeline(graphic_shader_modules, { graphic_context->get_descriptor_manager()->get_DescriptorSet(DescriptorManager::Graphic) }, sizeof(PC_Raster));

        graphic_context->post_prepare();
    }

    {
        // IBLManager::Get_Singleton()->pre_compute_irradiance();
        // IBLManager::Get_Singleton()->pre_compute_LUT();

        
        // ImageWriter::WriteImage(IBLManager::Get_Singleton()->get_LUT());
        // ImageWriter::WriteCubemap(IBLManager::Get_Singleton()->get_irradiance()->get_handle());
    }
}

std::shared_ptr<CommandBuffer> raster_context_pbr::Begin_Frame()
{

    CommandManager::ExecuteCmd(Context::Get_Singleton()
                               ->get_device()
                               ->Get_Graphic_queue(),
                               [&](vk::CommandBuffer& cmd) {
                                   cmd.updateBuffer<Camera_matrix>(camera_matrix->buffer->get_handle(),
                                                                   0,
                                                                   Camera_matrix{
                                                                       .view{ m_camera->Get_v_matrix() },
                                                                       .project{ m_camera->Get_p_matrix() } });
                               });

    // BeginComputeFrame();
    // EndComputeFrame();
    MCGS::GaussianManager::Get_Singleton()->Tick();

    return raster_context::Begin_Frame();
}

void raster_context_pbr::EndFrame()
{
    raster_context::EndFrame();
}

std::shared_ptr<CommandBuffer> raster_context_pbr::BeginGraphicFrame()
{

    // get_device()->get_handle().waitIdle();
    auto render_context = std::reinterpret_pointer_cast<GraphicPass>(PASS[Graphic]);

    std::shared_ptr<CommandBuffer> cmd = render_context->BeginFrame();
    {

        cmd->get_handle().bindPipeline(vk::PipelineBindPoint::eGraphics, render_context->get_pipeline()->get_handle());
        cmd->get_handle().bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                             render_context->get_pipeline()->get_layout(),
                                             0,
                                             {
                                                 render_context->get_pipeline()->get_descriptor_sets()

                                             },

                                             {});
        cmd->get_handle().bindIndexBuffer(index_buffer->get_handle(), 0, vk::IndexType::eUint32);
        cmd->get_handle().bindVertexBuffers(0, { m_vertex_buffer->get_handle() }, { 0 });
        render_context->record_command(cmd);
        cmd->get_handle()
           .drawIndexed(6, 1, 0, 0, 0);
    }

    return cmd;
}

void raster_context_pbr::EndGraphicFrame()
{
    auto& m_render_context = PASS[Graphic];
    m_render_context->Submit();
    m_render_context->EndFrame();
}


}