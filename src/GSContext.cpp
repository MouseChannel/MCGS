#include "GSContext.hpp"
#include "Helper/Camera.hpp"
#include "Helper/CommandManager.hpp"
#include "Wrapper/CommandBuffer.hpp"
#include "Wrapper/ComputePass/ComputePass.hpp"
#include "shaders/DataStruct.h"
// pos + normal+ de_012 + de_rest+opacity + scale + rot
const int vert_attr = 3 + 3 + 3 + 45 + 1 + 3 + 4;
constexpr int local_size = 256;
std::vector<std::vector<float>> load_ply(std::string ply_path)
{

    FILE* fp;

    std::vector<std::vector<float>> Pts;
    int numPts;
    fp = fopen(ply_path.c_str(), "rb");

    char strLine[102400];

    char end_flag[] = "end_header ";
    char num_flag[] = "element vertex ";
    char* p;
    char num[10000];

    if (fp == NULL) {
        printf("Error:Open input.c file fail!\n");
    }

    while (!feof(fp)) // 循环读取每一行，直到文件尾
    {
        fgets(strLine, 102400, fp);

        if (strlen(strLine) == (strlen(end_flag))) {
            break;
        }

        if ((p = strstr(strLine, num_flag)) != NULL) {
            int start = strlen(num_flag);
            int sub_len = strlen(strLine) - strlen(num_flag);

            for (int i = 0; i < sub_len; i++) {
                num[i] = strLine[start + i];
            }
            numPts = atoi(num);
        }
    }

    float* pts = (float*)malloc(numPts * vert_attr * sizeof(float));

    float cnt = numPts * vert_attr;

    fread(pts, sizeof(float), cnt, fp);

    fclose(fp);

    for (int i = 0; i < numPts; i++) {
        std::vector<float> temp;
        for (int j = 0; j < vert_attr; j++) {
            temp.push_back(pts[vert_attr * i + j]);
        }
        Pts.push_back(temp);
    }

    //   std::string out_file = "bin_pts_check.txt";
    //   std::ofstream out(out_file.c_str());
    //   for (int pt_idx = 0; pt_idx < numPts; ++pt_idx) {
    //     for (int j = 0; j < vert_attr; ++j) {
    //       out << pts[pt_idx * vert_attr + j] << " ";
    //     }
    //     out << std::endl;
    //   }
    //   out.close();
    return Pts;
}

std::vector<float> get_sub(std::vector<std::vector<float>>& origin,
                           int from,
                           int end)
{
    std::vector<std::vector<float>> res(origin.size(), std::vector<float> {});
    std::transform(
        origin.begin(),
        origin.end(),
        res.begin(),
        [&](std::vector<float>& data) {
            return std::vector<float> { data.data() + from, data.data() + end };
        });
    auto len = (end - from);
    std::vector<float> d(origin.size() * len);
    for (int i = 0; i < res.size(); i++) {
        for (int j = 0; j < len; j++) {
            d[i * len + j] = res[i][j];
        }
    }
    return d;
}

inline std::vector<float> get_xyz(std::vector<std::vector<float>>& origin)
{
    return get_sub(origin, 0, 3);
}

inline std::vector<float> get_normal(std::vector<std::vector<float>>& origin)
{
    return get_sub(origin, 3, 6);
}

inline std::vector<float> get_dc_012(std::vector<std::vector<float>>& origin)
{
    return get_sub(origin, 6, 9);
}

inline std::vector<float> get_dc_rest(std::vector<std::vector<float>>& origin)
{
    auto data = get_sub(origin, 9, 54);
    return data;
    std::vector<float> fixed(data.size());
    for (int i = 0; i < data.size(); i += 45) {
        for (int j = 0; j < 45; j++) {
            auto a = j / 15;
            auto b = j % 15;
            fixed[i + b * 3 + a] = data[i + j];
        }
    }

    return fixed;
}

inline std::vector<float> get_opacity(std::vector<std::vector<float>>& origin)
{
    auto data = get_sub(origin, 54, 55);
    std::for_each(data.begin(), data.end(), [](auto& item) {
        item = 1 / (1 + std::exp(-item));
    });
    return data;
}

