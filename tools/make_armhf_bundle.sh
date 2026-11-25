#!/usr/bin/env bash
# The 32-bit ARM build as a folder that runs on a 64-bit ARM Linux with no 32-bit libraries of its own
# (the AYN Thor under Armada: its kernel runs 32-bit programs on the cores that can, with
# allow_mismatched_32bit_el0). Run on the 32-bit ARM system or container that built build/albw
# (ALBW_ARCH=armhf tools/link.sh): the program, the dynamic loader and every library it and Mesa
# load, from that system, plus albw.sh, which starts the program through the bundled loader.
#
#   tools/make_armhf_bundle.sh [out]     default build/albw-armhf (and build/albw-armhf.tar.gz)
set -euo pipefail
cd "$(dirname "$0")/.."
OUT=${1:-build/albw-armhf}
[[ $(uname -m) == armv7l || $(uname -m) == armv8l ]] || { echo "run on the 32-bit ARM system that built build/albw"; exit 1; }
[[ -f build/albw && -f build/albw.ssmap ]] || { echo "build/albw missing: ALBW_ARCH=armhf tools/link.sh"; exit 1; }
rm -rf "$OUT"
mkdir -p "$OUT/lib/dri" "$OUT/share/glvnd/egl_vendor.d"
cp build/albw build/albw.ssmap "$OUT/"
# opened at run time, not linked: GL / EGL through glvnd and Mesa (Adreno: msm), Wayland (SDL, the
# leased panel), X11 (SDL under gamescope's Xwayland), sound (SDL: PipeWire, PulseAudio, ALSA)
DLOPEN=(libGL.so.1 libEGL.so.1 libGLX.so.0 libGLdispatch.so.0 libEGL_mesa.so.0 libGLX_mesa.so.0 libgbm.so.1
        libwayland-client.so.0 libwayland-egl.so.1 libwayland-cursor.so.0 libxkbcommon.so.0 libdecor-0.so.0
        libX11.so.6 libX11-xcb.so.1 libXext.so.6 libXcursor.so.1 libXi.so.6 libXrandr.so.2 libXfixes.so.3
        libXss.so.1 libpulse.so.0 libasound.so.2 libpipewire-0.3.so.0)
