#!/usr/bin/env bash


export QT_QPA_PLATFORM=offscreen 
exec ./build/tests/demos/calculator/calculator-qt6
