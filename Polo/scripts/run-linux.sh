#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."
if [[ ! -x ./Polo ]]; then
    ./scripts/build-linux.sh
fi
exec ./Polo



