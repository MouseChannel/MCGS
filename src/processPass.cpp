#include "Helper/Camera.hpp"
#include "precessPass.hpp"
namespace MCGS {
Process_Pass::Process_Pass(std::weak_ptr<GaussianContext> gaussianContext)
    : ComputePass(gaussianContext)
{
    Init();
}

void Process_Pass::Init()
{
    ComputePass<GaussianContext>::SetShaderModule("include/shaders/process.comp.spv");
    Prepare_DescriptorSet([&]() {
        AddDescriptorTarget(std::make_shared<BufferDescriptorTarget>(
            m_computeContext.lock()->get_address_buffer(),
            (int)Gaussian_Binding_Index::eAddress,
            vk::ShaderStageFlagBits::eCompute,
            vk::DescriptorType::eStorageBuffer,
            get_DescriptorSet()));
    });
    prepare_pipeline(sizeof(PushContant_GS));
}

void Process_Pass::Dispach(vk::CommandBuffer cmd, int local_size_x, int local_size_y, int local_size_z)
{
    auto m_computeContextp = m_computeContext.lock();

    //    auto camera = m_computeContextp->get_camera();
    //    camera->Get_p_matrix();

    PushContant_GS pc {
        .viewMatrix = { -.993989f,
                        .1083f,
                        -.021122f,
                        0.f,

                        .11034f,
                        .97551f,
                        -.19026f,
                        0.f,

                        0.f,
                        -.19143f,
                        -.98151f,
                        0.f,

                        0.f,
                        0.f,
                        4.0311f,
                        1.f },
        .projMatrix = { -2.760816f,
                        .300833f,
                        -0.021124f,
                        -0.021122f,
                        .306501f,
                        2.70976f,
                        -0.190277f,
                        -0.190258f,
                        -0.f,
                        -0.531742f,
                        -0.981605f,
                        -0.981507f,
                        -0.f,
                        -0.f,
                        4.021532f,
                        4.031129f },
        .campos = { 0.0851f,
                    0.7670f,
                    0.39566f },
        .tanfov = 0.36f,
        .point_num = static_cast<int>(m_computeContextp->get_point_num())
    };
    {

        auto camera = Context::Get_Singleton()->get_camera();
        auto fov = 60.f / 180.0f * float(M_PI);
        auto tfov = tan(fov * 0.5f);
        auto pmatrix = camera->Get_p_matrix();
        pmatrix[1][1] *=-1; 
        pc = PushContant_GS {
            .viewMatrix = camera->Get_v_matrix(),
            .projMatrix = pmatrix * camera->Get_v_matrix(),
            .campos = camera->get_pos(),
            .tanfov = float(tfov),
            //        .tanfov = 0.36f,

            .point_num = static_cast<int>(m_computeContextp->get_point_num())
        };
        
    }
    cmd.pushConstants<PushContant_GS>(
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

    cmd.dispatch(ceil(float(m_computeContextp->get_point_num()) / 256), 1, 1);
}

}