#include "GSContext.hpp"

// pos + normal+ de_012 + de_rest+opacity + scale + rot
const int vert_attr = 3 + 3 + 3 + 45 + 1 + 3 + 4;

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
    for (int i = 0; i < dc_012.size() / 3; i++) {
        for (int j = 0; j < 3; j++) {

            feature_d[i * 48 + j] = dc_012[i * 3 + j];
        }
        for (int j = 0; j < 45; j++) {
            feature_d[i * 48 + 3 + j] = dc_rest[i * 45 + j];
        }
    }
}
void GSContext::prepare()
{
    ComputeContext::prepare();
    

    
}