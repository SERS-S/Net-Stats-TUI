#!/usr/bin/env bash

set -eu

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$SCRIPT_DIR"
cd "$SCRIPT_DIR"

SUDO=""
if [[ "$(id -u)" -ne 0 ]]; then
    SUDO="sudo"
fi

printf "Remove installed net-stats and delete project directory '%s'? [y/N] " "$PROJECT_DIR"
read -r confirm

case "$confirm" in
    y|Y|yes|YES) ;;
    *)
        echo "Canceled"
        exit 0
    ;;
esac

if [[ -f build/install_manifest.txt ]]; then
    while IFS= read -r installed_path; do
        [[ -n "$installed_path" ]] || continue
        if [[ -e "$installed_path" || -L "$installed_path" ]]; then
            $SUDO rm -f "$installed_path"
            echo "Removed: $installed_path"
        fi
    done < build/install_manifest.txt
else
    if [[ -e /usr/local/bin/net-stats || -L /usr/local/bin/net-stats ]]; then
        $SUDO rm -f /usr/local/bin/net-stats
        echo "Removed: /usr/local/bin/net-stats"
    else
        echo "net-stats is not installed in /usr/local/bin"
    fi
fi

echo "OK : net-stats uninstalled"
echo "Removing project directory: $PROJECT_DIR"

cd /

if [[ -n "$SUDO" ]]; then
    nohup sudo sh -c 'sleep 1; rm -rf "$1"' _ "$PROJECT_DIR" >/dev/null 2>&1 &
else
    nohup sh -c 'sleep 1; rm -rf "$1"' _ "$PROJECT_DIR" >/dev/null 2>&1 &
fi
