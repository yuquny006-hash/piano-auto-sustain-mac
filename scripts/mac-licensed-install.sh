#!/bin/bash
set -euo pipefail
[[ "$(uname -s)" == Darwin ]] || { echo 'This installer requires macOS.'; exit 1; }
package="$(cd "$(dirname "$0")" && pwd)"
base="$HOME/Library/Audio/Plug-Ins"
names=('Piano Auto Sustain.vst3' 'Piano Auto Sustain MIDI FX.component')
formats=(VST3 Components)
stages=('' ''); moved=(0 0); installed=(0 0); committed=0
finish() {
  local status=$?
  trap - EXIT INT TERM
  if [[ "$committed" != 1 ]]; then
    echo 'Installation failed. Restoring previous plugins…'
    local failed=0
    for i in 1 0; do
      [[ "${stages[$i]:-}" != '' ]] || continue
      target="$base/${formats[$i]}/${names[$i]}"
      if [[ "${installed[$i]}" == 1 ]]; then
        mv "$target" "${stages[$i]}/failed-new" || failed=1
      fi
      if [[ "${moved[$i]}" == 1 ]]; then
        mv "${stages[$i]}/previous" "$target" || failed=1
      fi
    done
    if [[ "$failed" == 1 ]]; then
      echo 'RESTORE INCOMPLETE: backups preserved at paths below. Contact support before retrying.'
    else echo 'Previous installations preserved/restored.'; fi
    printf '%s\n' "${stages[@]}"
    status=1
  fi
  exit "$status"
}
trap finish EXIT
trap 'exit 1' INT TERM
[[ -f "$package/SHA256SUMS.txt" ]] || { echo 'Required checksums missing.'; exit 1; }
# Reject unsafe checksum paths before invoking the system verifier.
while IFS= read -r line; do
  name="${line:66}"
  [[ "$line" =~ ^[0-9a-f]{64}\ \  ]] && [[ "$name" != /* && "$name" != ../* && "$name" != *'/../'* ]] || exit 1
done < "$package/SHA256SUMS.txt"
(cd "$package" && shasum -a 256 -c SHA256SUMS.txt)
for i in 0 1; do
  destination="$base/${formats[$i]}"; target="$destination/${names[$i]}"
  [[ ! -L "$base" && ! -L "$destination" && ! -L "$target" ]] || { echo 'Symbolic-link installation paths are unsupported.'; exit 1; }
  mkdir -p "$destination"
  stages[$i]="$(mktemp -d "$destination/.PAS-backup-XXXXXX")"
  [[ -d "$package/${names[$i]}" ]] || exit 1
  /usr/bin/ditto "$package/${names[$i]}" "${stages[$i]}/new"
  diff -r "$package/${names[$i]}" "${stages[$i]}/new" >/dev/null
  codesign --verify --deep --strict "${stages[$i]}/new"
  xattr -dr com.apple.quarantine "${stages[$i]}/new"
done
for i in 0 1; do
  target="$base/${formats[$i]}/${names[$i]}"
  if [[ -e "$target" ]]; then
    mv "$target" "${stages[$i]}/previous"; moved[$i]=1
  fi
  mv "${stages[$i]}/new" "$target"; installed[$i]=1
  codesign --verify --deep --strict "$target"
done
committed=1
echo '安装完成。重启宿主，在插件右上方输入授权码。'
echo 'Installed. Restart your DAW and activate inside the plugin.'
echo 'Previous versions, if any, are preserved here:'
printf '%s\n' "${stages[@]}"
