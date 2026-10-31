# EVER Native One-Pass: implementation and testing

## Goal

Keep upstream [EVER](https://github.com/MiffedLyric559/EVER)'s genuine
DX11 GPU capture, ReShade support, game-bake hooks, LibAV/FFmpeg encoder,
audio handling and output presets. Do not implement temporal sampling.

The exported image path is:

```
Rockstar Editor native one-frame bake
  -> game GPU/postprocessing color buffer
  -> EVER's two genuine IDXGISwapChain::Present(1,0) callbacks (ReShade/ENB compatibility)
  -> GPU backbuffer -> D3D11 staging texture
  -> map exactly one frame
  -> EVER EncoderSession::enqueueVideoFrame
  -> same FFmpegEncoder worker and preset
  -> output video
```

## Changes from upstream EVER

- **No motion_blur_samples or motion_blur_strength** in the config schema.
- No `PSAccumulate.hlsl`, `PSDivide.hlsl`, or `VSFullScreen.hlsl`.
- No `drawAdditive`, `divideBuffer`, or accumulation target allocation.
- Preserved upstream EVER's two real `Present(1,0)` callbacks for ReShade and ENB compatibility. These do **not** re-render the GTA scene or generate extra samples.
- GPU backbuffer copied to staging directly, mapped once and queued to the
  existing asynchronous video encoder. The output color format is selected
  from the actual DXGI swapchain format: BGRA maps to `bgra`, RGBA to
  `rgba`. Only 8-bit BGRA/RGBA surfaces are supported at this stage.
- The `GetRenderTimeBase` timestep is now `1000 * fps_den / fps_num`,
  without a subframe multiplier.
- Removed sampling-era fixed-30Hz ripple and cloth synchronization hooks,
  leaving those native GTA simulation updates alone.
- Upstream's optional separate audio pass above 60 fps remains: that is
  an audio/video synchronization mechanism, **not** extra visual samples.
- Still resolves the GTA render texture named `VideoEncode` and identifies
  render boundaries using `OMSetRenderTargets`.

## Install (from successful Windows CI artifact)

**Do not install alongside BetterVideoExport.asi:** both intercept GTA
video/encoding hooks and can conflict.

1. Shut down FiveM and uninstall or move aside `BetterVideoExport.asi`,
   `VYNE_BakeFX.addon64` and the experimental VYNE ReShade shader.
2. Extract the **entire** EVER build artifact inside
   `%LOCALAPPDATA%\FiveM\FiveM.app\plugins\`. The correct structure is:

   ```
   plugins/
     EVER.asi
     EVER/
       EVER.ini
       preset.json
       presets/
       dlls/
         ... FFmpeg and runtime DLLs ...
   ```

3. Use a working ReShade installation for FiveM with the effects you
   intend to see. ReShade must be loaded for its shaders to be captured.
4. Run Rockstar Editor's native bake normally, at your selected resolution
   and FPS. Set `EVER/EVER.ini` `[EXPORT]` `fps = 60` initially.
5. Check the game's `EVER` log and the output against the ReShade preview.

## What this does not solve yet

- This is not a verified final **post-ReShade** video capture. Upstream
  EVER uses its swapchain Present hook to run ReShade; retaining its two
  genuine Presents should execute the same hooks but must be verified
  visually on FiveM 3788. Some flip-model configurations may require
  capturing before the buffer rotates.
- GTA's timecycle `postfx_motionblurlength` can be enabled with the
  user's Normal preset, but this branch does **not** yet force an editor
  camera's native motion-blur pass. The previously tested timecycle
  variable override and `SET_FORCE_MOTIONBLUR` calls alone did not
  establish native blur during video bake.
- Exported dimensions do **not** prove true internal scene resolution.
  GTA's resolution clamp may still upscale a 1080p source.
- The staging texture is currently created for each frame; reusable
  asynchronous staging readback can improve throughput after validation.
- Source has been changed and Windows CI run requested, but game-side
  validation still requires a render.

## Source and licensing

Based on MiffedLyric559/EVER (Apache 2.0); original credits and LICENSE
preserved. Native postprocessing changes require build-specific checks
and should not be copied blindly from FiveM b3258 into b3788.
