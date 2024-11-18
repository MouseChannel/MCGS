#ifdef __cplusplus

#include "glm/glm.hpp"

using vec3 = glm::vec3;

using vec4 = glm::vec4;
using vec2 = glm::vec2;
using uint = uint32_t;
#endif

struct GaussianPoint {
    vec3 pos;
    vec3 scale;
    vec4 rot;
    float sh[48];
    float opacity;
};

struct Instance {
    vec3 ndc_position;
    vec2 scale;
    float theta;
    vec4 color;
}; 

struct IndexedIndirectCommand {
    uint indexCount;
    uint instanceCount;
    uint firstIndex;
    int vertexOffset;
    uint firstInstance;
};