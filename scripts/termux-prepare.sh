#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
pkg update -y
pkg install -y git curl unzip zip coreutils clang make
echo
echo "Termux host-side tools installed."
echo "N64 cross-compilation is intended to run in GitHub Actions/libdragon Docker."
