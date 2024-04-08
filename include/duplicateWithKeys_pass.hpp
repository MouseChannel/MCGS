#pragma once

#include "GSPassBase.hpp"
namespace MCGS {

class duplicatePass : public GSPassBase {
public:
    duplicatePass(std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_key, std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_value);
    void Execute() override;

private:
    void prepare_buffer() override;
    void prepare_shader_pc() override;
    void prepare_descriptorset() override;
    std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_key;
    std::shared_ptr<Uniform_Stuff<uint64_t>> point_list_value;
    };
}