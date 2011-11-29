#!/bin/bash -e

# Sub-directory for generated Doxygen.
SUBDIR="./libcvd-cl-doc"

# Current git HEAD.
COMMIT="$(git rev-parse HEAD)"

# GitHub repository.
GITHUB="git@github.com:dnikulin/libcvd-cl-doc.git"

# Commit message.
MESSAGE="Generate Doxygen for commit $COMMIT"

# Clear past documentation.
# The * excludes .git
rm -fvr "$SUBDIR/*"

# Re-generate documentation.
doxygen doxygen.conf

# Re-commit and push documentation.
( cd "$SUBDIR" && git add . && git commit -a -m "$MESSAGE" && git push "$GITHUB" master:gh-pages )

