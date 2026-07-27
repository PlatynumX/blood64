#!/data/data/com.termux/files/usr/bin/bash
set -euo pipefail
pkg update -y
pkg install -y git curl unzip zip python

echo
echo "Blood64 r3 is the real-engine MIPS compile branch."
echo "After extracting this archive into your GitHub checkout:"
echo
echo "  git add ."
echo "  git commit -m 'Blood64 r3: bring in JFBuild and Blood engine compile'"
echo "  git push"
echo
echo "Then run the blood64-engine-r3 GitHub Action."
