#!/usr/bin/env bash
# Copy transitive Boost dylibs that macdeployqtplus misses (@loader_path deps).
set -euo pipefail
APP="${1:?usage: $0 /path/to/Foo-Qt.app}"
FW="$APP/Contents/Frameworks"
BREW_LIB="$(brew --prefix)/lib"
mkdir -p "$FW"

changed=1
round=0
while [[ $changed -eq 1 && $round -lt 10 ]]; do
  changed=0
  round=$((round + 1))
  for dylib in "$FW"/*.dylib; do
    [[ -f "$dylib" ]] || continue
    while IFS= read -r dep; do
      base="$(basename "$dep")"
      [[ "$base" == libboost_*.dylib ]] || continue
      if [[ ! -f "$FW/$base" ]]; then
        src="$BREW_LIB/$base"
        if [[ ! -f "$src" ]]; then
          echo "MISSING source for $base (needed by $(basename "$dylib"))" >&2
          continue
        fi
        echo "Adding $base"
        cp -a "$src" "$FW/$base"
        install_name_tool -id "@executable_path/../Frameworks/$base" "$FW/$base"
        # Rewrite absolute brew paths inside the copied lib to @loader_path
        otool -L "$FW/$base" | awk '/^\t\// {print $1}' | while read -r abs; do
          b="$(basename "$abs")"
          if [[ "$b" == libboost_*.dylib ]]; then
            install_name_tool -change "$abs" "@loader_path/$b" "$FW/$base" 2>/dev/null || true
          fi
        done
        changed=1
      fi
    done < <(otool -L "$dylib" | awk '/libboost_.*\.dylib/ {print $1}')
  done
done

echo "Boost libs now in Frameworks:"
ls "$FW"/libboost_*.dylib 2>/dev/null | xargs -n1 basename
