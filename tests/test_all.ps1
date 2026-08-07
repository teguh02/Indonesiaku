# =================================================
#  INDONESIAKU - GOLDEN OUTPUT TEST SUITE (Windows)
#  Runs every example and asserts stdout matches
#  tests/expected/<name>.out. Exits non-zero on any
#  failure so CI can gate on it.
# =================================================

# Native stderr from indk.exe must not abort the runner.
$ErrorActionPreference = "Continue"

# Resolve repo root (parent of this script's dir)
$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

$exe = Join-Path $repo "indk.exe"
if (-Not (Test-Path $exe)) {
    Write-Host "ERROR: indk.exe not found. Build first (make)." -ForegroundColor Red
    exit 1
}

$expectedDir = Join-Path $repo "tests\expected"
if (-Not (Test-Path $expectedDir)) {
    Write-Host "ERROR: tests\expected not found." -ForegroundColor Red
    exit 1
}

# Map example base name -> source path
$sources = @{}
foreach ($f in Get-ChildItem -Path (Join-Path $repo "examples") -Recurse -Filter *.idk) {
    $sources[$f.BaseName] = $f.FullName
}

$total = 0; $passed = 0; $failed = 0
$failures = @()

Write-Host "=================================================" -ForegroundColor Cyan
Write-Host "  INDONESIAKU - GOLDEN OUTPUT TESTS" -ForegroundColor Cyan
Write-Host "=================================================" -ForegroundColor Cyan

foreach ($exp in Get-ChildItem -Path $expectedDir -Filter *.out | Sort-Object Name) {
    $name = $exp.BaseName
    $total++
    if (-Not $sources.ContainsKey($name)) {
        Write-Host "  x $name  (no matching .idk source)" -ForegroundColor Red
        $failed++; $failures += $name; continue
    }

    $outFile = [System.IO.Path]::GetTempFileName()
    $errFile = [System.IO.Path]::GetTempFileName()
    $proc = Start-Process -FilePath $exe -ArgumentList $sources[$name] -NoNewWindow -Wait -PassThru `
        -RedirectStandardOutput $outFile -RedirectStandardError $errFile
    $code = $proc.ExitCode
    $actual = Get-Content -Raw -Path $outFile -ErrorAction SilentlyContinue
    $stderr = Get-Content -Raw -Path $errFile -ErrorAction SilentlyContinue
    Remove-Item $outFile, $errFile -Force -ErrorAction SilentlyContinue
    if ($null -eq $actual) { $actual = "" }
    $expectedText = Get-Content -Raw -Path $exp.FullName
    if ($null -eq $expectedText) { $expectedText = "" }

    # Normalize line endings for comparison
    $a = ($actual -replace "`r`n", "`n").TrimEnd("`n")
    $e = ($expectedText -replace "`r`n", "`n").TrimEnd("`n")

    if ($code -eq 0 -and $a -eq $e -and [string]::IsNullOrEmpty($stderr)) {
        Write-Host "  + $name" -ForegroundColor Green
        $passed++
    } else {
        $reason = if ($code -ne 0) { "exit=$code" }
                  elseif (-not [string]::IsNullOrEmpty($stderr)) { "stderr: $($stderr.Trim())" }
                  else { "output mismatch" }
        Write-Host "  x $name  ($reason)" -ForegroundColor Red
        $failed++; $failures += $name
    }
}

Write-Host "=================================================" -ForegroundColor Cyan
Write-Host "TOTAL: $passed/$total PASSED" -ForegroundColor Cyan
if ($failed -eq 0) {
    Write-Host "SUCCESS: ALL TESTS PASSING!" -ForegroundColor Green
    exit 0
} else {
    Write-Host "FAILURE: $failed test(s) failed:" -ForegroundColor Red
    $failures | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
    exit 1
}
