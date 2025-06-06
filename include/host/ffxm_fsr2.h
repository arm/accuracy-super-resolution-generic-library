// Copyright  © 2023 Advanced Micro Devices, Inc.
// Copyright  © 2024-2025 Arm Limited.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

// Include the interface for the backend of the FSR2 API.
#include "ffxm_interface.h"

namespace arm
{

/// @defgroup ffxmFsr2 FidelityFX FSR2
/// FidelityFX Super Resolution 2 runtime library
///
/// @ingroup SDKComponents

/// FidelityFX Super Resolution 2 major version.
///
/// @ingroup ffxmFsr2
#define FFXM_FSR2_VERSION_MAJOR      (2)

/// FidelityFX Super Resolution 2 minor version.
///
/// @ingroup ffxmFsr2
#define FFXM_FSR2_VERSION_MINOR      (2)

/// FidelityFX Super Resolution 2 patch version.
///
/// @ingroup ffxmFsr2
#define FFXM_FSR2_VERSION_PATCH      (2)

/// FidelityFX Super Resolution 2 context count
///
/// Defines the number of internal effect contexts required by FSR2
///
/// @ingroup ffxmFsr2
#define FFXM_FSR2_CONTEXT_COUNT   1

/// The size of the context specified in 32bit values.
///
/// @ingroup ffxmFsr2
#define FFXM_FSR2_CONTEXT_SIZE (24576)

#if defined(__cplusplus)
extern "C" {
#endif // #if defined(__cplusplus)

/// An enumeration of all the passes which constitute the FSR2 algorithm.
///
/// FSR2 is implemented as a composite of several compute passes each
/// computing a key part of the final result. Each call to the
/// <c><i>FfxmFsr2ScheduleGpuJobFunc</i></c> callback function will
/// correspond to a single pass included in <c><i>FfxmFsr2Pass</i></c>. For a
/// more comprehensive description of each pass, please refer to the FSR2
/// reference documentation.
///
/// Please note in some cases e.g.: <c><i>FFXM_FSR2_PASS_ACCUMULATE</i></c>
/// and <c><i>FFXM_FSR2_PASS_ACCUMULATE_SHARPEN</i></c> either one pass or the
/// other will be used (they are mutually exclusive). The choice of which will
/// depend on the way the <c><i>FfxmFsr2Context</i></c> is created and the
/// precise contents of <c><i>FfxmFsr2DispatchParamters</i></c> each time a call
/// is made to <c><i>ffxmFsr2ContextDispatch</i></c>.
///
/// @ingroup ffxmFsr2
typedef enum FfxmFsr2Pass
{

    FFXM_FSR2_PASS_DEPTH_CLIP                 = 0,  ///< A pass which performs depth clipping.
    FFXM_FSR2_PASS_RECONSTRUCT_PREVIOUS_DEPTH = 1,  ///< A pass which performs reconstruction of previous frame's depth.
    FFXM_FSR2_PASS_LOCK                       = 2,  ///< A pass which calculates pixel locks.
    FFXM_FSR2_PASS_ACCUMULATE                 = 3,  ///< A pass which performs upscaling.
    FFXM_FSR2_PASS_ACCUMULATE_SHARPEN         = 4,  ///< A pass which performs upscaling when sharpening is used.
    FFXM_FSR2_PASS_RCAS                       = 5,  ///< A pass which performs sharpening.
    FFXM_FSR2_PASS_COMPUTE_LUMINANCE_PYRAMID  = 6,  ///< A pass which generates the luminance mipmap chain for the current frame.
    FFXM_FSR2_PASS_GENERATE_REACTIVE          = 7,  ///< An optional pass to generate a reactive mask.

    FFXM_FSR2_PASS_COUNT  ///< The number of passes performed by FSR2.
} FfxmFsr2Pass;

/// An enumeration of all the quality modes supported by FidelityFX Super
/// Resolution 2 upscaling.
///
/// In order to provide a consistent user experience across multiple
/// applications which implement FSR2. It is strongly recommended that the
/// following preset scaling factors are made available through your
/// application's user interface.
///
/// If your application does not expose the notion of preset scaling factors
/// for upscaling algorithms (perhaps instead implementing a fixed ratio which
/// is immutable) or implementing a more dynamic scaling scheme (such as
/// dynamic resolution scaling), then there is no need to use these presets.
///
/// Please note that <c><i>FFXM_FSR2_QUALITY_MODE_ULTRA_PERFORMANCE</i></c> is
/// an optional mode which may introduce significant quality degradation in the
/// final image. As such it is recommended that you evaluate the final results
/// of using this scaling mode before deciding if you should include it in your
/// application.
///
/// @ingroup ffxmFsr2
typedef enum FfxmFsr2ShaderQualityMode
{

    FFXM_FSR2_SHADER_QUALITY_MODE_QUALITY                       = 1,        ///< Perform upscaling with a shader quality mode of 'Quality'
    FFXM_FSR2_SHADER_QUALITY_MODE_BALANCED                      = 2,        ///< Perform upscaling with a shader quality mode of 'Balanced'
    FFXM_FSR2_SHADER_QUALITY_MODE_PERFORMANCE                   = 3,        ///< Perform upscaling with a shader quality mode of 'Performance'
    FFXM_FSR2_SHADER_QUALITY_MODE_ULTRA_PERFORMANCE             = 4,        ///< Perform upscaling with a shader quality mode of 'Ultra Performance'
} FfxmFsr2ShaderQualityMode;

/// @ingroup ffxmFsr2
typedef enum FfxmFsr2UpscalingRatio
{
	FFXM_FSR2_UPSCALING_RATIO_X1_5	= 1,	///< Perform upscaling with a per-dimension upscaling ratio of 1.5x.
	FFXM_FSR2_UPSCALING_RATIO_X1_7	= 2,	///< Perform upscaling with a per-dimension upscaling ratio of 1.7x.
	FFXM_FSR2_UPSCALING_RATIO_X2	= 3,	///< Perform upscaling with a per-dimension upscaling ratio of 2.0x.

} FfxmFsr2UpscalingRatio;

/// An enumeration of bit flags used when creating a
/// <c><i>FfxmFsr2Context</i></c>. See <c><i>FfxmFsr2ContextDescription</i></c>.
///
/// @ingroup ffxmFsr2
typedef enum FfxmFsr2InitializationFlagBits {

    FFXM_FSR2_ENABLE_HIGH_DYNAMIC_RANGE                  = (1<<0),   ///< A bit indicating if the input color data provided is using a high-dynamic range.
    FFXM_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS   = (1<<1),   ///< A bit indicating if the motion vectors are rendered at display resolution.
    FFXM_FSR2_ENABLE_MOTION_VECTORS_JITTER_CANCELLATION  = (1<<2),   ///< A bit indicating that the motion vectors have the jittering pattern applied to them.
    FFXM_FSR2_ENABLE_DEPTH_INVERTED                      = (1<<3),   ///< A bit indicating that the input depth buffer data provided is inverted [1..0].
    FFXM_FSR2_ENABLE_DEPTH_INFINITE                      = (1<<4),   ///< A bit indicating that the input depth buffer data provided is using an infinite far plane.
    FFXM_FSR2_ENABLE_AUTO_EXPOSURE                       = (1<<5),   ///< A bit indicating if automatic exposure should be applied to input color data.
    FFXM_FSR2_ENABLE_DYNAMIC_RESOLUTION                  = (1<<6),   ///< A bit indicating that the application uses dynamic resolution scaling.
    FFXM_FSR2_ENABLE_TEXTURE1D_USAGE                     = (1<<7),   ///< A bit indicating that the backend should use 1D textures.
    FFXM_FSR2_ENABLE_DEBUG_CHECKING                      = (1<<8),   ///< A bit indicating that the runtime should check some API values and report issues.
	FFXM_FSR2_OPENGL_ES_3_2							     = (1<<9),   ///< A bit indicating that Arm ASR should run in a GLES 3.2 friendly manner
} FfxmFsr2InitializationFlagBits;

/// An enumeration of bit flags used when creating a reactive mask
///
/// @ingroup ffxmFsr2
typedef enum FfxmFsr2AutoReactiveFlagBits {

    FFXM_FSR2_AUTOREACTIVEFLAGS_APPLY_TONEMAP            = 1,
    FFXM_FSR2_AUTOREACTIVEFLAGS_APPLY_INVERSETONEMAP     = 2,
    FFXM_FSR2_AUTOREACTIVEFLAGS_APPLY_THRESHOLD          = 4,
    FFXM_FSR2_AUTOREACTIVEFLAGS_USE_COMPONENTS_MAX       = 8,
} FfxmFsr2AutoReactiveFlagBits;

/// Pass a string message
///
/// Used for debug messages.
///
/// @param [in] type                       The type of message.
/// @param [in] message                    A string message to pass.
///
///
/// @ingroup ffxmFsr2
typedef void(*FfxmFsr2Message)(
    FfxmMsgType type,
    const wchar_t* message);

/// A structure encapsulating the parameters required to initialize FidelityFX
/// Super Resolution 2 upscaling.
///
/// @ingroup ffxmFsr2
typedef struct FfxmFsr2ContextDescription {
	FfxmFsr2ShaderQualityMode	qualityMode;						///< What shader quality mode to use
    uint32_t                    flags;                              ///< A collection of <c><i>FfxmFsr2InitializationFlagBits</i></c>.
    FfxmDimensions2D             maxRenderSize;                      ///< The maximum size that rendering will be performed at.
    FfxmDimensions2D             displaySize;                        ///< The size of the presentation resolution targeted by the upscaling process.
    FfxmInterface                backendInterface;                   ///< A set of pointers to the backend implementation for FidelityFX SDK
    FfxmFsr2Message              fpMessage;                          ///< A pointer to a function that can receive messages from the runtime.
} FfxmFsr2ContextDescription;

/// A structure encapsulating the parameters for dispatching the various passes
/// of FidelityFX Super Resolution 2.
///
/// @ingroup ffxmFsr2
typedef struct FfxmFsr2DispatchDescription {

    FfxmCommandList              commandList;                        ///< The <c><i>FfxmCommandList</i></c> to record FSR2 rendering commands into.
    FfxmResource                 color;                              ///< A <c><i>FfxmResource</i></c> containing the color buffer for the current frame (at render resolution).
    FfxmResource                 depth;                              ///< A <c><i>FfxmResource</i></c> containing 32bit depth values for the current frame (at render resolution).
    FfxmResource                 motionVectors;                      ///< A <c><i>FfxmResource</i></c> containing 2-dimensional motion vectors (at render resolution if <c><i>FFXM_FSR2_ENABLE_DISPLAY_RESOLUTION_MOTION_VECTORS</i></c> is not set).
    FfxmResource                 exposure;                           ///< A optional <c><i>FfxmResource</i></c> containing a 1x1 exposure value.
    FfxmResource                 reactive;                           ///< A optional <c><i>FfxmResource</i></c> containing alpha value of reactive objects in the scene.
    FfxmResource                 transparencyAndComposition;         ///< A optional <c><i>FfxmResource</i></c> containing alpha value of special objects in the scene.
    FfxmResource                 output;                             ///< A <c><i>FfxmResource</i></c> containing the output color buffer for the current frame (at presentation resolution).
    FfxmFloatCoords2D            jitterOffset;                       ///< The subpixel jitter offset applied to the camera.
    FfxmFloatCoords2D            motionVectorScale;                  ///< The scale factor to apply to motion vectors.
    FfxmDimensions2D             renderSize;                         ///< The resolution that was used for rendering the input resources.
    bool                        enableSharpening;                   ///< Enable an additional sharpening pass.
    float                       sharpness;                          ///< The sharpness value between 0 and 1, where 0 is no additional sharpness and 1 is maximum additional sharpness.
    float                       frameTimeDelta;                     ///< The time elapsed since the last frame (expressed in milliseconds).
    float                       preExposure;                        ///< The pre exposure value (must be > 0.0f)
    bool                        reset;                              ///< A boolean value which when set to true, indicates the camera has moved discontinuously.
    float                       cameraNear;                         ///< The distance to the near plane of the camera.
    float                       cameraFar;                          ///< The distance to the far plane of the camera.
    float                       cameraFovAngleVertical;             ///< The camera angle field of view in the vertical direction (expressed in radians).
    float                       viewSpaceToMetersFactor;            ///< The scale factor to convert view space units to meters
} FfxmFsr2DispatchDescription;

/// A structure encapsulating the parameters for automatic generation of a reactive mask
///
/// @ingroup ffxmFsr2
typedef struct FfxmFsr2GenerateReactiveDescription {

    FfxmCommandList              commandList;                        ///< The <c><i>FfxmCommandList</i></c> to record FSR2 rendering commands into.
    FfxmResource                 colorOpaqueOnly;                    ///< A <c><i>FfxmResource</i></c> containing the opaque only color buffer for the current frame (at render resolution).
    FfxmResource                 colorPreUpscale;                    ///< A <c><i>FfxmResource</i></c> containing the opaque+translucent color buffer for the current frame (at render resolution).
    FfxmResource                 outReactive;                        ///< A <c><i>FfxmResource</i></c> containing the surface to generate the reactive mask into.
    FfxmDimensions2D             renderSize;                         ///< The resolution that was used for rendering the input resources.
    float                       scale;                              ///< A value to scale the output
    float                       cutoffThreshold;                    ///< A threshold value to generate a binary reactive mask
    float                       binaryValue;                        ///< A value to set for the binary reactive mask
    uint32_t                    flags;                              ///< Flags to determine how to generate the reactive mask
} FfxmFsr2GenerateReactiveDescription;

/// A structure encapsulating the FidelityFX Super Resolution 2 context.
///
/// This sets up an object which contains all persistent internal data and
/// resources that are required by FSR2.
///
/// The <c><i>FfxmFsr2Context</i></c> object should have a lifetime matching
/// your use of FSR2. Before destroying the FSR2 context care should be taken
/// to ensure the GPU is not accessing the resources created or used by FSR2.
/// It is therefore recommended that the GPU is idle before destroying the
/// FSR2 context.
///
/// @ingroup ffxmFsr2
typedef struct FfxmFsr2Context
{
    uint32_t data[FFXM_FSR2_CONTEXT_SIZE * 2];  ///< An opaque set of <c>uint32_t</c> which contain the data for the context.
} FfxmFsr2Context;


/// Create a FidelityFX Super Resolution 2 context from the parameters
/// programmed to the <c><i>FfxmFsr2CreateParams</i></c> structure.
///
/// The context structure is the main object used to interact with the FSR2
/// API, and is responsible for the management of the internal resources used
/// by the FSR2 algorithm. When this API is called, multiple calls will be
/// made via the pointers contained in the <c><i>callbacks</i></c> structure.
/// These callbacks will attempt to retreive the device capabilities, and
/// create the internal resources, and pipelines required by FSR2's
/// frame-to-frame function. Depending on the precise configuration used when
/// creating the <c><i>FfxmFsr2Context</i></c> a different set of resources and
/// pipelines might be requested via the callback functions.
///
/// The flags included in the <c><i>flags</i></c> field of
/// <c><i>FfxmFsr2Context</i></c> how match the configuration of your
/// application as well as the intended use of FSR2. It is important that these
/// flags are set correctly (as well as a correct programmed
/// <c><i>FfxmFsr2DispatchDescription</i></c>) to ensure correct operation. It is
/// recommended to consult the overview documentation for further details on
/// how FSR2 should be integerated into an application.
///
/// When the <c><i>FfxmFsr2Context</i></c> is created, you should use the
/// <c><i>ffxmFsr2ContextDispatch</i></c> function each frame where FSR2
/// upscaling should be applied. See the documentation of
/// <c><i>ffxmFsr2ContextDispatch</i></c> for more details.
///
/// The <c><i>FfxmFsr2Context</i></c> should be destroyed when use of it is
/// completed, typically when an application is unloaded or FSR2 upscaling is
/// disabled by a user. To destroy the FSR2 context you should call
/// <c><i>ffxmFsr2ContextDestroy</i></c>.
///
/// @param [out] pContext                A pointer to a <c><i>FfxmFsr2Context</i></c> structure to populate.
/// @param [in]  pContextDescription     A pointer to a <c><i>FfxmFsr2ContextDescription</i></c> structure.
///
/// @retval
/// FFXM_OK                              The operation completed successfully.
/// @retval
/// FFXM_ERROR_CODE_NULL_POINTER         The operation failed because either <c><i>context</i></c> or <c><i>contextDescription</i></c> was <c><i>NULL</i></c>.
/// @retval
/// FFXM_ERROR_INCOMPLETE_INTERFACE      The operation failed because the <c><i>FfxmFsr2ContextDescription.callbacks</i></c>  was not fully specified.
/// @retval
/// FFXM_ERROR_BACKEND_API_ERROR         The operation failed because of an error returned from the backend.
///
/// @ingroup ffxmFsr2
FFXM_API FfxmErrorCode ffxmFsr2ContextCreate(FfxmFsr2Context* pContext, const FfxmFsr2ContextDescription* pContextDescription);

/// Dispatch the various passes that constitute FidelityFX Super Resolution 2.
///
/// FSR2 is a composite effect, meaning that it is compromised of multiple
/// constituent passes (implemented as one or more clears, copies and compute
/// dispatches). The <c><i>ffxmFsr2ContextDispatch</i></c> function is the
/// function which (via the use of the functions contained in the
/// <c><i>callbacks</i></c> field of the <c><i>FfxmFsr2Context</i></c>
/// structure) utlimately generates the sequence of graphics API calls required
/// each frame.
///
/// As with the creation of the <c><i>FfxmFsr2Context</i></c> correctly
/// programming the <c><i>FfxmFsr2DispatchDescription</i></c> is key to ensuring
/// the correct operation of FSR2. It is particularly important to ensure that
/// camera jitter is correctly applied to your application's projection matrix
/// (or camera origin for raytraced applications). FSR2 provides the
/// <c><i>ffxmFsr2GetJitterPhaseCount</i></c> and
/// <c><i>ffxmFsr2GetJitterOffset</i></c> entry points to help applications
/// correctly compute the camera jitter. Whatever jitter pattern is used by the
/// application it should be correctly programmed to the
/// <c><i>jitterOffset</i></c> field of the <c><i>dispatchDescription</i></c>
/// structure. For more guidance on camera jitter please consult the
/// documentation for <c><i>ffxmFsr2GetJitterOffset</i></c> as well as the
/// accompanying overview documentation for FSR2.
///
/// @param [in] pContext                 A pointer to a <c><i>FfxmFsr2Context</i></c> structure.
/// @param [in] pDispatchDescription     A pointer to a <c><i>FfxmFsr2DispatchDescription</i></c> structure.
///
/// @retval
/// FFXM_OK                              The operation completed successfully.
/// @retval
/// FFXM_ERROR_CODE_NULL_POINTER         The operation failed because either <c><i>context</i></c> or <c><i>dispatchDescription</i></c> was <c><i>NULL</i></c>.
/// @retval
/// FFXM_ERROR_OUT_OF_RANGE              The operation failed because <c><i>dispatchDescription.renderSize</i></c> was larger than the maximum render resolution.
/// @retval
/// FFXM_ERROR_NULL_DEVICE               The operation failed because the device inside the context was <c><i>NULL</i></c>.
/// @retval
/// FFXM_ERROR_BACKEND_API_ERROR         The operation failed because of an error returned from the backend.
///
/// @ingroup ffxmFsr2
FFXM_API FfxmErrorCode ffxmFsr2ContextDispatch(FfxmFsr2Context* pContext, const FfxmFsr2DispatchDescription* pDispatchDescription);

/// A helper function generate a Reactive mask from an opaque only texure and one containing translucent objects.
///
/// @param [in] pContext                 A pointer to a <c><i>FfxmFsr2Context</i></c> structure.
/// @param [in] pParams                  A pointer to a <c><i>FfxmFsr2GenerateReactiveDescription</i></c> structure
///
/// @retval
/// FFXM_OK                              The operation completed successfully.
///
/// @ingroup ffxmFsr2
FFXM_API FfxmErrorCode ffxmFsr2ContextGenerateReactiveMask(FfxmFsr2Context* pContext, const FfxmFsr2GenerateReactiveDescription* pParams);

/// Destroy the FidelityFX Super Resolution context.
///
/// @param [out] pContext                A pointer to a <c><i>FfxmFsr2Context</i></c> structure to destroy.
///
/// @retval
/// FFXM_OK                              The operation completed successfully.
/// @retval
/// FFXM_ERROR_CODE_NULL_POINTER         The operation failed because either <c><i>context</i></c> was <c><i>NULL</i></c>.
///
/// @ingroup ffxmFsr2
FFXM_API FfxmErrorCode ffxmFsr2ContextDestroy(FfxmFsr2Context* pContext);

/// Get the upscale ratio from the quality mode.
///
/// The following table enumerates the mapping of the quality modes to
/// per-dimension scaling ratios.
///
/// Upscaling ratio                                       | Scale factor
/// ----------------------------------------------------- | -------------
/// <c><i>FFXM_FSR2_UPSCALING_RATIO_X1_5</i></c>          | 1.5x
/// <c><i>FFXM_FSR2_UPSCALING_RATIO_X1_7</i></c>          | 1.7x
/// <c><i>FFXM_FSR2_UPSCALING_RATIO_X2</i></c>			  | 2.0x
///
/// Passing an invalid <c><i>upscalingRatio</i></c> will return 0.0f.
///
/// @param [in] upscalingRatio             The upscaling ratio.
///
/// @returns
/// The upscaling the per-dimension upscaling ratio for
/// <c><i>upscalingRatio</i></c> according to the table above.
///
/// @ingroup ffxmFsr2
FFXM_API float ffxmFsr2GetUpscaleRatioFactor(FfxmFsr2UpscalingRatio upscalingRatio);

/// A helper function to calculate the rendering resolution from a target
/// resolution and desired quality level.
///
/// This function applies the scaling factor returned by
/// <c><i>ffxmFsr2GetUpscaleRatioFromQualityMode</i></c> to each dimension.
///
/// @param [out] pRenderWidth            A pointer to a <c>uint32_t</c> which will hold the calculated render resolution width.
/// @param [out] pRenderHeight           A pointer to a <c>uint32_t</c> which will hold the calculated render resolution height.
/// @param [in] displayWidth            The target display resolution width.
/// @param [in] displayHeight           The target display resolution height.
/// @param [in] upscalingRatio          The selected upscaling ratio.
///
/// @retval
/// FFXM_OK                              The operation completed successfully.
/// @retval
/// FFXM_ERROR_INVALID_POINTER           Either <c><i>renderWidth</i></c> or <c><i>renderHeight</i></c> was <c>NULL</c>.
/// @retval
/// FFXM_ERROR_INVALID_ENUM              An invalid upscalingRatio was specified.
///
/// @ingroup ffxmFsr2
FFXM_API FfxmErrorCode ffxmFsr2GetRenderResolutionFromUpscalingRatio(
    uint32_t* pRenderWidth,
    uint32_t* pRenderHeight,
    uint32_t displayWidth,
    uint32_t displayHeight,
    FfxmFsr2UpscalingRatio upscalingRatio);

/// A helper function to calculate the jitter phase count from display
/// resolution.
///
/// For more detailed information about the application of camera jitter to
/// your application's rendering please refer to the
/// <c><i>ffxmFsr2GetJitterOffset</i></c> function.
///
/// The table below shows the jitter phase count which this function
/// would return for each of the quality presets.
///
/// Quality preset                                        | Scale factor  | Phase count
/// ----------------------------------------------------- | ------------- | ---------------
/// <c><i>FFXM_FSR2_QUALITY_MODE_QUALITY</i></c>           | 1.5x          | 18
/// <c><i>FFXM_FSR2_QUALITY_MODE_BALANCED</i></c>          | 1.7x          | 23
/// <c><i>FFXM_FSR2_QUALITY_MODE_PERFORMANCE</i></c>       | 2.0x          | 32
/// <c><i>FFXM_FSR2_QUALITY_MODE_ULTRA_PERFORMANCE</i></c> | 3.0x          | 72
/// Custom                                                | [1..n]x       | ceil(8*n^2)
///
/// @param [in] renderWidth             The render resolution width.
/// @param [in] displayWidth            The display resolution width.
///
/// @returns
/// The jitter phase count for the scaling factor between <c><i>renderWidth</i></c> and <c><i>displayWidth</i></c>.
///
/// @ingroup ffxmFsr2
FFXM_API int32_t ffxmFsr2GetJitterPhaseCount(int32_t renderWidth, int32_t displayWidth);

/// A helper function to calculate the subpixel jitter offset.
///
/// FSR2 relies on the application to apply sub-pixel jittering while rendering.
/// This is typically included in the projection matrix of the camera. To make
/// the application of camera jitter simple, the FSR2 API provides a small set
/// of utility function which computes the sub-pixel jitter offset for a
/// particular frame within a sequence of separate jitter offsets. To begin, the
/// index within the jitter phase must be computed. To calculate the
/// sequence's length, you can call the <c><i>ffxmFsr2GetJitterPhaseCount</i></c>
/// function. The index should be a value which is incremented each frame modulo
/// the length of the sequence computed by <c><i>ffxmFsr2GetJitterPhaseCount</i></c>.
/// The index within the jitter phase  is passed to
/// <c><i>ffxmFsr2GetJitterOffset</i></c> via the <c><i>index</i></c> parameter.
///
/// This function uses a Halton(2,3) sequence to compute the jitter offset.
/// The ultimate index used for the sequence is <c><i>index</i></c> %
/// <c><i>phaseCount</i></c>.
///
/// It is important to understand that the values returned from the
/// <c><i>ffxmFsr2GetJitterOffset</i></c> function are in unit pixel space, and
/// in order to composite this correctly into a projection matrix we must
/// convert them into projection offsets. This is done as per the pseudo code
/// listing which is shown below.
///
///     const int32_t jitterPhaseCount = ffxmFsr2GetJitterPhaseCount(renderWidth, displayWidth);
///
///     float jitterX = 0;
///     float jitterY = 0;
///     ffxmFsr2GetJitterOffset(&jitterX, &jitterY, index, jitterPhaseCount);
///
///     const float jitterX = 2.0f * jitterX / (float)renderWidth;
///     const float jitterY = -2.0f * jitterY / (float)renderHeight;
///     const Matrix4 jitterTranslationMatrix = translateMatrix(Matrix3::identity, Vector3(jitterX, jitterY, 0));
///     const Matrix4 jitteredProjectionMatrix = jitterTranslationMatrix * projectionMatrix;
///
/// Jitter should be applied to all rendering. This includes opaque, alpha
/// transparent, and raytraced objects. For rasterized objects, the sub-pixel
/// jittering values calculated by the <c><i>iffxmFsr2GetJitterOffset</i></c>
/// function can be applied to the camera projection matrix which is ultimately
/// used to perform transformations during vertex shading. For raytraced
/// rendering, the sub-pixel jitter should be applied to the ray's origin,
/// often the camera's position.
///
/// Whether you elect to use the <c><i>ffxmFsr2GetJitterOffset</i></c> function
/// or your own sequence generator, you must program the
/// <c><i>jitterOffset</i></c> field of the
/// <c><i>FfxmFsr2DispatchParameters</i></c> structure in order to inform FSR2
/// of the jitter offset that has been applied in order to render each frame.
///
/// If not using the recommended <c><i>ffxmFsr2GetJitterOffset</i></c> function,
/// care should be taken that your jitter sequence never generates a null vector;
/// that is value of 0 in both the X and Y dimensions.
///
/// @param [out] pOutX                   A pointer to a <c>float</c> which will contain the subpixel jitter offset for the x dimension.
/// @param [out] pOutY                   A pointer to a <c>float</c> which will contain the subpixel jitter offset for the y dimension.
/// @param [in] index                   The index within the jitter sequence.
/// @param [in] phaseCount              The length of jitter phase. See <c><i>ffxmFsr2GetJitterPhaseCount</i></c>.
///
/// @retval
/// FFXM_OK                              The operation completed successfully.
/// @retval
/// FFXM_ERROR_INVALID_POINTER           Either <c><i>outX</i></c> or <c><i>outY</i></c> was <c>NULL</c>.
/// @retval
/// FFXM_ERROR_INVALID_ARGUMENT          Argument <c><i>phaseCount</i></c> must be greater than 0.
///
/// @ingroup ffxmFsr2
FFXM_API FfxmErrorCode ffxmFsr2GetJitterOffset(float* pOutX, float* pOutY, int32_t index, int32_t phaseCount);

/// A helper function to check if a resource is
/// <c><i>FFXM_FSR2_RESOURCE_IDENTIFIER_NULL</i></c>.
///
/// @param [in] resource                A <c><i>FfxmResource</i></c>.
///
/// @returns
/// true                                The <c><i>resource</i></c> was not <c><i>FFXM_FSR2_RESOURCE_IDENTIFIER_NULL</i></c>.
/// @returns
/// false                               The <c><i>resource</i></c> was <c><i>FFXM_FSR2_RESOURCE_IDENTIFIER_NULL</i></c>.
///
/// @ingroup ffxmFsr2
FFXM_API bool ffxmFsr2ResourceIsNull(FfxmResource resource);

#if defined(__cplusplus)
}
#endif // #if defined(__cplusplus)

} // namespace arm
