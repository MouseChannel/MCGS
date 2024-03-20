#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace MCGS
{
    std::vector<std::vector<float>> load_ply(std::string ply_path);
    std::vector<float> get_sub(std::vector<std::vector<float>>& origin,
                               int from,
                               int end);

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
        for (int i = 0; i < data.size(); i += 45)
        {
            for (int j = 0; j < 45; j++)
            {
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
        std::for_each(data.begin(), data.end(), [](auto& item)
        {
            item = 1 / (1 + std::exp(-item));
        });
        return data;
    }

    inline std::vector<float> get_scale(std::vector<std::vector<float>>& origin)
    {
        auto data = get_sub(origin, 55, 58);
        std::for_each(data.begin(), data.end(), [](auto& item)
        {
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

        for (int i = 0; i < data.size() / 4; i++)
        {
            float cur_sum = 0.;
            for (int j = 0; j < 4; j++)
            {
                cur_sum += std::pow(data[i * 4 + j], 2);
            }
            auto div = std::sqrt(cur_sum);
            for (int j = 0; j < 4; j++)
            {
                data[i * 4 + j] /= div;
            }
        }
        return data;
    }
}
