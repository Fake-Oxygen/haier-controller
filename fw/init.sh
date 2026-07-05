#!/bin/bash
set -e

PRJ_DIR=$(pwd)

python -m venv "$PRJ_DIR/.venv"

PIP=$PRJ_DIR/.venv/bin/pip
WEST=$PRJ_DIR/.venv/bin/west

$PIP install west

mkdir -p external

export ZEPHYR_BASE="$PRJ_DIR/external/"

$WEST update 
$WEST zephyr-export

export ZEPHYR_BASE="$PRJ_DIR/external/zephyr"

$WEST packages pip --install 

$WEST sdk install -b $ZEPHYR_BASE/zephyr -d $ZEPHYR_BASE/zephyr --gnu-toolchains riscv64-zephyr-elf x86_64-zephyr-elf
$WEST blobs fetch hal_espressif
echo "source .venv/bin/activate" > .envrc
echo "export ZEPHYR_BASE=\"$ZEPHYR_BASE\"" >> .envrc
echo "west zephyr-export" >> .envrc

direnv allow
