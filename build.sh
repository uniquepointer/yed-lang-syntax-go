#!/usr/bin/env bash
gcc -o go.so go.c $(yed --print-cflags) $(yed --print-ldflags)
