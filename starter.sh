#! /usr/bin/env bash

set -eu

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

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
    $SUDO apt-get update
    $SUDO apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        ninja-build \
        pkg-config \
        libncursesw5-dev \
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
        ncurses-devel

    $SUDO $PM -y install cmake || $SUDO $PM -y install cmake3

    if command -v cmake3 >/dev/null 2>&1 && ! command -v cmake >/dev/null 2>&1; then
        $SUDO ln -sf "$(command -v cmake3)" /usr/local/bin/cmake
    fi
}

case "${ID:-}" in 
    ubuntu|debian) install_ubuntu_like ;;
    centos|rhel|rocky|almalinux|ol|fedora) install_rhel_like ;;
    *)

        if [[ "${ID_LIKE:-}" == *debian* ]]; then
            install_ubuntu_like
        elif [[ "${ID_LIKE:-}" == *rhel* || "${ID_LIKE:-}" == *fedora* ]]; then
            install_rhel_like
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

cmake -S . -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/net-stats

echo "OK : Cmake deployed"