DRI=(msm_dri.so kgsl_dri.so swrast_dri.so kms_swrast_dri.so libdril_dri.so)
want=(build/albw)
for l in "${DLOPEN[@]}"; do [[ -e /usr/lib/$l ]] && want+=("/usr/lib/$l"); done
for d in "${DRI[@]}"; do [[ -e /usr/lib/dri/$d ]] && cp -L "/usr/lib/dri/$d" "$OUT/lib/dri/" && want+=("/usr/lib/dri/$d"); done
# GBM's backend (the tests' panel buffers, ALBW_DRM_LEASE_TEST)
mkdir -p "$OUT/lib/gbm"
for g in /usr/lib/gbm/*.so; do [[ -e $g ]] && cp -L "$g" "$OUT/lib/gbm/" && want+=("$g"); done
# every library those need, the C library and loader included (the target has none of its own)
ldd "${want[@]}" 2>/dev/null | awk '/=> \//{print $3} /^\s*\/.*ld-linux/{print $1}' | sort -u | while read -r so; do
    cp -L "$so" "$OUT/lib/"
done
for l in "${DLOPEN[@]}"; do [[ -e /usr/lib/$l ]] && cp -L "/usr/lib/$l" "$OUT/lib/"; done
cp /usr/share/glvnd/egl_vendor.d/50_mesa.json "$OUT/share/glvnd/egl_vendor.d/"
[[ -e "$OUT/lib/ld-linux-armhf.so.3" ]] || { echo "the dynamic loader was not found"; exit 1; }
cat > "$OUT/albw.sh" <<'EOF'
#!/usr/bin/env bash
# Starts the 32-bit ARM build with its own libraries (tools/make_armhf_bundle.sh).
D=$(dirname "$(readlink -f "$0")")
# the game file: --rom PATH, ALBW_ROM or the remembered path, as packaging/appimage/AppRun does
CFG="${XDG_CONFIG_HOME:-$HOME/.config}/albw-port"
ARGS=()
while [ $# -gt 0 ]; do
    case "$1" in
        --rom) ALBW_ROM="$2"; shift 2 ;;
        --rom=*) ALBW_ROM="${1#--rom=}"; shift ;;
        *) ARGS+=("$1"); shift ;;
    esac
done
: "${ALBW_ROM:=$(cat "$CFG/rom_path" 2>/dev/null)}"
if [ ! -f "${ALBW_ROM:-}" ]; then
    echo "No game file. Run with --rom /path/to/your/decrypted.3ds (EU release), or set ALBW_ROM." >&2
    exit 1
fi
mkdir -p "$CFG" && printf '%s\n' "$(readlink -f "$ALBW_ROM")" > "$CFG/rom_path"
export ALBW_ROM
# no terminal (Steam Game Mode, desktop launchers): the output goes to a log file, as
# packaging/appimage/AppRun does. The game's Help menu (Esc) copies it (ALBW_LOG_FILE).
if [ ! -t 2 ]; then
    LOGDIR="${XDG_STATE_HOME:-$HOME/.local/state}/albw-port"
    if mkdir -p "$LOGDIR" 2>/dev/null; then
        export ALBW_LOG_FILE="$LOGDIR/last-run.log"
        echo "== ALBW native armhf $(date) GAMESCOPE_WAYLAND_DISPLAY=${GAMESCOPE_WAYLAND_DISPLAY:-} DISPLAY=${DISPLAY:-} lease ${ALBW_DRM_LEASE:-none}" > "$ALBW_LOG_FILE"
        exec >> "$ALBW_LOG_FILE" 2>&1
    fi
fi
# gamescope (Game Mode): Steam's overlay library is preloaded into shortcuts (a 64-bit library: the
# loader only warns, but the AppImage drops it too) and games run on its Xwayland
if [ -n "${GAMESCOPE_WAYLAND_DISPLAY:-}" ]; then
    unset LD_PRELOAD
    [ -n "${DISPLAY:-}" ] && [ -z "${SDL_VIDEO_DRIVER:-}" ] && export SDL_VIDEO_DRIVER=x11
fi
# Armada (github.com/armada-os/armada) on a two-panel handheld such as the AYN Thor: the tool
# /usr/libexec/armada/device-env prints the device profile as shell variables, among them the bottom
# panel's connector (ARMADA_SECONDARY_CONNECTOR) and its touchscreen's input name
# (ARMADA_SECONDARY_TOUCHSCREEN). In Game Mode the main gamescope (socket gamescope-primary) shows
# the top panel and offers the bottom one for lease: ALBW_DRM_LEASE asks for it. The touchscreen is
# found by its name in /sys/class/input. Values already set are kept.
profile=/usr/libexec/armada/device-env
if [ -x "$profile" ]; then
    connector=$("$profile" 2>/dev/null | sed -n "s/^ARMADA_SECONDARY_CONNECTOR=['\"]\{0,1\}\([^'\"]*\).*/\1/p")
    touch_name=$("$profile" 2>/dev/null | sed -n "s/^ARMADA_SECONDARY_TOUCHSCREEN=['\"]\{0,1\}\([^'\"]*\).*/\1/p")
    if [ -n "$touch_name" ] && [ -z "${ALBW_TOUCH_DEVICE:-}" ]; then
        for dev in /sys/class/input/event*; do
            if [ "$(cat "$dev/device/name" 2>/dev/null)" = "$touch_name" ]; then
                export ALBW_TOUCH_DEVICE="/dev/input/$(basename "$dev")"
                break
            fi
        done
    fi
    runtime="${XDG_RUNTIME_DIR:-/run/user/$(id -u)}"
    if [ -n "$connector" ] && [ -n "${GAMESCOPE_WAYLAND_DISPLAY:-}" ] && [ -e "$runtime/gamescope-primary" ] && [ -z "${ALBW_DRM_LEASE+set}" ]; then
        export ALBW_DRM_LEASE="$connector" GAMESCOPE_WAYLAND_DISPLAY=gamescope-primary
    fi
    echo "Armada: bottom panel ${connector:-none}, touchscreen ${ALBW_TOUCH_DEVICE:-not found}, lease ${ALBW_DRM_LEASE:-off (not Game Mode)}" >&2
fi
export LD_LIBRARY_PATH="$D/lib" LIBGL_DRIVERS_PATH="$D/lib/dri" GBM_BACKENDS_PATH="$D/lib/gbm" __EGL_VENDOR_LIBRARY_DIRS="$D/share/glvnd/egl_vendor.d"
export ALBW_EXE="$D/albw"
exec "$D/lib/ld-linux-armhf.so.3" "$D/albw" "${ARGS[@]}"
EOF
chmod +x "$OUT/albw.sh"
tar -C "$(dirname "$OUT")" -czf "$OUT.tar.gz" "$(basename "$OUT")"
echo "bundle: $OUT ($(du -sh "$OUT" | cut -f1)), $OUT.tar.gz"