inline std::vector<float> get_scale(std::vector<std::vector<float>>& origin)
{
    auto data = get_sub(origin, 55, 58);
    std::for_each(data.begin(), data.end(), [](auto& item) {
        item = std::exp(item);
    });
    return data;
}

inline std::vector<float> get_rotation(std::vector<std::vector<float>>& origin)
{
    auto data = get_sub(origin, 58, 62);
    // std::for_each(data.begin(), data.end(), [](auto& item) {
    //     item = std::norma(item);
    // });

    for (int i = 0; i < data.size() / 4; i++) {
        float cur_sum = 0.;
        for (int j = 0; j < 4; j++) {
            cur_sum += std::pow(data[i * 4 + j], 2);
        }
        auto div = std::sqrt(cur_sum);
        for (int j = 0; j < 4; j++) {
            data[i * 4 + j] /= div;
        }
    }
    return data;
}

namespace MCRT {

void LookDeviceBuffer1(vk::Buffer device_buffer, int size)
{
    Context::Get_Singleton()->get_device()->Get_Graphic_queue().waitIdle();
    auto host_buffer = Buffer::create_buffer(nullptr, size, vk::BufferUsageFlagBits::eTransferDst);
    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),
                               [&](vk::CommandBuffer& cmd) {
                                   cmd.copyBuffer(
                                       device_buffer,
                                       host_buffer->get_handle(),
                                       vk::BufferCopy()
                                           .setDstOffset(0)
                                           .setSrcOffset(0)
                                           .setSize(size));
                               });
    auto cpu_raw_data = host_buffer->Get_mapped_data();
    std::vector<int> data(size / 4);
    std::memcpy(data.data(), cpu_raw_data.data(), sizeof(data));
    int r = 0;
}
GSContext::GSContext(std::string path)
{
    auto gs_data = load_ply(path);

    auto xyz_d = get_xyz(gs_data);

    std::vector<glm::vec3> xyz_3 {
        xyz_d.begin(),
        xyz_d.end() - 3
    };
    auto scale_d = get_scale(gs_data);
    auto dc_012 = get_dc_012(gs_data);
    auto dc_rest = get_dc_rest(gs_data);

    auto opacity_d = get_opacity(gs_data);
    auto rotations_d = get_rotation(gs_data);

    std::vector<float> feature_d(dc_012.size() + dc_rest.size());
    for (int j = 0; j < (int)dc_012.size() / 3; j++) {

        feature_d[j * 48] = dc_012[j * 3 + 0];
        feature_d[j * 48 + 16] = dc_012[j * 3 + 1];
        feature_d[j * 48 + 32] = dc_012[j * 3 + 2];
        for (int i = 0; i < 15; i++) {
            feature_d[j * 48 + 1 + i] = dc_rest[j * 45 + i];
            feature_d[j * 48 + 17 + i] = dc_rest[j * 45 + 15 + i];
            feature_d[j * 48 + 33 + i] = dc_rest[j * 45 + 30 + i];
        }
        // for (int i = 0; i < 3; i++) {

        //     for (int j = 0; j < 3; j++) {
        //         feature_d[j * 48 + j * 16] = dc_012[j + j + 0];

        //         feature_d[i * 48 + j] = dc_012[i * 3 + j];
        //     }
        //     for (int j = 0; j < 45; j++) {
        //         feature_d[i * 48 + 3 + j] = dc_rest[i * 45 + j];
        //     }
        // }
        int s = 0;
    }
    std::vector<GaussianPoint> raw_data_cpu(opacity_d.size());
    for (int i = 0; i < opacity_d.size(); i++) {
        auto& data = raw_data_cpu[i];
        data.opacity = opacity_d[i];
        data.pos = glm::vec3(xyz_d[i * 3],
                             xyz_d[i * 3 + 1],
                             xyz_d[i * 3 + 2]);
        data.rot = glm::vec4(
            rotations_d[i * 4 + 1],
            rotations_d[i * 4 + 2],
            rotations_d[i * 4 + 3],
            rotations_d[i * 4]);
        data.scale = glm::vec3(scale_d[i * 3],
                               scale_d[i * 3 + 1],
                               scale_d[i * 3 + 2]);

        for (int j = 0; j < 12; j++) {
            glm::vec4 temp;
            for (int k = 0; k < 4; k++) {
                temp[k] = feature_d[i * 48 + 4 * j + k];
            }
            data.sh[j] = temp;
        }
    }
    raw_data = Buffer::CreateDeviceBuffer(raw_data_cpu.data(), sizeof(raw_data_cpu[0]) * raw_data_cpu.size(), vk::BufferUsageFlagBits::eStorageBuffer);
    all_point_count = opacity_d.size();
    point_count_buffer = Buffer::CreateDeviceBuffer(
        &all_point_count,
        sizeof(all_point_count),
        vk::BufferUsageFlagBits::eStorageBuffer);

    m_gpu_sort = std::make_shared<gpusort>();
}
void GSContext::prepare()
{
    ComputeContext::prepare();
    // auto rr = new ComputePass({ sets });
    auto pre_set = std::make_shared<DescriptorSet>();
    std::shared_ptr<DescriptorPool> pre_pool;
    pre_set->AddBufferDescriptorTarget(
        point_count_buffer,
        0,
        vk::ShaderStageFlagBits::eCompute,
        vk::DescriptorType ::eStorageBuffer);
    pre_set->AddBufferDescriptorTarget(
        raw_data,
        1,
        vk::ShaderStageFlagBits::eCompute,
        vk::DescriptorType::eStorageBuffer);
    pre_pool.reset(new DescriptorPool({ pre_set }));
    pre_set->build(pre_pool, 1);
    pre_process_pass.reset(new ComputePass({ pre_set }, 4, "/home/mousechannel/project/MCGS/include/shaders/process.comp.spv"));
    CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(),
                               [&](vk::CommandBuffer& cmd) {
                                   pre_process_pass->Dispach(
                                       cmd,
                                       std::ceil(all_point_count / 256),
                                       1,
                                       1);
                               });

    key_buffer = Buffer::CreateDeviceBuffer(nullptr, all_point_count * sizeof(uint32_t), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferSrc);

    value_buffer = Buffer::CreateDeviceBuffer(nullptr, all_point_count * sizeof(uint32_t), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress | vk::BufferUsageFlagBits::eTransferSrc);

    inverse_index_buffer = Buffer::CreateDeviceBuffer(nullptr, all_point_count * sizeof(uint32_t), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst);
    vk::DrawIndexedIndirectCommand indirct_cmd;
    indirct_cmd.setFirstIndex(0)
        .setFirstInstance(0)
        .setIndexCount(0) // modified in shader
        .setInstanceCount(1)
        .setVertexOffset(0);

    indirct_cmd_buffer = Buffer::CreateDeviceBuffer(&indirct_cmd, sizeof(vk::DrawIndexedIndirectCommand), vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc);
    std::vector<uint32_t> splat_index;
    splat_index.reserve(all_point_count * 6);
    for (int i = 0; i < all_point_count; ++i) {
        splat_index.push_back(4 * i + 0);
        splat_index.push_back(4 * i + 1);
        splat_index.push_back(4 * i + 2);
        splat_index.push_back(4 * i + 2);
        splat_index.push_back(4 * i + 1);
        splat_index.push_back(4 * i + 3);
    }

    index_buffer = Buffer::CreateDeviceBuffer(
        splat_index.data(),
        splat_index.size() * sizeof(uint32_t),
        vk::BufferUsageFlagBits::eIndexBuffer);

    // auto ee = sizeof(InstancePoint);
    instance_buffer = Buffer::CreateDeviceBuffer(
        nullptr,
        all_point_count * sizeof(InstancePoint),
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eVertexBuffer);

    glm::mat4 p;
    {
        float aspect = static_cast<float>(1600) / 900;
        glm::mat4 projection = glm::perspective(glm::radians(60.f), aspect, 0.01f, 100.f);

        // // gl to vulkan projection matrix
        glm::mat4 conversion = glm::mat4(1.f);
        conversion[1][1] = -1.f;
        conversion[2][2] = 0.5f;
        conversion[3][2] = 0.5f;
        p = conversion * projection;
    }
    glm::mat4 v;

    glm::vec3 center_(0.f, 0.f, 0.f);
    // camera = center + r (sin phi sin theta, cos phi, sin phi cos theta)
    float r_ = 2.f;
    float phi_ = glm::radians(45.f);
    float theta_ = glm::radians(45.f);

    const auto sin_phi = std::sin(phi_);
    const auto cos_phi = std::cos(phi_);
    const auto sin_theta = std::sin(theta_);
    const auto cos_theta = std::cos(theta_);
    auto eye = center_ +
        r_ * glm::vec3(sin_phi * sin_theta, cos_phi, sin_phi * cos_theta);
    v = glm::lookAt(eye, center_, glm::vec3(0.f, 1.f, 0.f));

    CameraInfo camera {
        .projection = Context::Get_Singleton()->get_camera()->Get_p_matrix(),
        .view = Context::Get_Singleton()->get_camera()->Get_v_matrix(),
        .camera_position = Context::Get_Singleton()->get_camera()->get_pos(),
        // .projection = p,
        // .view = v,
        // .camera_position = eye,

        .pad = 0,
        .screen_size = glm::uvec2(1600, 900)
    };
    camera_buffer = Buffer::CreateDeviceBuffer(&camera, sizeof(CameraInfo), vk::BufferUsageFlagBits::eUniformBuffer);
    {
        set = std::make_shared<DescriptorSet>();

        set->AddBufferDescriptorTarget(indirct_cmd_buffer,
                                       e_indir_cmd,
                                       vk::ShaderStageFlagBits::eCompute,
                                       vk::DescriptorType ::eStorageBuffer);
        set->AddBufferDescriptorTarget(point_count_buffer,
                                       e_point_count,
                                       vk::ShaderStageFlagBits::eCompute,
                                       vk::DescriptorType ::eStorageBuffer);
        set->AddBufferDescriptorTarget(raw_data,
                                       e_gaussian_raw_point,
                                       vk::ShaderStageFlagBits::eCompute,
                                       vk::DescriptorType::eStorageBuffer);
        set->AddBufferDescriptorTarget(key_buffer,
                                       e_instance_key,
                                       vk::ShaderStageFlagBits::eCompute,
                                       vk::DescriptorType::eStorageBuffer);

        set->AddBufferDescriptorTarget(value_buffer,
                                       e_instance_value,
                                       vk::ShaderStageFlagBits::eCompute,
                                       vk::DescriptorType::eStorageBuffer);
        set->AddBufferDescriptorTarget(instance_buffer,
                                       e_instance_point,
                                       vk::ShaderStageFlagBits::eCompute | vk::ShaderStageFlagBits::eVertex,
                                       vk::DescriptorType::eStorageBuffer);
        set->AddBufferDescriptorTarget(inverse_index_buffer,
                                       e_inverse_index,
                                       vk::ShaderStageFlagBits::eCompute,
                                       vk::DescriptorType::eStorageBuffer);
        set->AddBufferDescriptorTarget(camera_buffer,
                                       e_camera,
                                       vk::ShaderStageFlagBits::eCompute,
                                       vk::DescriptorType::eUniformBuffer);
        setpool.reset(new DescriptorPool({ set }));
        set->build(setpool, 1);
    }
    {
        // auto rank_shader = std::make_shared<ShaderModule>("/home/mousechannel/project/MCGS/include/shaders/rank.comp.spv");
        // rank_pipeline.reset(new Compute_Pipeline(rank_shader));
        rank_pass.reset(new ComputePass(
            { set },
            sizeof(glm::mat4),
            "/home/mousechannel/project/MCGS/include/shaders/rank.comp.spv"));
    }
    {
        inverse_pass.reset(new ComputePass(
            { set },
            sizeof(glm::mat4),
            "/home/mousechannel/project/MCGS/include/shaders/inverseIndex.comp.spv"));
        ;
    }
    {
        projection_pass.reset(new ComputePass(
            { set },
            sizeof(glm::mat4),
            "/home/mousechannel/project/MCGS/include/shaders/projection.comp.spv"));
        ;
    }

    m_gpu_sort->Init(all_point_count, indirct_cmd_buffer);
}
void GSContext::tick(std::shared_ptr<CommandBuffer> command)
{
    auto cmd = command->get_handle();
    { // rank

        cmd.fillBuffer(indirct_cmd_buffer->get_handle(), 0, sizeof(uint32_t), 0);
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlagBits::eByRegion,
            vk::MemoryBarrier()
                .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
            {},
            {});

        cmd.pushConstants<glm::mat4>(
            // rank_pipeline->get_layout(),
            rank_pass->get_pipeline()->get_layout(),
            vk::ShaderStageFlagBits::eCompute,
            0,
            glm::mat4(1));
        rank_pass->Dispach(cmd, ceil(float(all_point_count) / local_size), 1, 1);
        // cmd.bindDescriptorSets(vk::PipelineBindPoint ::eCompute,
        //                        rank_pipeline->get_layout(),
        //                        0,
        //                        set->get_handle()[0],
        //                        {});
        // cmd.dispatch(ceil(float(all_point_count) / local_size), 1, 1);
    }
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                        vk::PipelineStageFlagBits::eComputeShader,
                        vk::DependencyFlagBits::eByRegion,
                        vk::MemoryBarrier()
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
                        {},
                        {});
    // LookDeviceBuffer(key_buffer->get_handle(), 1000);
    {
        m_gpu_sort->sort(cmd, key_buffer, value_buffer);
    }
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                        vk::PipelineStageFlagBits::eComputeShader,
                        vk::DependencyFlagBits::eByRegion,
                        vk::MemoryBarrier()
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
                        {},
                        {});
    {
        cmd.fillBuffer(inverse_index_buffer->get_handle(), 0, all_point_count * sizeof(uint32_t), -1);
        // vkCmdFillBuffer(cb, splat_storage_.inverse_index, 0, loaded_point_count_ * sizeof(uint32_t), -1);
        // cmd.debugMarkerBeginEXT(vk::DebugMarkerMarkerInfoEXT().setPMarkerName("inverse_pass"));
        // barrier = { VK_STRUCTURE_TYPE_MEMORY_BARRIER };
        // barrier.srcAccessMask =
        //     VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        // barrier.dstAccessMask =
        //     VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
        // vkCmdPipelineBarrier(cb,
        //                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT |
        //                          VK_PIPELINE_STAGE_TRANSFER_BIT,
        //                      VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        //                      0,
        //                      1,
        //                      &barrier,
        //                      0,
        //                      NULL,
        //                      0,
        //                      NULL);
        cmd.pipelineBarrier(
            vk::PipelineStageFlagBits::eComputeShader | vk::PipelineStageFlagBits::eTransfer,
            vk::PipelineStageFlagBits::eComputeShader,
            vk::DependencyFlagBits::eByRegion,
            vk::MemoryBarrier()
                .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead),
            {},
            {});

        inverse_pass->Dispach(cmd, ceil(float(all_point_count) / local_size), 1, 1);
        // cmd.debugMarkerEndEXT();
    }
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                        vk::PipelineStageFlagBits::eComputeShader,
                        vk::DependencyFlagBits::eByRegion,
                        vk::MemoryBarrier()
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
                        {},
                        {});
    {
        cmd.pushConstants<glm::mat4>(
            projection_pass->get_pipeline()->get_layout(),
            vk::ShaderStageFlagBits::eCompute,
            0,
            glm::mat4(1));
        // cmd.debugMarkerBeginEXT(vk::DebugMarkerMarkerInfoEXT().setPMarkerName("projection_pass"));
        projection_pass->Dispach(cmd, ceil(float(all_point_count) / local_size), 1, 1);
        // cmd.debugMarkerEndEXT();
    }

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                        vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eDrawIndirect,
                        vk::DependencyFlagBits::eByRegion,
                        vk::MemoryBarrier()
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eIndirectCommandRead | vk::AccessFlagBits::eVertexAttributeRead),
                        {},
                        {});
}
std::shared_ptr<CommandBuffer> GSContext::BeginFrame()
{
    auto command = ComputeContext::BeginFrame();
    auto cmd = command->get_handle();
    // CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(), [&](vk::CommandBuffer cmd) {
    //     { // rank
    //         cmd.pushConstants<glm::mat4>(
    //             // rank_pipeline->get_layout(),
    //             rank_pass->get_pipeline()->get_layout(),
    //             vk::ShaderStageFlagBits::eCompute,
    //             0,
    //             glm::mat4(1));
    //         rank_pass->Dispach(cmd, ceil(float(all_point_count) / local_size), 1, 1);
    //         // cmd.bindDescriptorSets(vk::PipelineBindPoint ::eCompute,
    //         //                        rank_pipeline->get_layout(),
    //         //                        0,
    //         //                        set->get_handle()[0],
    //         //                        {});
    //         // cmd.dispatch(ceil(float(all_point_count) / local_size), 1, 1);
    //     } });
    // // cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
    // //                     vk::PipelineStageFlagBits::eComputeShader,
    // //                     vk::DependencyFlagBits::eByRegion,
    // //                     vk::MemoryBarrier()
    // //                         .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
    // //                         .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
    // //                     {},
    // //                     {});
    // // LookDeviceBuffer(key_buffer->get_handle(), 1000);
    // CommandManager::ExecuteCmd(Context::Get_Singleton()->get_device()->Get_Graphic_queue(), [&](vk::CommandBuffer cmd) {
    //     {

    //         m_gpu_sort->sort(cmd, key_buffer, value_buffer);

    //         // static auto copy = vk::BufferCopy()
    //         //                        .setSize(sizeof(uint32_t))
    //         //                        .setDstOffset(0)
    //         //                        .setSrcOffset(0);
    //         // _cmd.copyBuffer(m_gpu_sort->indirect_buffer->get_handle(), m_gpu_sort->storage_buffer->get_handle(), copy);
    //     }
    // });
    // LookDeviceBuffer1(value_buffer->get_handle(), 10000);
    // LookDeviceBuffer1(m_gpu_sort->storage_buffer->get_handle(), 20);
    // LookDeviceBuffer1(m_gpu_sort->storage_buffer->get_handle(), 20);

    cmd.pushConstants<glm::mat4>(
        // rank_pipeline->get_layout(),
        rank_pass->get_pipeline()->get_layout(),
        vk::ShaderStageFlagBits::eCompute,
        0,
        glm::mat4(1));
    rank_pass->Dispach(cmd, ceil(float(all_point_count) / local_size), 1, 1);
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                        vk::PipelineStageFlagBits::eComputeShader,
                        vk::DependencyFlagBits::eByRegion,
                        vk::MemoryBarrier()
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
                        {},
                        {});
    m_gpu_sort->sort(cmd, key_buffer, value_buffer);

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                        vk::PipelineStageFlagBits::eComputeShader,
                        vk::DependencyFlagBits::eByRegion,
                        vk::MemoryBarrier()
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
                        {},
                        {});
    {
        // cmd.debugMarkerBeginEXT(vk::DebugMarkerMarkerInfoEXT().setPMarkerName("inverse_pass"));
        inverse_pass->Dispach(cmd, ceil(float(all_point_count) / local_size), 1, 1);
        // cmd.debugMarkerEndEXT();
    }
    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                        vk::PipelineStageFlagBits::eComputeShader,
                        vk::DependencyFlagBits::eByRegion,
                        vk::MemoryBarrier()
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eShaderRead),
                        {},
                        {});
    {
        cmd.pushConstants<glm::mat4>(
            projection_pass->get_pipeline()->get_layout(),
            vk::ShaderStageFlagBits::eCompute,
            0,
            glm::mat4(1));
        // cmd.debugMarkerBeginEXT(vk::DebugMarkerMarkerInfoEXT().setPMarkerName("projection_pass"));

        projection_pass->Dispach(cmd, ceil(float(all_point_count) / local_size), 1, 1);
        // cmd.debugMarkerEndEXT();
    }

    cmd.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader,
                        vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eDrawIndirect,
                        vk::DependencyFlagBits::eByRegion,
                        vk::MemoryBarrier()
                            .setSrcAccessMask(vk::AccessFlagBits::eShaderWrite)
                            .setDstAccessMask(vk::AccessFlagBits::eIndirectCommandRead | vk::AccessFlagBits::eVertexAttributeRead),
                        {},
                        {});
    return command;
}
}