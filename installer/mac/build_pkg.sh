#!/bin/bash
# Baut das macOS-Installer-Paket.  Aufruf:  installer/mac/build_pkg.sh [version]
# Optional (für Signierung/Notarisierung, braucht Apple Developer Account):
#   APP_SIGN_ID="Developer ID Application: Name (TEAMID)"
#   PKG_SIGN_ID="Developer ID Installer: Name (TEAMID)"
set -euo pipefail

VERSION="${1:-1.0.0}"
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUNDLE="$ROOT/build/RavenClipper_artefacts/Release/VST3/RavenClipper.vst3"
STAGE="$ROOT/build/pkg-root"
OUT="$ROOT/dist"

[ -d "$BUNDLE" ] || { echo "Plugin nicht gefunden: $BUNDLE (erst bauen!)"; exit 1; }

rm -rf "$STAGE"; mkdir -p "$STAGE/Library/Audio/Plug-Ins/VST3" "$OUT"
cp -R "$BUNDLE" "$STAGE/Library/Audio/Plug-Ins/VST3/"

if [ -n "${APP_SIGN_ID:-}" ]; then
    codesign --force --deep --timestamp --options runtime --sign "$APP_SIGN_ID" \
        "$STAGE/Library/Audio/Plug-Ins/VST3/RavenClipper.vst3"
else
    # Ad-hoc-Signatur: nötig, damit Apple-Silicon-Macs das Plugin überhaupt laden
    codesign --force --deep --sign - "$STAGE/Library/Audio/Plug-Ins/VST3/RavenClipper.vst3"
fi

pkgbuild --root "$STAGE" \
         --identifier com.ravenclipper.vst3 \
         --version "$VERSION" \
         --install-location / \
         --ownership recommended \
         "$ROOT/build/RavenClipper-component.pkg"

SIGN_ARGS=()
[ -n "${PKG_SIGN_ID:-}" ] && SIGN_ARGS=(--sign "$PKG_SIGN_ID")

productbuild --distribution "$ROOT/installer/mac/Distribution.xml" \
             --resources "$ROOT/installer/mac/resources" \
             --package-path "$ROOT/build" \
             ${SIGN_ARGS[@]+"${SIGN_ARGS[@]}"} \
             "$OUT/RavenClipper-$VERSION-macOS.pkg"

echo "Fertig: $OUT/RavenClipper-$VERSION-macOS.pkg"
