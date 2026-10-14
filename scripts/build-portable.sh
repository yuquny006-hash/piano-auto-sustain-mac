#!/usr/bin/env bash
set -euo pipefail
repo="$(cd "$(dirname "$0")/.." && pwd)"
: "${1:?Usage: bash scripts/build-portable.sh /absolute/path/to/JUCE-8.0.12}"
args=(-S "$repo" -B "$repo/build-portable" -DCMAKE_BUILD_TYPE=Release -DBUILD_PLUGIN=ON -DBUILD_NATIVE_MIDI_PROBE=OFF "-DJUCE_SOURCE_DIR=$1")
platform="$(uname -s)"
if [[ "$platform" == Darwin ]]; then args+=("-DCMAKE_OSX_ARCHITECTURES=${PAS_ARCHS:-arm64;x86_64}"); fi
cmake "${args[@]}"
cmake --build "$repo/build-portable" --config Release --parallel 2 --target BassPedal_VST3 vst3_processor_tests
ctest --test-dir "$repo/build-portable" -C Release -R '^vst3_(processor|scan)$' --output-on-failure
mkdir -p "$repo/dist"
tar -czf "$repo/dist/Piano-Auto-Sustain-1.0-$platform-local-build.tar.gz" -C "$repo/build-portable/BassPedal_artefacts/Release/VST3" 'Piano Auto Sustain.vst3' -C "$repo" licenses docs/VST3-GUIDE.md
echo 'Local build only: signing and host acceptance remain required.'
