#include "ply_loader.hpp"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <ostream>
#include <valarray>
#include <vector>
namespace MCGS {

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

} // namespace MCGS