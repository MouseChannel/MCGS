#include "config.h"
#include "ply_loader.hpp"
#include "rasterizer.h"
#include "torch/torch.h"
#include "torch/utils.h"
#include <cstring>
#include <fstream>
#include <iostream>
// #include <opencv2/opencv.hpp>
#include <vector>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

std::function<char*(size_t N)> resizeFunctional(torch::Tensor& t)
{
    auto lambda = [&t](size_t N) {
        t.resize_({ (long long)N });
        return reinterpret_cast<char*>(t.contiguous().data_ptr());
    };
    return lambda;
}

std::tuple<int, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor, torch::Tensor>
RasterizeGaussiansCUDA(
    const torch::Tensor& background,
    const torch::Tensor& means3D,
    const torch::Tensor& colors,
    const torch::Tensor& opacity,
    const torch::Tensor& scales,
    const torch::Tensor& rotations,
    const float scale_modifier,
    const torch::Tensor& cov3D_precomp,
    const torch::Tensor& viewmatrix,
    const torch::Tensor& projmatrix,
    const float tan_fovx,
    const float tan_fovy,
    const int image_height,
    const int image_width,
    const torch::Tensor& sh,
    const int degree,
    const torch::Tensor& campos,
    const bool prefiltered,
    const bool debug)
{
    if (means3D.ndimension() != 2 || means3D.size(1) != 3) {
        AT_ERROR("means3D must have dimensions (num_points, 3)");
    }

    const int P = means3D.size(0);
    const int H = image_height;
    const int W = image_width;

    auto int_opts = means3D.options().dtype(torch::kInt32);
    auto float_opts = means3D.options().dtype(torch::kFloat32);

    torch::Tensor out_color = torch::full({ NUM_CHANNELS, H, W }, 0.0, float_opts);
    torch::Tensor radii =
        torch::full({ P }, 0, means3D.options().dtype(torch::kInt32));

    torch::Device device(torch::kCUDA);
    torch::TensorOptions options(torch::kByte);
    torch::Tensor geomBuffer = torch::empty({ 0 }, options.device(device));
    torch::Tensor binningBuffer = torch::empty({ 0 }, options.device(device));
    torch::Tensor imgBuffer = torch::empty({ 0 }, options.device(device));
    std::function<char*(size_t)> geomFunc = resizeFunctional(geomBuffer);
    std::function<char*(size_t)> binningFunc = resizeFunctional(binningBuffer);
    std::function<char*(size_t)> imgFunc = resizeFunctional(imgBuffer);

    int rendered = 0;
    if (P != 0) {
        int M = 0;
        if (sh.size(0) != 0) {
            M = sh.size(1);
        }

        rendered = CudaRasterizer::Rasterizer::forward(
            geomFunc,
            binningFunc,
            imgFunc,
            P,
            degree,
            M,
            background.contiguous().data<float>(),
            W,
            H,
            means3D.contiguous().data<float>(),
            sh.contiguous().data<float>(),
            colors.contiguous().data<float>(),
            opacity.contiguous().data<float>(),
            scales.contiguous().data<float>(),
            scale_modifier,
            rotations.contiguous().data<float>(),
            cov3D_precomp.contiguous().data<float>(),
            viewmatrix.contiguous().data<float>(),
            projmatrix.contiguous().data<float>(),
            campos.contiguous().data<float>(),
            tan_fovx,
            tan_fovy,
            prefiltered,
            out_color.contiguous().data<float>(),
            radii.contiguous().data<int>(),
            debug);
    }
    return std::make_tuple(rendered, out_color, radii, geomBuffer, binningBuffer, imgBuffer);
}

