#!/bin/bash
set -u
out="$HOME/Desktop/Piano-Auto-Sustain-Logic-$(date +%Y%m%d-%H%M%S).txt"
{
  echo 'Piano Auto Sustain - in-host activation diagnostics V1'
  date -u
  sw_vers
  uname -m
  echo '--- Last 15 minutes: diagnostic build activation callbacks only ---'
  /usr/bin/log show --last 15m --style compact --predicate 'eventMessage CONTAINS "PAS_AUTH_NETWORK_V1"' 2>&1
  echo "log_exit=$?"
  echo 'No authorization requests were sent by this collector.'
  echo 'No result means: old build, no recent request, or OS log unavailable; not proof of a healthy connection.'
} > "$out"
open -a TextEdit "$out"
echo "Saved: $out"
