#pragma once
#include "Rendering/ComputePass.hpp"
#include "shaders/push_contant.h"
#include <memory>
namespace MCGS {
using namespace MCRT;
class GSPassBase {
public:
    GSPassBase() = default;
    void Init();
    // virtual void Execute() = 0;
    virtual  void run_pass(vk::CommandBuffer& cmd) = 0;
    [[nodiscard]]auto get_context(){return content;}
private:
    // virtual void custom_init(std::shared_ptr<Uniform_Stuff<GS_Address>> _address) = 0;
    virtual void prepare_shader_pc() = 0;
    virtual void prepare_descriptorset() = 0;
    virtual void prepare_buffer() = 0;

protected:
    std::shared_ptr<ComputePass> content;
    std::shared_ptr<ShaderModule> shader_module;
    int pc_size;
};

}