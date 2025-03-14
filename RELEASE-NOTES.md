<!-- Copyright © 2024-2025 Arm Limited.
SPDX-License-Identifier: MIT -->

## Initial Public Release

**Arm® Accuracy Super Resolution™** is a mobile-optimized temporal upscaling technique derived from AMD's [FidelityFX™ Super Resolution 2 v2.2.2](https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK/blob/main/docs/techniques/super-resolution-temporal.md). Arm ASR includes multiple optimizations on top of the original **FidelityFX™ Super Resolution 2** to make the technique suited for the more resource-constrained environment of mobile gaming. The team would like to express their gratitude to AMD and the engineers behind the original technique for their work and releasing it publicly to the world.

This generic library implementation can be integrated into a custom engine. An integration guide can be found in the [README.md](./README.md).

Features include:
- Different integration possibilities - Quick integration with built-in standalone renderer or tight integration with user specified renderer.
- Robust Contrast-Adaptive Sharpening (RCAS) - User specified option to enable image sharpening. This will improve the level of detail in the output.
- Automatic reactive mask generation - User specified option to improve quality for fast moving, translucent objects.
- Shader quality presets (Quality, Balanced or Performance) - User specified option to allow fine tuning of image quality, performance or a balance of both.

More information on the technique, features, performance and the image quality of Arm ASR can be found [here](https://community.arm.com/arm-community-blogs/b/graphics-gaming-and-vr-blog/posts/introducing-arm-accuracy-super-resolution) or in the [README.md](./README.md).