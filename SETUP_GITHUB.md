# Setting Up GitHub Repository

This repository has been initialized locally. To push it to GitHub:

## Steps to Push to GitHub

1. **Create a new repository on GitHub**:
   - Go to https://github.com/organizations/sensorsINI/repositories/new
   - Repository name: `chipmunk`
   - Description: "Chipmunk CAD tools - fork with attribution"
   - Choose Public or Private as appropriate
   - **Do NOT** initialize with README, .gitignore, or license (we already have these)
   - Click "Create repository"

2. **Add the remote and push**:
   ```bash
   cd ~/chipmunk
   git remote add origin https://github.com/sensorsINI/chipmunk.git
   git push -u origin main
   ```

   If you're using SSH instead:
   ```bash
   git remote add origin git@github.com:sensorsINI/chipmunk.git
   git push -u origin main
   ```

3. **Verify the repository**:
   - Check that README.md displays correctly
   - Verify that COPYING files are present
   - Ensure all source files are included

## Repository Structure

- `README.md` - Main documentation with attribution
- `.gitignore` - Excludes build artifacts
- `psys/` - Psys library source code
- `log/` - Log system source code
- `bin/` - Compiled binaries and wrapper scripts
- `lib/` - Library files

## License Compliance

The repository includes:
- Original GPL license files in `psys/src/COPYING` and `log/src/COPYING`
- Proper attribution in README.md
- Link back to original source

All GPL requirements are satisfied.

