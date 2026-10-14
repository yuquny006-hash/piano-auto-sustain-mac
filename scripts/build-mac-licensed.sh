#!/bin/bash
set -euo pipefail
[[ "$(uname -s)" == Darwin ]] || { echo 'Run on a Mac.'; exit 1; }
root="$(cd "$(dirname "$0")/.." && pwd)"
keyfile="${1:-$root/mac-public-key.txt}"
[[ -f "$keyfile" ]] || { echo 'Missing mac-public-key.txt: obtain PUBLIC key from server administrator.'; exit 1; }
public_key="$(tr -d '\r\n' < "$keyfile")"
[[ "$public_key" =~ ^[0-9A-F]{64}$ ]] || { echo 'Expected 64 uppercase hex public key.'; exit 1; }
command -v cmake >/dev/null
command -v python3 >/dev/null
xcodebuild -version
# A new workspace per invocation preserves previous successful outputs.
work="$(mktemp -d "$root/mac-build-XXXXXX")"
echo "Build workspace: $work"
archive="$root/third_party/libsodium-1.0.20.tar.gz"
[[ "$(shasum -a 256 "$archive" | cut -d ' ' -f1)" == ebb65ef6ca439333c2bb41a0c1990587288da07f6c7fd07cb3a18cc18d30ce19 ]]
sdk="$(xcrun --sdk macosx --show-sdk-path)"
for architecture in arm64 x86_64; do
  mkdir -p "$work/$architecture"
  tar -xzf "$archive" -C "$work/$architecture"
  host=aarch64-apple-darwin; [[ "$architecture" != x86_64 ]] || host=x86_64-apple-darwin
  (cd "$work/$architecture/libsodium-1.0.20"
    CC="$(xcrun -f clang)" CFLAGS="-O2 -arch $architecture -isysroot $sdk -mmacosx-version-min=11.0" \
    LDFLAGS="-arch $architecture -isysroot $sdk -mmacosx-version-min=11.0" \
      ./configure --host="$host" --disable-shared --enable-static --prefix="$work/$architecture/install"
    make -j "$(sysctl -n hw.ncpu)"
    make install)
done
mkdir -p "$work/sodium/lib"
cp -R "$work/arm64/install/include" "$work/sodium/include"
lipo -create "$work/arm64/install/lib/libsodium.a" "$work/x86_64/install/lib/libsodium.a" -output "$work/sodium/lib/libsodium.a"
lipo -verify_arch arm64 x86_64 "$work/sodium/lib/libsodium.a"
cmake -S "$root" -B "$work/plugin" -G Xcode -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64' \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 -DPAS_MAC_PUBLIC_KEY_HEX="$public_key" -DPAS_SODIUM_ROOT="$work/sodium"
cmake --build "$work/plugin" --config Release --target BassPedal_VST3 BassPedalAU_AU mac_license_tests vst3_processor_tests -j 4
ctest --test-dir "$work/plugin" -C Release --output-on-failure -R '^(mac_license|vst3_processor|vst3_scan|product_version)$'
for bundle in "$work/plugin/BassPedal_artefacts/Release/VST3/Piano Auto Sustain.vst3" "$work/plugin/BassPedalAU_artefacts/Release/AU/Piano Auto Sustain MIDI FX.component"; do
  binary="$(/usr/libexec/PlistBuddy -c 'Print CFBundleExecutable' "$bundle/Contents/Info.plist")"
  lipo -verify_arch arm64 x86_64 "$bundle/Contents/MacOS/$binary"
  codesign --force --sign - --timestamp=none "$bundle"
  codesign --verify --deep --strict "$bundle"
  otool -L "$bundle/Contents/MacOS/$binary" > "$work/$(basename "$bundle").dependencies.txt"
  if grep -E '(libsodium.*dylib|/opt/homebrew/|/usr/local/)' "$work/$(basename "$bundle").dependencies.txt"; then
    echo 'Nonportable dynamic dependency found'; exit 1
  fi
done
python3 "$root/scripts/package_mac_licensed.py" --build "$work/plugin" --output "$work/output" --public-key "$public_key"
echo "Built candidate only: $work/output. Install and follow MAC-ACCEPTANCE.md before customer release."
