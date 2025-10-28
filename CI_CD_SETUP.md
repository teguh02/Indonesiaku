# 🔄 CI/CD Setup - Indonesiaku

GitHub Actions workflows untuk automated testing, building, dan releasing Indonesiaku.

---

## 📋 Workflows Overview

### 1. **Automated Tests** (`.github/workflows/test.yml`)

**Trigger**: 
- Push ke `main` atau `develop` branch
- Pull Request ke `main` atau `develop`
- Manual trigger via `workflow_dispatch`

**Jobs**:

#### 🐧 Test on Linux
- Tests pada berbagai versi GCC (11, 12, 13)
- Runs `bash tests/test_all.sh`
- Matrix build strategy

#### 🍎 Test on macOS
- Tests dengan GCC di macOS
- Runs `bash tests/test_all.sh`
- Latest macOS runner

#### 🪟 Test on Windows
- Tests menggunakan MSYS2
- Runs `powershell tests/test_all.ps1`
- Latest Windows runner

#### 📊 Code Quality Checks
- Code formatting verification (clang-format)
- Static analysis (cppcheck)
- File integrity checks

#### 📚 Documentation Verification
- Check semua doc files exist
- Verify example programs
- Validate repository structure

#### 📈 Test Results Report
- Aggregates hasil dari semua OS
- Generate summary report
- Status badge

---

### 2. **Build & Release** (`.github/workflows/release.yml`)

**Trigger**:
- Push git tag yang matches pattern `v*.*.*` (e.g., `v0.1.1`)
- Manual trigger via `workflow_dispatch`

**Jobs**:

#### 🏗️ Build Release Artifacts
- Builds binary untuk Linux, macOS, dan Windows
- Matrix build strategy (3 OS)
- Upload artifacts untuk 90 hari

#### 🐳 Docker Build & Push
- Build Docker image dengan Buildx
- Push ke Docker Hub (`teguh02/indonesiaku:latest`)
- Tag dengan version (e.g., `teguh02/indonesiaku:v0.1.1`)
- Requires Docker Hub secrets

#### 📦 Create GitHub Release
- Download build artifacts
- Create GitHub Release
- Attach binaries ke release
- Publish dengan release notes

#### 🔔 Notify Completion
- Print summary ke logs

---

## 🚀 How to Use

### Running Tests Locally

#### Linux/macOS
```bash
chmod +x tests/test_all.sh
bash tests/test_all.sh
```

#### Windows (PowerShell)
```powershell
powershell -ExecutionPolicy Bypass -File tests/test_all.ps1
```

### Triggering GitHub Actions

#### Automated Tests
Tests automatically run ketika:
- Push ke main branch
- Create pull request
- Modify source atau test files

**View results**: Go to repository → Actions tab → Select workflow run

#### Release Build
Release build automatically runs ketika:
- Create git tag: `git tag v0.1.1`
- Push tag: `git push origin v0.1.1`

**Expected output**:
- GitHub Release created dengan binaries
- Docker image pushed ke Docker Hub
- All artifacts available for download

### Manual Trigger
```bash
# Via GitHub CLI
gh workflow run test.yml -r main

gh workflow run release.yml -r main
```

---

## 🔐 Secrets Required for Release

Set these secrets di GitHub repository settings → Secrets and variables → Actions:

| Secret | Value | Required For |
|--------|-------|--------------|
| `DOCKER_USERNAME` | Your Docker Hub username | Docker Hub push |
| `DOCKER_PASSWORD` | Your Docker Hub token/password | Docker Hub push |

### How to Set Secrets

1. Go to: `https://github.com/teguh02/Indonesiaku/settings/secrets/actions`
2. Click "New repository secret"
3. Name: `DOCKER_USERNAME`, Value: `your-dockerhub-username`
4. Click "Add secret"
5. Repeat for `DOCKER_PASSWORD`

---

## 📊 Workflow Files Location

```
.github/
└── workflows/
    ├── test.yml       ← Automated testing on push/PR
    └── release.yml    ← Build & release on tag push
```

---

## ✅ Test Coverage

### Automated Tests Check

- ✅ 6 Basic tests (hello, variabel, fungsi, etc)
- ✅ 9 Advanced tests (recursion, pattern generation, etc)
- ✅ 5 OOP tests (closure, inheritance simulation)
- ✅ Code formatting
- ✅ Static analysis
- ✅ Documentation completeness

