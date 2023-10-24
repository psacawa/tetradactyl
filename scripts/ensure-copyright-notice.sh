#!/usr/bin/env bash
# Copyright 2023 Paweł Sacawa. All rights reserved.

ret=0
for file in $@; do
  if ! file $file | rg -q "\btext\b"; then
    continue
  fi
  if ! head $file | rg -q Copyright; then
    echo $file has no copyright notice >/dev/stderr
    ret=1
  fi
done

exit $ret
