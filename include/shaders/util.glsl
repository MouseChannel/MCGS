// #ifdef __cplusplus
// #include "glm/glm.hpp"
// using mat4 = glm::mat4;
// using vec3 = glm::vec3;
// using vec4 = glm::vec4;
// #endif
#include "./allbuffer_reference.glsl"
#include "./config.glsl"
// #define M 16
float SH_C0 = 0.28209479177387814f;
float SH_C1 = 0.4886025119029199f;
float SH_C2[] = {
    1.0925484305920792f,
    -1.0925484305920792f,
    0.31539156525252005f,
    -1.0925484305920792f,
    0.5462742152960396f
};
float SH_C3[] = {
    -0.5900435899266435f,
    2.890611442640554f,
    -0.4570457994644658f,
    0.3731763325901154f,
    -0.4570457994644658f,
    1.445305721320277f,
    -0.5900435899266435f
};
vec4 transformPoint4x4(vec3 p, mat4 matrix)
{
    vec4 transformed = {
        matrix[0][0] * p.x + matrix[1][0] * p.y + matrix[2][0] * p.z + matrix[3][0],
        matrix[0][1] * p.x + matrix[1][1] * p.y + matrix[2][1] * p.z + matrix[3][1],
        matrix[0][2] * p.x + matrix[1][2] * p.y + matrix[2][2] * p.z + matrix[3][2],
        matrix[0][3] * p.x + matrix[1][3] * p.y + matrix[2][3] * p.z + matrix[3][3]
    };
    return transformed;
}
vec3 transformPoint4x3(vec3 p, mat4 matrix)
{
    vec3 transformed = {
        matrix[0][0] * p.x + matrix[1][0] * p.y + matrix[2][0] * p.z + matrix[3][0],
        matrix[0][1] * p.x + matrix[1][1] * p.y + matrix[2][1] * p.z + matrix[3][1],
        matrix[0][2] * p.x + matrix[1][2] * p.y + matrix[2][2] * p.z + matrix[3][2],

    };
    return transformed;
}
float ndc2Pix(float v, int S)
{
    return ((v + 1.0) * S - 1.0) * 0.5;
}
void getRect(vec2 p, int max_radius, inout uvec2 rect_min, inout uvec2 rect_max, uvec3 grid)
{
    rect_min = uvec2(
        min(grid.x,
            max(int(0),
                int((p.x - max_radius) / BLOCK_X))),
        min(grid.y,
            max(int(0),
                int((p.y - max_radius) / BLOCK_Y))));
    rect_max = uvec2(
        min(grid.x,
            max(int(0), int((p.x + max_radius + BLOCK_X - 1) / BLOCK_X))),
        min(grid.y,
            max(int(0), int((p.y + max_radius + BLOCK_Y - 1) / BLOCK_Y))));
}

bool in_frustum(int idx,
                uint64_t xyz_address,
                mat4 viewmatrix,
                mat4 projmatrix,
                bool prefiltered,
                inout vec3 p_view)
{
    _xyz xyz = _xyz(xyz_address);
    // vec3 p_orig = { xyz.xyz[3 * idx], xyz.xyz[3 * idx + 1], xyz.xyz[3 * idx + 2] };

    vec3 p_orig = xyz.xyz[idx];
    // if (idx == 12345) {
    //     debugPrintfEXT("messagein %f %f %f\n", p_orig.x, p_orig.y, p_orig.z);
    // }

    // vec4 p_hom = transformPoint4x4(p_orig, projmatrix);
    // float p_w = 1.0f / (p_hom.w + 0.0000001f);
    // vec3 p_proj = { p_hom.x * p_w, p_hom.y * p_w, p_hom.z * p_w };

    p_view = transformPoint4x3(p_orig, viewmatrix);
    if (p_view.z <= 0.2f) // || ((p_proj.x < -1.3 || p_proj.x > 1.3 || p_proj.y < -1.3 || p_proj.y > 1.3)))
    {
        return false;
    }
    return true;
}
void computeCov3D(vec3 scale, float modify, vec4 rot, inout float[6] cov3D)
{
    mat3 S = mat3(1.f);
    S[0][0] = modify * scale.x;
    S[1][1] = modify * scale.y;
    S[2][2] = modify * scale.z;

    vec4 q = rot;
    float r = q.x;
    float x = q.y;
    float y = q.z;
    float z = q.w;

    mat3 R = mat3(
        1.f - 2.f * (y * y + z * z),
        2.f * (x * y - r * z),
        2.f * (x * z + r * y),
        2.f * (x * y + r * z),
        1.f - 2.f * (x * x + z * z),
        2.f * (y * z - r * x),
        2.f * (x * z - r * y),
        2.f * (y * z + r * x),
        1.f - 2.f * (x * x + y * y));
    mat3 M = S * R;
    mat3 Sigma = transpose(M) * M;
    cov3D[0] = Sigma[0][0];
    cov3D[1] = Sigma[0][1];
    cov3D[2] = Sigma[0][2];
    cov3D[3] = Sigma[1][1];
    cov3D[4] = Sigma[1][2];
    cov3D[5] = Sigma[2][2];
}

