#!/bin/bash
set -euo pipefail
[[ "$(uname -s)" == Darwin ]] || exit 1
base="$HOME/Library/Audio/Plug-Ins"
for entry in 'VST3/Piano Auto Sustain.vst3' 'Components/Piano Auto Sustain MIDI FX.component'; do
  target="$base/$entry"
  [[ ! -L "$target" && ! -L "$(dirname "$target")" ]] || exit 1
  if [[ -d "$target" ]]; then
    backup="$(mktemp -d "$(dirname "$target")/.PAS-uninstalled-XXXXXX")"
    mv "$target" "$backup/$(basename "$target")"
    echo "Moved to $backup"
  fi
done
echo '插件已移出扫描路径；授权和工程均保留。重启宿主。'
