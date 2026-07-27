#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
pkg install -y curl unzip coreutils findutils
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
exec bash "$SCRIPT_DIR/fetch-shareware.sh"
