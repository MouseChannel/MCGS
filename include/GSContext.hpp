#include <Rendering/ComputeContext.hpp>
#include <Wrapper/ComputePass/ComputePass.hpp>
#include "shaders/DataStruct.h"
class GSContext : public MCRT::ComputeContext {
public:

    GSContext(std::string path);

    void prepare() override;

private:
    std::unique_ptr<MCRT::ComputePass<>> duplicatePass;
};