### Build Targets

- ✅ Linux (x64)
- ✅ macOS (x64)
- ✅ Windows (x64)

---

## 🔍 Workflow Details

### Test Workflow Structure

```yaml
on:
  push:
    branches: [main, develop]
    paths: [src/**, Makefile, tests/**]
  pull_request:
    branches: [main, develop]
  workflow_dispatch:  # Manual trigger

jobs:
  test-linux:
    runs-on: ubuntu-latest
    strategy:
      matrix:
        gcc-version: ['11', '12', '13']
    steps:
      - checkout
      - install GCC
      - build
      - run tests
      
  test-macos:
    runs-on: macos-latest
    # ... similar steps
    
  test-windows:
    runs-on: windows-latest
    # ... similar steps
```

### Release Workflow Structure

```yaml
on:
  push:
    tags: ['v*.*.*']  # Triggered on tags like v0.1.1
  workflow_dispatch:

jobs:
  build-artifacts:
    strategy:
      matrix:
        include:
          - os: ubuntu-latest
          - os: macos-latest
          - os: windows-latest
    # ... build on each OS
    
  docker-build-push:
    # ... build & push to Docker Hub
    
  create-release:
    # ... create GitHub Release with artifacts
```

---

## 📈 Monitoring Builds

### View Test Results

1. Push ke main branch
2. Go to repo → **Actions** tab
3. Click "Automated Tests" workflow
4. See test results untuk semua OS
5. Download logs jika perlu

### View Release Status

1. Create dan push git tag: `git tag v0.1.1 && git push origin v0.1.1`
2. Go to **Actions** tab
3. Click "Build & Release" workflow
4. See build progress

---

## 🐛 Troubleshooting

### Tests Fail on GitHub but Pass Locally

**Cause**: Environment differences

**Solution**:
```bash
# Try to replicate CI environment
make clean
make
./indk examples/hello.idk

# Check build flags
gcc --version
make --version
```

### Docker Push Fails

**Cause**: Missing or invalid Docker Hub secrets

**Solution**:
1. Verify `DOCKER_USERNAME` dan `DOCKER_PASSWORD` are set
2. Check Docker Hub credentials are correct
3. Ensure account has push permissions

### Release Artifacts Missing

**Cause**: Build failed on one or more OS

**Solution**:
1. Check workflow logs
2. See which OS failed
3. Debug locally with `make clean && make`

---

## 🚀 Creating a Release

### Step-by-Step

```bash
# 1. Update version in code
# Edit src/common.h, update INDK_VERSION

# 2. Commit changes
git add .
git commit -m "chore: bump version to v0.2.0"

# 3. Create annotated tag
git tag -a v0.2.0 -m "Release version 0.2.0"

# 4. Push tag to GitHub
git push origin v0.2.0

# 5. Wait for GitHub Actions
# Go to https://github.com/teguh02/Indonesiaku/actions
# Watch "Build & Release" workflow

# 6. Release appears at:
# https://github.com/teguh02/Indonesiaku/releases/tag/v0.2.0
```

---

## 📝 Example Outputs

### Successful Test Run

```
Run Linux Tests
✓ hello.idk OK
✓ variabel.idk OK
...
TOTAL: 19/20 PASSED
SUCCESS: ALL TESTS PASSING!
```

### Successful Release

```
Build & Release workflow completed
✅ Linux binary: indk-linux (uploaded)
✅ macOS binary: indk-macos (uploaded)
✅ Windows binary: indk-windows.exe (uploaded)
✅ Docker image: teguh02/indonesiaku:v0.2.0 (pushed)
✅ GitHub Release created with artifacts
```

---

## 📚 Additional Resources

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [Workflow Syntax](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)
- [Using Secrets](https://docs.github.com/en/actions/security-guides/using-secrets-in-github-actions)
- [Matrix Builds](https://docs.github.com/en/actions/using-jobs/using-a-matrix-for-your-jobs)

---

## ✨ Future Enhancements

- [ ] Add coverage reporting
- [ ] Add performance benchmarking
- [ ] Add automatic changelog generation
- [ ] Add semantic versioning automation
- [ ] Add Slack notifications
- [ ] Add code review automation

---

**CI/CD Setup Complete!** 🎉

All workflows are now ready to automatically test, build, and release Indonesiaku.

🇮🇩 **Happy automation!** 🚀
