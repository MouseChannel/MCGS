## MC Gaussian

use Vulkan Compute-Shader to rewrite [3D Gaussian splatting](https://repo-sam.inria.fr/fungraph/3d-gaussian-splatting/) 

### roadmap

- :white_check_mark: Minimal example to verify feasibility(2024-4-10🎊works!)
- :white_large_square: Camera control(in develop current now)
- :white_large_square: **SOTA** GPU sort,by [Nvidia 2022 research](https://research.nvidia.com/publication/2022-06_onesweep-faster-least-significant-digit-radix-sort-gpus)
- :white_large_square: OpenXR support

### showcase

|                         | origin CUDA |  Compute Shader | 
|---------------------|-----------------|-----------------|
| quality   |  ![origincuda](https://github.com/MouseChannel/MCGS/blob/main/showcase/origincuda.png)  | ![output](https://github.com/MouseChannel/MCGS/blob/main/showcase/output.png)  | 
|PSNR |------------------------   |   $+\infty$(same in every single pixel😏)  | 
| FPS      | 227 fps / 4.4ms      | 15 fps / 66.2ms  ( :arrow_heading_down:1504%)    | 

> GPU sort cost 55ms/66.2ms(83%),so It must be optimized!!
 

