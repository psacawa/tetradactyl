#!/usr/bin/env bash
# Copyright 2023 Paweł Sacawa. All rights reserved.

[[ -z "${HOME}" ]] && exit 1

# remove tetradactyl config/data from FS
rm -r "${XDG_DATA_HOME:-${HOME}}/.local/share/tetradactyl"
rm -r "${XDG_CONFIG_HOME:-${HOME}/.config}/tetradactyl"
