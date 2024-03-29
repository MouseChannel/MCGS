#include "GSPassBase.hpp"
#include "shaders/push_contant.h"

namespace MCGS {

void GSPassBase::Init()
{
    prepare_buffer();
    // prepare_shader_pc(std::string shader_path, int pc_size)
    content.reset(new ComputePass);
    // content->set_constants_size(pc_size);
    prepare_shader_pc("/home/mocheng/project/MCGS/include/shaders/sort/single_radixsort.comp.spv", sizeof(PushContant_Sort));
    content->prepare();
    prepare_descriptorset();
    content->prepare_pipeline({ shader_module },
                              { content->get_descriptor_manager()->get_DescriptorSet(DescriptorManager::Compute) },
                              pc_size);
    content->post_prepare();
}

}