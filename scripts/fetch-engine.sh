#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
. "$ROOT/vendor/JFBLOOD.lock"

DST="$ROOT/vendor/jfblood"
rm -rf "$DST"
git clone --no-checkout "$JFBLOOD_REPO" "$DST"
git -C "$DST" checkout --detach "$JFBLOOD_COMMIT"
git -C "$DST" submodule update --init --recursive

actual="$(git -C "$DST" rev-parse HEAD)"
[[ "$actual" == "$JFBLOOD_COMMIT" ]] || { echo "wrong JFBlood commit: $actual" >&2; exit 1; }

jf="$(git -C "$DST/jfbuild" rev-parse HEAD)"
case "$jf" in
  ${JFBUILD_EXPECTED_PREFIX}*) ;;
  *) echo "unexpected JFBuild commit: $jf" >&2; exit 1 ;;
esac

echo "JFBlood: $actual"
echo "JFBuild: $jf"
echo "Engine/game source fetched and pinned."