vec3 computeCov2D(vec3 mean, float focal_x, float focal_y, float tan_fovx, float tan_fovy, float[6] cov3D, mat4 viewmatrix)
{
    // The following models the steps outlined by equations 29
    // and 31 in "EWA Splatting" (Zwicker et al., 2002).
    // Additionally considers aspect / scaling of viewport.
    // Transposes used to account for row-/column-major conventions.
    vec3 t = transformPoint4x3(mean, viewmatrix);

    float limx = 1.3f * tan_fovx;
    float limy = 1.3f * tan_fovy;
    float txtz = t.x / t.z;
    float tytz = t.y / t.z;
    t.x = min(limx, max(-limx, txtz)) * t.z;
    t.y = min(limy, max(-limy, tytz)) * t.z;

    mat3 J = mat3(
        focal_x / t.z,
        0.0f,
        -(focal_x * t.x) / (t.z * t.z),
        0.0f,
        focal_y / t.z,
        -(focal_y * t.y) / (t.z * t.z),
        0,
        0,
        0);

    mat3 W = mat3(
        viewmatrix[0][0],
        viewmatrix[1][0],
        viewmatrix[2][0],
        viewmatrix[0][1],
        viewmatrix[1][1],
        viewmatrix[2][1],
        viewmatrix[0][2],
        viewmatrix[1][2],
        viewmatrix[2][2]);

    mat3 T = W * J;

    mat3 Vrk = mat3(
        cov3D[0],
        cov3D[1],
        cov3D[2],
        cov3D[1],
        cov3D[3],
        cov3D[4],
        cov3D[2],
        cov3D[4],
        cov3D[5]);

    mat3 cov = transpose(T) * transpose(Vrk) * T;

    // Apply low-pass filter: every Gaussian should be at least
    // one pixel wide/high. Discard 3rd row and column.
    cov[0][0] += 0.3f;
    cov[1][1] += 0.3f;
    return vec3(float(cov[0][0]), float(cov[0][1]), float(cov[1][1]));
}

vec3 computeColorFromSH(int idx, int deg, int max_coeffs, uint64_t means_addr, vec3 campos, uint64_t shs_addr, uint64_t clamped_addr)
{
    _xyz xyz = _xyz(means_addr);
    _feature shs = _feature(shs_addr);
    _clamped clamped = _clamped(clamped_addr);
    // The implementation is loosely based on code for
    // "Differentiable Point-Based Radiance Fields for
    // Efficient View Synthesis" by Zhang et al. (2022)

    vec3 pos = xyz.xyz[idx];
    vec3 dir = pos - campos;
    dir = dir / length(dir);

    // vec3[16] sh = ((glm::vec3*)shs) + idx * max_coeffs;
    // vec3[16] sh = {}
    int from_index = idx * max_coeffs;
    vec3 result = SH_C0 * shs.feature[from_index + 0];

    if (deg > 0) {
        float x = dir.x;
        float y = dir.y;
        float z = dir.z;
        result = result - SH_C1 * y * shs.feature[from_index + 1] + SH_C1 * z * shs.feature[from_index + 2] - SH_C1 * x * shs.feature[from_index + 3];

        if (deg > 1) {
            float xx = x * x, yy = y * y, zz = z * z;
            float xy = x * y, yz = y * z, xz = x * z;
            result = result +
                SH_C2[0] * xy * shs.feature[from_index + 4] +
                SH_C2[1] * yz * shs.feature[from_index + 5] +
                SH_C2[2] * (2.0f * zz - xx - yy) * shs.feature[from_index + 6] +
                SH_C2[3] * xz * shs.feature[from_index + 7] +
                SH_C2[4] * (xx - yy) * shs.feature[from_index + 8];

            if (deg > 2) {
                result = result +
                    SH_C3[0] * y * (3.0f * xx - yy) * shs.feature[from_index + 9] +
                    SH_C3[1] * xy * z * shs.feature[from_index + 10] +
                    SH_C3[2] * y * (4.0f * zz - xx - yy) * shs.feature[from_index + 11] +
                    SH_C3[3] * z * (2.0f * zz - 3.0f * xx - 3.0f * yy) * shs.feature[from_index + 12] +
                    SH_C3[4] * x * (4.0f * zz - xx - yy) * shs.feature[from_index + 13] +
                    SH_C3[5] * z * (xx - yy) * shs.feature[from_index + 14] +
                    SH_C3[6] * x * (xx - 3.0f * yy) * shs.feature[from_index + 15];
            }
        }
    }
    result += 0.5f;

    // RGB colors are clamped to positive values. If values are
    // clamped, we need to keep track of this for the backward pass.
    clamped.clamped[3 * idx + 0] = int(result.x < 0);
    clamped.clamped[3 * idx + 1] = int(result.y < 0);
    clamped.clamped[3 * idx + 2] = int(result.z < 0);
    return max(result, 0.0f);
}

bool test(uint64_t addr)
{

    _opacity opacity = _opacity(opacity_address);
    float rr = opacity.opacity[0];
    debugPrintfEXT("messagemochenggg %f \n ", rr);
    return true;
}