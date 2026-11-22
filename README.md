# DIRT

DIRT is an original JUCE-based audio-effect plugin prototype inspired by the broad sound-design ideas in the reference image: tube-like saturation, biased mechanical distortion, a resonant multimode filter, dynamics, output and wet/dry mix. It is not affiliated with or a reproduction of IFeature's proprietary implementation or assets.

## Build

Requirements: CMake 3.22+, a C++17 compiler, and Git. JUCE 8.0.6 is fetched by CMake on the first configure.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The build generates VST3 and Standalone targets. Open the generated solution/project in your IDE if you want to debug the plugin.

## Controls

- **Tube**: warm, soft-clipping saturation.
- **Mech**: biased arctangent waveshaping.
- **Filter**: low-pass, band-pass or high-pass; 12/24/36 dB-per-octave cascades, cutoff and resonance.
- **Comp / Phat**: lightweight broadband downward/upward dynamics controls (not a multiband compressor).
- **Volume / Mix**: output level and parallel wet/dry blend.

This is an early DSP/UI foundation, not a feature-equivalent recreation. The original plugin's exact algorithms and behavior are not public; tune and extend this code by ear. No original plugin binaries, code, logos or artwork are included.
