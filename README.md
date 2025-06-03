<!-- Copyright © 2024-2025 Arm Limited.
SPDX-License-Identifier: MIT -->

![Arm ASR Logo](res/logo.png)

# Arm® Accuracy Super Resolution™ (Arm ASR) Generic Library

## Table of contents

- [Introduction](#introduction)
- [Integration steps](#integration-steps)
    - [Quick integration](#quick-integration)
    - [Tight integration](#tight-integration)
- [Integration guidelines](#integration-guidelines)
    - [Quality presets](#quality-presets)
    - [Performance](#performance)
    - [Shader variants and Extensions](#shader-variants-and-extensions)
    - [Input resources](#input-resources)
    - [Providing motion vectors](#providing-motion-vectors)
    - [Reactive mask](#reactive-mask)
    - [Automatically generating reactivity](#automatically-generating-reactivity)
	- [Modular backend](#modular-backend)
    - [Camera jitter](#camera-jitter)
	- [Camera jump cuts](#camera-jump-cuts)
    - [Mipmap biasing](#mipmap-biasing)
    - [Frame Time Delta Input](#frame-time-delta-input)
    - [HDR support](#hdr-support)
    - [API Debug Checker](#debug-checker)
- [Extended ffx_shader_compiler](#extended-ffx_shader_compiler)
- [Generate prebuilt shaders](#generate-prebuilt-shaders)
- [Targeting OpenGL® ES 3.2](#targeting-opengl-es-32)
- [License](#license)
- [Trademarks and Copyrights](#trademarks-and-copyrights)
- [Contact](#contact)
- [Security Issues](#security-issues)

## Introduction

## Introduction

**Arm® Accuracy Super Resolution™** is a mobile-optimized temporal upscaling technique derived from AMD's [FidelityFX™ Super Resolution 2 v2.2.2](https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/blob/main/docs/techniques/super-resolution-temporal.md). Arm ASR includes multiple optimizations on top of the original **FidelityFX™ Super Resolution 2** to make the technique suited for the more resource-constrained environment of mobile gaming. The team would like to express their gratitude to AMD and the engineers behind the original technique for their work and releasing it publicly to the world.

Arm ASR uses temporal accumulation to reconstruct high-resolution images while maintaining fine geometric and texture details compared to native rendering. It can enable "practical performance" for costly render operations, such as hardware ray tracing.

## Integration steps

### Quick integration

To quickly integrate Arm ASR, which means the built-in standalone backend is used, you could follow the steps below:

1. Copy **Arm_ASR** directory into your project, and add **Arm_ASR/src/backends/shared/blob_accessors/prebuilt_shaders** in include path if you want to use prebuilt shaders.

2. Include the [`ffxm_fsr2.h`](./include/host/ffxm_fsr2.h#L1) and [`ffxm_vk.h`](./include/host/backends/vk/ffxm_vk.h#L1) header file in your codebase where you wish to interact with the technique.

3. Create a Vulkan backend. Allocate Vulkan scratch buffer of the size returned by [`ffxmGetScratchMemorySizeVK`](./include/host/backends/vk/ffxm_vk.h#L65). Create `FfxmDevice` via [`ffxmGetDeviceVK`](./include/host/backends/vk/ffxm_vk.h#L65). Create `FfxmInterface` by calling [`ffxmGetInterfaceVK`](./include/host/backends/vk/ffxm_vk.h#L99).

4. Create a context by calling [`ffxmFsr2ContextCreate`](./include/host/ffxm_fsr2.h#L296). The parameters structure should be filled out matching the configuration of your application. See [Integration guidelines](#integration-guidelines) for more details.

5. Each frame call [`ffxmFsr2ContextDispatch`](./include/host/ffxm_fsr2.h#L337) to record/execute the technique's workloads. The parameters structure should be filled out matching the configuration of your application. See [Integration guidelines](#integration-guidelines) for more details.

6. When your application is terminating (or you wish to destroy the context for another reason) you should call [`ffxmFsr2ContextDestroy`](./include/host/ffxm_fsr2.h#L360). The GPU should be idle before calling this function.

7. Sub-pixel jittering should be applied to your application's projection matrix. This should be done when performing the main rendering of your application. You should use the [`ffxmFsr2GetJitterOffset`](./include/host/ffxm_fsr2.h#L504) function to compute the precise jitter offsets. See [Camera jitter](#camera-jitter) section for more details.

8. A global mip bias should be applied when texturing. See [Mipmap biasing](#mipmap-biasing) section for more details.

9. For the best upscaling quality it is strongly advised that you populate the [Reactive mask](#reactive-mask) according to our guidelines. You can also use [`ffxmFsr2ContextGenerateReactiveMask`](./include/host/ffxm_fsr2.h#L348) as a starting point.

10. Finally link the two built libraries (**Arm_ASR_api** & **Arm_ASR_backend**).

### Tight integration

If you wish to use your own backend/renderer, a tight integration with your engine is required. For this a similar process to the "quick integration" described above will be done with the main difference being filling the [`FfxmInterface`](./include/host/ffxm_interface.h#L438) with functions implemented on your end:

1. Include [`ffxm_interface.h`](./include/host/ffxm_interface.h#L1) header file in your codebase.

2. Implement your own functions (assume the names are `xxxGetInterfacexxx`, `xxxGetScratchMemorySizexxx`) and callbacks in [`FfxmInterface`](./include/host/ffxm_interface.h#L438) to link Arm ASR with the engine's renderer.

3. Create your own backend by calling `xxxGetInterfacexxx`. A scratch buffer should be allocated of the size returned by calling `xxxGetScratchMemorySizexxx` and the pointer to that buffer passed to `xxxGetInterfacexxx`.

4. The rest are the same as quick integration start from creating a Arm ASR context (starting at step 4) except only linking **Arm_ASR_api** is enough.

In this approach the shaders are expected to be built by the engine. Arm ASR's shaders have been micro-optimized to use explicit 16b types so it is advisable that the shaders are built using such types (i.e min16float in hlsl or float16_t in glsl). For this you should define the symbols `#define FFXM_HLSL_6_2 1` and `#define FFXM_HALF 1` (FFXM_HALF is already defined in the provided shader sources) enabled with a value of `1`.

In an HLSL-based workflow using DirectX Shader Compiler to cross-compile to SPIRV, the following flags should be used when building:
```
-fspv-target-env=vulkan1.1spirv1.4 -enable-16bit-types
```
and the extension **VK_KHR_shader_float16_int8** used at runtime.

## Integration guidelines

### Quality presets

Arm ASR API provides a number of shader quality presets, that can be used to select a more efficient version of the technique that applies some reasonable quality/performance tradeoffs:

**Quality**: This is in essence an extremely optimized version of Fsr2 maintaining the same image quality as the original technique.

**Balanced**: This preset gives a significant improvement in both bandwidth savings and performance uplift while maintaining close image quality to the 'quality' preset.

**Performance**: A more aggressive preset that will give you the highest performance with some quality sacrifices.

When creating context, a [`FfxmFsr2ShaderQualityMode`](./include/host/ffxm_fsr2.h#L119) needs to be provided as part of the input settings in [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181).

### Upscaling ratios

To improve the flexbility when working with the technique, we decided to leave the door open for having separately the shader quality presets and the upscaling ratio. So, developers are free to choose any combination of **FfxmFsr2ShaderQualityMode** and **FfxmFsr2UpscalingRatio** based on their needs to fine tune the quality/performance of their applications.

We provide a couple of utilities to get the corresponding src resolution the frame should be using for rendering pre-upscaling based on the desired upscaling ratio [`FfxmFsr2UpscalingRatio`](./include/host/ffxm_fsr2.h#L128).

``` CPP
float ffxmFsr2GetUpscaleRatioFactor(FfxmFsr2UpscalingRatio upscalingRatio)
FfxErrorCode ffxmFsr2GetRenderResolutionFromUpscalingRatio(
    uint32_t* renderWidth,
    uint32_t* renderHeight,
    uint32_t displayWidth,
    uint32_t displayHeight,
    FfxmFsr2UpscalingRatio upscalingRatio)
```

### Performance
Depending on your target hardware and operating configuration Arm ASR will operate at different performance levels.


| Target resolution | Quality | Upscaling Ratio | Immortalis-G715 | Immortalis-G720 |
|-------------------|--------------------|-----------|-----------------|-----------------|
| 2800x1260 | Quality     | 1.5x | <span style="color: #FF0000;">6.5 ms</span> | <span style="color: #feff00;">4.1 ms</span> |
|           |             | 1.7x | <span style="color: #FF0000;">6.3 ms</span> | <span style="color: #8fff00;">3.8 ms</span> |
|           |             | 2x   | <span style="color: #FF0000;">6.1 ms</span> | <span style="color: #8fff00;">3.3 ms</span> |
|           | Balanced    | 1.5x | <span style="color: #FFAC00;">5.8 ms</span> | <span style="color: #8fff00;">3.3 ms</span> |
|           |             | 1.7x | <span style="color: #FFAC00;">5.4 ms</span> | <span style="color: #8fff00;">3.1 ms</span> |
|           |             | 2x   | <span style="color: #feff00;">4.7 ms</span> | <span style="color: #55ff00;">2.8 ms</span> |
|           | Performance | 1.5x | <span style="color: #FFAC00;">5.4 ms</span> | <span style="color: #8fff00;">3.2 ms</span> |
|           |             | 1.7x | <span style="color: #FFAC00;">5.3 ms</span> | <span style="color: #55ff00;">2.9 ms</span> |
|           |             | 2x   | <span style="color: #feff00;">4.6 ms</span> | <span style="color: #55ff00;">2.5 ms</span> |
| 2400x1080 | Quality     | 1.5x | <span style="color: #FFAC00;">5.3 ms</span> | <span style="color: #55ff00;">2.9 ms</span> |
|           |             | 1.7x | <span style="color: #feff00;">4.8 ms</span> | <span style="color: #55ff00;">2.7 ms</span> |
|           |             | 2x   | <span style="color: #feff00;">4.3 ms</span> | <span style="color: #55ff00;">2.5 ms</span> |
|           | Balanced    | 1.5x | <span style="color: #feff00;">4.2 ms</span> | <span style="color: #55ff00;">2.5 ms</span> |
|           |             | 1.7x | <span style="color: #feff00;">4.0 ms</span> | <span style="color: #55ff00;">2.3 ms</span> |
|           |             | 2x   | <span style="color: #8fff00;">3.6 ms</span> | <span style="color: #55ff00;">2.2 ms</span> |
|           | Performance | 1.5x | <span style="color: #feff00;">4.1 ms</span> | <span style="color: #55ff00;">2.4 ms</span> |
|           |             | 1.7x | <span style="color: #8fff00;">3.7 ms</span> | <span style="color: #55ff00;">2.1 ms</span> |
|           |             | 2x   | <span style="color: #8fff00;">3.6 ms</span> | <span style="color: #55ff00;">2 ms  </span> |

### Shader variants and Extensions

**Unless you are using the prebuilt shaders with the standalone VK backend**, when doing the integration of the Arm ASR shaders there are some defines you need to be aware of:
- **FFXM_GPU**. Needs to be defined globally when including the shader headers.
- **FFXM_HLSL**. If defined, the logic will fallback to use the **HLSL** specific syntax (i.e types, resource declaration ...).
- **FFXM_GLSL**. If defined, the logic will fallback to use the **GLSL** specific syntax (i.e types, resource declaration ...).

The following table shows the list of the different shader mutators that can be used. All of them must be defined either with a value of 0 or 1. Which shader variant to use is guided internally by **getPipelinePermutationFlags(...)** based on things like user's flags and shader quality.

| Define | Description |
| -------- | ------- |
| FFXM_FSR2_OPTION_HDR_COLOR_INPUT | If **1**, will assume that the input color is in linear RGB. |
| FFXM_FSR2_OPTION_LOW_RESOLUTION_MOTION_VECTORS | If **1**, will assume the input motion vectors texture is in low resolution |
| FFXM_FSR2_OPTION_JITTERED_MOTION_VECTORS | If **1**, will assume jittered motion vectors using the same jitter offsets as the input color and depth. |
| FFXM_FSR2_OPTION_INVERTED_DEPTH | If **1**, it will assume the input depth containing reversed depth values (far == 0.0f) |
| FFXM_FSR2_OPTION_APPLY_SHARPENING | If **1**, informs the shaders that RCAS (sharpening) pass will be used. |
| FFXM_FSR2_OPTION_SHADER_OPT_BALANCED | If **1**, enables a batch of optimizations when the **Balanced** quality preset is selected. |
| FFXM_FSR2_OPTION_SHADER_OPT_PERFORMANCE | If **1**,  enables a batch of optimizations when the **Performance** quality preset is selected. When this is enabled then **FFXM_FSR2_OPTION_SHADER_OPT_BALANCED** will be enabled too. |
| FFXM_FSR2_OPTION_SHADER_OPT_ULTRA_PERFORMANCE | If **1**,  enables a batch of optimizations when the **Ultra Performance** quality preset is selected.

Lastly, when using an HLSL-based workflow, we also have the **FFXM_HLSL_6_2** global define. If defined with a value of **1**, this will enable the use of explicit 16 bit types instead of relying in **half** (RelaxedPrecision). The **VK_KHR_shader_float16_int8** extension is required on Vulkan.

### Input resources
Arm ASR is a temporal algorithm, and therefore requires access to data from both the current and previous frame. The following table enumerates all external inputs required by it.

The resolution column indicates if the data should be at 'rendered' resolution or 'presentation' resolution. 'Rendered' resolution indicates that the resource should match the resolution at which the application is performing its rendering. Conversely, 'presentation' indicates that the resolution of the target should match that which is to be presented to the user. All resources are from the current rendered frame, for Vulkan applications all input resources should be transitioned to [`VK_ACCESS_SHADER_READ_BIT`](https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html) respectively before calling [`ffxmFsr2ContextDispatch`](./include/host/ffxm_fsr2.h#L337).

| Name            | Resolution                   |  Format                            | Type      | Notes                                          |
| ----------------|------------------------------|------------------------------------|-----------|------------------------------------------------|
| Color buffer    | Render                       | `APPLICATION SPECIFIED`            | Texture   | The render resolution color buffer for the current frame provided by the application. If the contents of the color buffer are in high dynamic range (HDR), then the [`FFXM_FSR2_ENABLE_HIGH_DYNAMIC_RANGE`](./include/host/ffxm_fsr2.h#L140) flag should be set in  the [`flags`](./include/host/ffxm_fsr2.h#L183) field of the [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181) structure. |
| Depth buffer    | Render                       | `APPLICATION SPECIFIED (1x FLOAT)` | Texture   | The render resolution depth buffer for the current frame provided by the application. The data should be provided as a single floating point value, the precision of which is under the application's control. The configuration of the depth should be communicated to Arm ASR via the [`flags`](./include/host/ffxm_fsr2.h#L183) field of the [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181) structure when creating the [`FfxmFsr2Context`](./include/host/ffxm_fsr2.h#L247). You should set the [`FFXM_FSR2_ENABLE_DEPTH_INVERTED`](./include/host/ffxm_fsr2.h#L145) flag if your depth buffer is inverted (that is [1..0] range), and you should set the [`FFXM_FSR2_ENABLE_DEPTH_INFINITE`](./include/host/ffxm_fsr2.h#L146) flag if your depth buffer has an infinite far plane. If the application provides the depth buffer in `D32S8` format, then it will ignore the stencil component of the buffer, and create an `R32_FLOAT` resource to address the depth buffer. |
| Motion vectors  | Render or presentation       | `APPLICATION SPECIFIED (2x FLOAT)` | Texture   | The 2D motion vectors for the current frame provided by the application in **[<-width, -height> ... <width, height>]** range. If your application renders motion vectors with a different range, you may use the [`motionVectorScale`](./include/host/ffxm_fsr2.h#L205) field of the [`FfxmFsr2DispatchDescription`](./include/host/ffxm_fsr2.h#L194) structure to adjust them to match the expected range for Arm ASR. Internally, Arm ASR uses 16-bit quantities to represent motion vectors in many cases, which means that while motion vectors with greater precision can be provided, Arm ASR will not benefit from the increased precision. The resolution of the motion vector buffer should be equal to the render resolution, unless the [`FFXM_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS`](./include/host/ffxm_fsr2.h#L143) flag is set in the [`flags`](./include/host/ffxm_fsr2.h#L183) field of the [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181) structure when creating the [`FfxmFsr2Context`](./include/host/ffxm_fsr2.h#L246), in which case it should be equal to the presentation resolution. |
| Reactive mask   | Render                       | `R8_UNORM`                         | Texture   | As some areas of a rendered image do not leave a footprint in the depth buffer or include motion vectors, Arm ASR provides support for a reactive mask texture which can be used to indicate to the technique where such areas are. Good examples of these are particles, or alpha-blended objects which do not write depth or motion vectors. If this resource is not set, then Arm ASR's shading change detection logic will handle these cases as best it can, but for optimal results, this resource should be set. For more information on the reactive mask please refer to the [Reactive mask](#reactive-mask) section.  |
| Exposure        | 1x1                          | `R32_FLOAT/ R16_FLOAT`                        | Texture   | A 1x1 texture containing the exposure value computed for the current frame. This resource is optional, and may be omitted if the [`FFXM_FSR2_ENABLE_AUTO_EXPOSURE`](./include/host/ffxm_fsr2.h#L147) flag is set in the [`flags`](./include/host/ffxm_fsr2.h#L183) field of the [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181) structure when creating the [`FfxmFsr2Context`](./include/host/ffxm_fsr2.h#L246).  |

All inputs that are provided at Render Resolution, except for motion vectors, should be rendered with jitter. By default, Motion vectors are expected to be unjittered unless the [`FFXM_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION`](./include/host/ffxm_fsr2.h#L144) flag is present.

### Providing motion vectors

#### Space
A key part of a temporal algorithm (be it antialiasing or upscaling) is the provision of motion vectors. Arm ASR accepts motion vectors in 2D which encode the motion from a pixel in the current frame to the position of that same pixel in the previous frame. It expects that motion vectors are provided by the application in [**<-width, -height>**..**<width, height>**] range; this matches Screen-Space. For example, a motion vector for a pixel in the upper-left corner of the screen with a value of <width, height> would represent a motion that traversed the full width and height of the input surfaces, originating from the bottom-right corner.

If your application computes motion vectors in another space - for example normalized device coordinate space - then you may use the [`motionVectorScale`](./include/host/ffxm_fsr2.h#L205) field of the [`FfxmFsr2DispatchDescription`](./include/host/ffxm_fsr2.h#L194) structure to instruct the technique to adjust them to match the expected range. The code examples below illustrate how motion vectors may be scaled to screen space. The example HLSL and C++ code below illustrates how NDC-space motion vectors can be scaled using the Arm ASR host API.

```HLSL
// GPU: Example of application NDC motion vector computation
float2 motionVector = (previousPosition.xy / previousPosition.w) - (currentPosition.xy / currentPosition.w);

// CPU: Matching Arm ASR motionVectorScale configuration
dispatchParameters.motionVectorScale.x = (float)renderWidth;
dispatchParameters.motionVectorScale.y = (float)renderHeight;
```

#### Precision & resolution

Internally, Arm ASR uses 16bit quantities to represent motion vectors in many cases, which means that while motion vectors with greater precision can be provided, it will not currently benefit from the increased precision. The resolution of the motion vector buffer should be equal to the render resolution, unless the [`FFXM_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS`](./include/host/ffxm_fsr2.h#L143) flag is set in the [`flags`](./include/host/ffxm_fsr2.h#L183) field of the [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181) structure when creating the [`FfxmFsr2Context`](./include/host/ffxm_fsr2.h#L246), in which case it should be equal to the presentation resolution.

#### Coverage

Arm ASR will perform better quality upscaling when more objects provide their motion vectors. It is therefore advised that all opaque, alpha-tested and alpha-blended objects should write their motion vectors for all covered pixels. If vertex shader effects are applied - such as scrolling UVs - these calculations should also be factored into the calculation of motion for the best results. For alpha-blended objects it is also strongly advised that the alpha value of each covered pixel is stored to the corresponding pixel in the [reactive mask](#reactive-mask). This will allow the technique to perform better handling of alpha-blended objects during upscaling. The reactive mask is especially important for alpha-blended objects where writing motion vectors might be prohibitive, such as particles.

### Reactive mask

In the context of Arm ASR, the term "reactivity" means how much influence the samples rendered for the current frame have over the production of the final upscaled image. Typically, samples rendered for the current frame contribute a relatively modest amount to the result computed by the algorithm; however, there are exceptions. As there is no good way to determine from either color, depth or motion vectors which pixels have been rendered using alpha blending, Arm ASR performs best when applications explicitly mark such areas.

Therefore, it is strongly encouraged that applications provide a reactive mask as an input. The reactive mask guides Arm ASR on where it should reduce its reliance on historical information when compositing the current pixel, and instead allow the current frame's samples to contribute more to the final result. The reactive mask allows the application to provide a value from [0.0..1.0] where 0.0 indicates that the pixel is not at all reactive (and should use the default composition strategy), and a value of 1.0 indicates the pixel should be fully reactive. This is a floating point range and can be tailored to different situations.

While there are other applications for the reactive mask, the primary application for the reactive mask is producing better results of upscaling images which include alpha-blended objects. A good proxy for reactiveness is actually the alpha value used when compositing an alpha-blended object into the scene, therefore, applications should write `alpha` to the reactive mask. It should be noted that it is unlikely that a reactive value of close to 1 will ever produce good results. Therefore, we recommend clamping the maximum reactive value to around 0.9.

If a reactive mask is not provided (by setting the [`reactive`](./include/host/ffxm_fsr2.h#L201) field of [`FfxmFsr2DispatchDescription`](./include/host/ffxm_fsr2.h#L194) to `NULL`) then an internally generated 1x1 texture with a cleared reactive value will be used.

### Automatically generating reactivity

To help applications generate the reactive mask, we provide an optional utility pass. Under the hood, the API launches a fragment shader which computes these values for each pixel using a luminance-based heuristic.

Applications wishing to do this can call the [`ffxmFsr2ContextGenerateReactiveMask`](./include/host/ffxm_fsr2.h#L348) function and should pass two versions of the color buffer, one containing opaque only geometry, and the other containing both opaque and alpha-blended objects.

### Exposure
Arm ASR provides two values which control the exposure used when performing upscaling. They are as follows:

1. **Pre-exposure**: a value by which we divide the input signal to get back to the original signal produced by the game before any packing into lower precision render targets.
2. **Exposure**: a value which is multiplied against the result of the pre-exposed color value.

The exposure value should match that which the application uses during any subsequent tonemapping passes performed by the application. This means Arm ASR will operate consistently with what is likely to be visible in the final tonemapped image.

> In various stages of the algorithm, the technique will compute its own exposure value for internal use. It is worth noting that all outputs will have this internal tonemapping reversed before the final output is written. Meaning that Arm ASR returns results in the same domain as the original input signal.

Poorly selected exposure values can have a drastic impact on the final quality of Arm ASR's upscaling. Therefore, it is recommended that [`FFXM_FSR2_ENABLE_AUTO_EXPOSURE`](./include/host/ffxm_fsr2.h#L147) is used by the application, unless there is a particular reason not to. When [`FFXM_FSR2_ENABLE_AUTO_EXPOSURE`](./include/host/ffxm_fsr2.h#L147) is set in the [`flags`](./include/host/ffxm_fsr2.h#L183) field of the [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181) structure, the exposure calculation in [`ComputeAutoExposureFromLavg`](./include/gpu/fsr2/ffxm_fsr2_common.h#L412) is used to compute the exposure value, which matches the exposure response of ISO 100 film stock.

### Modular backend

The design of the Arm ASR API means that the core implementation of the algorithm is unaware upon which rendering API it sits. Instead, it calls functions provided to it through an interface, allowing different backends to be used with the technique. Applications which have their own rendering abstractions can implement their own backend, taking control of all aspects of Arm ASR's underlying function, including memory management, resource creation, shader compilation, shader resource bindings, and the submission of the workloads to the graphics device.

Out of the box, the API will compile into multiple libraries following the separation already outlined between the core API and the backends. This means if you wish to use the backends provided, you should link both the core API lib (Arm_ASR_api) as well the backend (Arm_ASR_backend) matching your requirements.

Arm ASR only provides a built-in Vulkan backend as it targets Vulkan mobile apps.

### Camera jitter
Arm ASR relies on the application to apply sub-pixel jittering while rendering - this is typically included in the projection matrix of the camera. To make the application of camera jitter simple, the API provides a small set of utility function which computes the sub-pixel jitter offset for a particular frame within a sequence of separate jitter offsets.

``` CPP
int32_t ffxmFsr2GetJitterPhaseCount(int32_t renderWidth, int32_t displayWidth);
FfxErrorCode ffxmFsr2GetJitterOffset(float* outX, float* outY, int32_t jitterPhase, int32_t sequenceLength);
```

Internally, these functions implement a **Halton[2,3]** sequence. The goal of the Halton sequence is to provide spatially separated points, which cover the available space.

It is important to understand that the values returned from the [`ffxmFsr2GetJitterOffset`](./include/host/ffxm_fsr2.h#L504) are in unit pixel space, and in order to composite this correctly into a projection matrix we must convert them into projection offsets. The code listing below shows how to correctly composite the sub-pixel jitter offset value into a projection matrix.

``` CPP
const int32_t jitterPhaseCount = ffxmFsr2GetJitterPhaseCount(renderWidth, displayWidth);

float jitterX = 0;
float jitterY = 0;
ffxmFsr2GetJitterOffset(&jitterX, &jitterY, index, jitterPhaseCount);

// Calculate the jittered projection matrix.
const float jitterX = 2.0f * jitterX / (float)renderWidth;
const float jitterY = -2.0f * jitterY / (float)renderHeight;
const Matrix4 jitterTranslationMatrix = translateMatrix(Matrix3::identity, Vector3(jitterX, jitterY, 0));
const Matrix4 jitteredProjectionMatrix = jitterTranslationMatrix * projectionMatrix;
```

Jitter should be applied to *all* rendering. This includes opaque, alpha transparent, and raytraced objects. For rasterized objects, the sub-pixel jittering values calculated by the [`ffxmFsr2GetJitterOffset`](./include/host/ffxm_fsr2.h#L504) function can be applied to the camera projection matrix which is ultimately used to perform transformations during vertex shading. For raytraced rendering, the sub-pixel jitter should be applied to the ray's origin - often the camera's position.

Whether you elect to use the recommended [`ffxmFsr2GetJitterOffset`](./include/host/ffxm_fsr2.h#L504) function or your own sequence generator, you must set the [`jitterOffset`](./include/host/ffxm_fsr2.h#L204) field of the [`FfxmFsr2DispatchDescription`](./include/host/ffxm_fsr2.h#L194) structure to inform the algorithm of the jitter offset that has been applied in order to render each frame. Moreover, if not using the recommended [`ffxmFsr2GetJitterOffset`](./include/host/ffxm_fsr2.h#L504) function, care should be taken that your jitter sequence never generates a null vector; that is value of 0 in both the X and Y dimensions.

### Camera jump cuts

Most applications with real-time rendering have a large degree of temporal consistency between any two consecutive frames. However, there are cases where a change to a camera's transformation might cause an abrupt change in what is rendered. In such cases, ASR is unlikely to be able to reuse any data it has accumulated from previous frames, and should clear this data such to exclude it from consideration in the compositing process. In order to indicate that a jump cut has occurred with the camera you should set the [`reset`](./include/host/ffxm_fsr2.h#L211) field of the [`FfxmFsr2DispatchDescription`](./include/host/ffxm_fsr2.h#L194) structure to `true` for the first frame of the discontinuous camera transformation.

Rendering performance may be slightly less than typical frame-to-frame operation when using the reset flag, as Arm ASR will clear some additional internal resources.

### Mipmap biasing

Applying a negative mipmap biasing will typically generate an upscaled image with better texture detail. We recommend applying the following formula to your Mipmap bias:

``` CPP
mipBias = log2(renderResolution/displayResolution) - 1.0;
```

### Frame Time Delta Input

The API requires [`frameTimeDelta`](./include/host/ffxm_fsr2.h#L209) be provided by the application through the [`FfxmFsr2DispatchDescription`](./include/host/ffxm_fsr2.h#L194) structure. This value is in __milliseconds__: if running at 60fps, the value passed should be around __16.6f__.

The value is used within the temporal component of the auto-exposure feature. This allows for tuning of the history accumulation for quality purposes.

### HDR support

High dynamic range images are supported. To enable this, you should set the [`FFXM_FSR2_ENABLE_HIGH_DYNAMIC_RANGE`](./include/host/ffxm_fsr2.h#L142) bit in the [`flags`](./include/host/ffxm_fsr2.h#L183) field of the [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181) structure. When using this flag, the input color image should be provided in linear RGB color space.

### Debug Checker

The context description structure can be provided with a callback function for passing textual warnings from the runtime to the underlying application. The `fpMessage` member of the description is of type `FfxmFsr2Message` which is a function pointer for passing string messages of various types. Assigning this variable to a suitable function, and passing the [`FFXM_FSR2_ENABLE_DEBUG_CHECKING`](./include/host/ffxm_fsr2.h#L150) flag within the flags member of [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181) will enable the feature. It is recommended this is enabled only in debug development builds.

## Extended ffx_shader_compiler

Most of the workloads in the upscalers have been converted to Fragment Shaders. Since the workflow using the standalone VK backend relies in reflection data generated with [`AMD's Shader Compiler`](https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/blob/main/docs/tools/ffx-sc.md), it become necessary to do an ad-hoc extension of the tool to provide reflection data for the RenderTargets so resources could be resolved automatically in the backend. Users might want to evolve the algorithm potentially changing the RenderTargets in the process, to do so we provide a diff file with the changes that were applied locally [`ffx_shader_compiler`](./tools/ffx_shader_compiler.diff) for the latest version of the technique.

## Generate prebuilt shaders

We provide a helper script to generate prebuilt shaders which are used for standalone backend, you can just run [`generate_prebuilt_shaders.py`](./tools/generate_prebuilt_shaders.py), and output path is **src/backends/shared/blob_accessors/prebuilt_shaders**.

## Targeting OpenGL ES 3.2

Running Arm ASR on GLES is possible when using the [tight integration](#tight-integration) approach. In this scenario, the user will have to apply two minor changes on their side:
1. When creating the context, in [`flags`](./include/host/ffxm_fsr2.h#L183) field of the [`FfxmFsr2ContextDescription`](./include/host/ffxm_fsr2.h#L181) the user will have to specify the flag [`FFXM_FSR2_OPENGL_ES_3_2`](./include/host/ffxm_fsr2.h#L149). This will trigger some minor changes internally so Arm ASR adapts to a GLES friendly approach.
2. The [`permutationOptions`](./include/host/ffxm_interface.h#L307) provided when creating the pipelines will now include the new permutation option [`FSR2_SHADER_PERMUTATION_PLATFORM_GLES_3_2`](./src/components/fsr2/ffxm_fsr2_private.h#L47). This is a hint to the user that they will need to use the shader variants for the technique with the following symbol defined:
```
#define FFXM_SHADER_PLATFORM_GLES_3_2 1
```

## License

Please see the [LICENSE file](./LICENSES/MIT.txt) for details.

## Trademarks and Copyrights

AMD is a trademark of Advanced Micro Devices, Inc.

AMD FidelityFX™ is a trademark of Advanced Micro Devices, Inc.

Arm® is a registered trademark of Arm Limited (or its subsidiaries) in the US and/or elsewhere.

Unreal® is a trademark or registered trademark of Epic Games, Inc. in the United States of America and elsewhere.

Vulkan is a registered trademark and the Vulkan SC logo is a trademark of the Khronos Group Inc.

DirectX, Windows are registered trademarks or trademarks of Microsoft Corporation in the US and other jurisdictions.

OpenGL is a registered trademark and the OpenGL ES logo is a trademark of Hewlett Packard Enterprise used by permission by Khronos.

## Contact

You can reach out to us on the following email address: <arm-asr-support@arm.com>.

## Security Issues

Arm takes security issues seriously please see [Security](SECURITY.md) for more details.
