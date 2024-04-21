#pragma once
#include "GSPassBase.hpp"
namespace MCGS {
using namespace MCRT;
class SortHistogramPass : public GSPassBase {

public:
    SortHistogramPass(std::shared_ptr<Uniform_Stuff<uint64_t>> _element_in_data,
                      std::shared_ptr<Uniform_Stuff<uint64_t>> _ping_pong_data,
                      std::shared_ptr<Uniform_Stuff<uint32_t>> _histograms_data,
                      std::shared_ptr<Uniform_Stuff<TestAddr>> _test_data);

    void run_pass(vk::CommandBuffer& cmd) override;

    // private:
    void prepare_buffer() override;
    void prepare_shader_pc() override;
    void prepare_descriptorset() override;
    void record_command(vk::CommandBuffer& cmd);
    PushContant_SortHisgram pc;

    const uint num_element = 1000000;

    // std::vector<uint64_t> element_in;
    // std::vector<uint64_t> ping_pong;

    // std::vector<uint32_t> histograms;

    std::shared_ptr<Uniform_Stuff<uint64_t>> element_in_data;
    std::shared_ptr<Uniform_Stuff<uint64_t>> ping_pong_data;

    std::shared_ptr<Uniform_Stuff<uint32_t>> histograms_data;
    std::shared_ptr<Uniform_Stuff<TestAddr>> test_data;
};
}