#!/usr/bin/env bash
set -e

BRANCH="$(git branch --show-current)"

if [ -z "$BRANCH" ]; then
  echo "can't find current branch"
  exit 1
fi

if [ $# -lt 1 ]; then
  echo "usage: ./tools/gpush.sh \"commit message\""
  exit 1
fi

MSG="$1"

echo "[INFO] branch: $BRANCH"
git status --short
git add .
git commit -m "$MSG"
git pull --rebase origin "$BRANCH"
git push origin "$BRANCH"
