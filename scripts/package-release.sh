#!/bin/bash
# Package a release for GitHub
# Usage: ./scripts/package-release.sh [version]
# Example: ./scripts/package-release.sh 6.0.0

set -e

VERSION=${1:-$(cat VERSION)}
RELEASE_NAME="chipmunk-${VERSION}"
ARCHIVE_NAME="${RELEASE_NAME}.tar.gz"

echo "Packaging release ${VERSION}..."

# Get repo root (absolute path)
REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# Ensure we're in the repo root
cd "${REPO_ROOT}"

# Clean and build
echo "Building binaries..."
make clean
make build

# Create temporary release directory
TMPDIR=$(mktemp -d)
RELEASE_DIR="${TMPDIR}/${RELEASE_NAME}"
mkdir -p "${RELEASE_DIR}"

echo "Copying files to release package..."

# Copy binaries
mkdir -p "${RELEASE_DIR}/bin"
cp bin/analog bin/diglog bin/loged bin/fixfet7 "${RELEASE_DIR}/bin/" 2>/dev/null || true

# Copy configuration files
mkdir -p "${RELEASE_DIR}/log/lib"
cp -r log/lib/* "${RELEASE_DIR}/log/lib/"

# Copy lessons
mkdir -p "${RELEASE_DIR}/lessons"
cp -r lessons/* "${RELEASE_DIR}/lessons/"

# Copy documentation
mkdir -p "${RELEASE_DIR}/docs"
cp -r docs/* "${RELEASE_DIR}/docs/" 2>/dev/null || true
cp README.md CHANGELOG.md LICENSE* "${RELEASE_DIR}/" 2>/dev/null || true

# Copy source code (for reference, but users should clone repo for full source)
mkdir -p "${RELEASE_DIR}/src"
echo "Source code available at: https://github.com/sensorsINI/chipmunk" > "${RELEASE_DIR}/src/README.txt"

# Create package
cd "${TMPDIR}"
tar czf "${ARCHIVE_NAME}" "${RELEASE_NAME}"

# Move to repo root
mv "${ARCHIVE_NAME}" "${REPO_ROOT}/"

# Cleanup
rm -rf "${TMPDIR}"

echo ""
echo "Release package created: ${REPO_ROOT}/${ARCHIVE_NAME}"
echo "Size: $(du -h "${REPO_ROOT}/${ARCHIVE_NAME}" | cut -f1)"
echo ""
echo "To create a GitHub release:"
echo "  1. Push the tag: git push origin v${VERSION}"
echo "  2. Go to: https://github.com/sensorsINI/chipmunk/releases/new"
echo "  3. Select tag: v${VERSION}"
echo "  4. Upload: ${ARCHIVE_NAME}"
echo "  5. Add release notes from CHANGELOG.md"

