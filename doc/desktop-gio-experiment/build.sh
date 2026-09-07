#!/bin/sh
set -eu

: "${BOOT_JDK:?Set BOOT_JDK to a complete JDK 21}"
CONF=${CONF:-desktop-gio}
JOBS=${JOBS:-8}
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
cd "$ROOT"
bash configure --with-conf-name="$CONF" --with-boot-jdk="$BOOT_JDK" \
    --with-jvm-variants=server --with-debug-level=release \
    --with-native-debug-symbols=internal --with-jobs="$JOBS" \
    --disable-warnings-as-errors
make CONF="$CONF" JOBS="$JOBS" images
