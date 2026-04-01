#!/usr/bin/env bash
set -e

BRANCH="$(git branch --show-current)"

if [ -z "$BRANCH" ]; then
  echo "can't find current branch"
  exit 1
fi

echo "[INFO] branch: $BRANCH"
git status --short
git pull --rebase origin "$BRANCH"