int main()
{

    auto gs_data = MCGS::load_ply("/home/mocheng/project/MCGS/point_cloud.ply");

    auto background = torch::tensor({ 0., 0., 0. });

    auto xyz = MCGS::get_xyz(gs_data);
    auto scale_d = MCGS::get_scale(gs_data);
    auto dc_012 = MCGS::get_dc_012(gs_data);
    auto dc_rest = MCGS::get_dc_rest(gs_data);

    auto opacity_d = MCGS::get_opacity(gs_data);
    auto rotations_d = MCGS::get_rotation(gs_data);

    torch::Tensor tensor_xyz = torch::from_blob(xyz.data(), xyz.size(), torch::kFloat);

    torch::Tensor tensor_dc_012 = torch::from_blob(dc_012.data(), dc_012.size(), torch::kFloat);

    torch::Tensor tensor_dc_rest = torch::from_blob(dc_rest.data(), dc_rest.size(), torch::kFloat);
    // std::vector<float> feature = dc_012;
    std::vector<float> feature(dc_012.size() + dc_rest.size());
    for (int i = 0; i < dc_012.size() / 3; i++) {
        for (int j = 0; j < 3; j++) {

            feature[i * 48 + j] = dc_012[i * 3 + j];
        }
        for (int j = 0; j < 45; j++) {
            feature[i * 48 + 3 + j] = dc_rest[i * 45 + j];
        }
    }
    torch::Tensor tensor_feature = torch::from_blob(feature.data(), feature.size(), torch::kFloat);
    torch::Tensor tensor_opacity_d = torch::from_blob(opacity_d.data(), opacity_d.size(), torch::kFloat);

    torch::Tensor tensor_scale_d = torch::from_blob(scale_d.data(), scale_d.size(), torch::kFloat);

    torch::Tensor tensor_rotations_d = torch::from_blob(rotations_d.data(), rotations_d.size(), torch::kFloat);

    auto proj = torch::tensor({ -2.760816,
                                .300833,
                                -0.021124,
                                -0.021122,
                                .306501,
                                2.70976,
                                -0.190277,
                                -0.190258,
                                -0.,
                                -0.531742,
                                -0.981605,
                                -0.981507,
                                -0.,
                                -0.,
                                4.021532,
                                4.031129 },
                              torch::kFloat);

    auto c2w = torch::tensor({ -.993989,
                               .1083,
                               -.021122,
                               0.,
                               .11034,
                               .97551,
                               -.19026,
                               0.,
                               0.,
                               -.19143,
                               -.98151,
                               0.,
                               0.,
                               0.,
                               4.0311,
                               1. },
                             torch::kFloat);

    auto camera_pos = torch::tensor({ 0.0851, 0.7670, 0.39566 }, torch::kFloat);
    auto color = torch::tensor({}, torch::kFloat);
    auto pre_comp = torch::tensor({}, torch::kFloat);

    auto res = RasterizeGaussiansCUDA(background,
                                      tensor_xyz.reshape({ -1, 3 }),
                                      color,
                                      tensor_opacity_d.reshape({ -1, 1 }),
                                      tensor_scale_d.reshape({ -1, 3 }),
                                      tensor_rotations_d.reshape({ -1, 4 }),
                                      1.f,
                                      pre_comp,
                                      c2w.reshape({ 4, 4 }),
                                      proj.reshape({ 4, 4 }),
                                      0.36,
                                      0.36,
                                      800,
                                      800,
                                      tensor_feature.reshape({ -1, 16, 3 }),
                                      3,
                                      camera_pos,
                                      false,
                                      true);

    auto rr = std::get<1>(res);
    auto num_rendered = std::get<0>(res);

    rr = rr.permute({ 1, 2, 0 });

    rr = rr.contiguous();
    auto tt = rr.type();
    auto data_ptr = rr.data_ptr<float>();
    size_t vector_size = rr.numel();
    std::vector<float> vec(data_ptr, data_ptr + vector_size);
    std::for_each(vec.begin(),
                  vec.end(),
                  [](float& i) {
                      i *= 255.f;
                  });

    std::vector<unsigned char> data(800 * 800 * 3);
    for (int i = 0; i < data.size(); i++) {

        data[i] = (unsigned char)vec[i];
    }

    for (int i = 0; i < data.size(); i++) {
    }

    auto suc = stbi_write_jpg("/home/mocheng/project/MCGS/main1.jpg", 800, 800, 3, data.data(), 800 * 3);
    std::cout << suc << std::endl;

    return 0;
}
