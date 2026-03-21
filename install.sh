#!/usr/bin/env bash

set -eu

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if [[ "$(uname -s)" != "Linux" ]]; then
    echo "ERROR: this installer is intended for Linux only"
    exit 1
fi

if [[ -r /etc/os-release ]]; then
    . /etc/os-release
else
    echo "ERROR : /etc/os-release not found!"
    exit 1
fi

SUDO=""
if [[ "$(id -u)" -ne 0 ]]; then
    SUDO="sudo"
fi

function install_ubuntu_like() {
    if command -v timedatectl >/dev/null 2>&1; then
        $SUDO timedatectl set-ntp true || true
    fi

    local attempts=3
    local wait_seconds=75
    local i=1
    while (( i <= attempts )); do
        if $SUDO apt-get update; then
            break
        fi

        if (( i == attempts )); then
            echo "ERROR : apt-get update failed after ${attempts} attempts"
            return 1
        fi

        echo "WARN : apt metadata is not valid yet, retrying in ${wait_seconds}s (attempt ${i}/${attempts})"
        sleep "${wait_seconds}"
        ((i++))
    done

    $SUDO apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        pkg-config \
        libncursesw5-dev \
        libnl-3-dev \
        libnl-genl-3-dev \
        git \
        gdb
}

function install_rhel_like() {
    local PM
    if command -v dnf >/dev/null 2>&1; then
        PM=dnf
    else
        PM=yum
    fi

    $SUDO $PM -y install epel-release || true

    $SUDO $PM -y install \
        gcc gcc-c++ make git \
        pkgconf-pkg-config \
        ncurses-devel \
        libnl3-devel

    $SUDO $PM -y install cmake || $SUDO $PM -y install cmake3

    if command -v cmake3 >/dev/null 2>&1 && ! command -v cmake >/dev/null 2>&1; then
        $SUDO ln -sf "$(command -v cmake3)" /usr/local/bin/cmake
    fi
}

function install_arch_like() {
    $SUDO pacman -Sy --noconfirm --needed \
        base-devel \
        cmake \
        ninja \
        pkgconf \
        ncurses \
        libnl \
        git
}

function install_suse_like() {
    $SUDO zypper --non-interactive install \
        gcc gcc-c++ make git \
        cmake ninja \
        pkg-config \
        ncurses-devel \
        libnl3-devel
}

case "${ID:-}" in
    ubuntu|debian) install_ubuntu_like ;;
    centos|rhel|rocky|almalinux|ol|fedora) install_rhel_like ;;
    arch|manjaro) install_arch_like ;;
    opensuse*|sles) install_suse_like ;;
    *)
        if [[ "${ID_LIKE:-}" == *debian* ]]; then
            install_ubuntu_like
        elif [[ "${ID_LIKE:-}" == *rhel* || "${ID_LIKE:-}" == *fedora* ]]; then
            install_rhel_like
        elif [[ "${ID_LIKE:-}" == *arch* ]]; then
            install_arch_like
        elif [[ "${ID_LIKE:-}" == *suse* ]]; then
            install_suse_like
        else
            echo "ERROR : Unsupported distributive: ID=${ID:-?} ID_LIKE=${ID_LIKE:-?}"
            exit 1
        fi
    ;;
esac

echo "OK : deps installed"

if [[ -f build/CMakeCache.txt ]]; then
    CACHED_HOME_DIR="$(sed -n 's|^CMAKE_HOME_DIRECTORY:INTERNAL=||p' build/CMakeCache.txt | head -n1 || true)"
    if [[ -n "${CACHED_HOME_DIR:-}" && "$CACHED_HOME_DIR" != "$SCRIPT_DIR" ]]; then
        echo "INFO : stale CMake cache detected (${CACHED_HOME_DIR}), resetting build cache"
        rm -f build/CMakeCache.txt
        rm -rf build/CMakeFiles
    fi
fi

cmake -S . -B build -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build -- -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"
$SUDO cmake --install build

echo "OK : net-stats installed"
echo "Run it with: net-stats"
