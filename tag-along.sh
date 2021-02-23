#!/bin/bash
inotifywait -q -m -e close_write program.c |
while read -r filename event; do
  ctags program.c
done
