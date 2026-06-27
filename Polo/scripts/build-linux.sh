#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
qmake6 Polo.pro
make -j"$(nproc)"
