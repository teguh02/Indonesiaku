# Simple performance benchmark for Indonesiaku (Windows).
# Times compute-heavy example programs. Baseline for spotting regressions,
# not a pass/fail gate.

$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

$exe = Join-Path $repo "indk.exe"
if (-Not (Test-Path $exe)) {
    Write-Host "indk.exe not found. Build first (make)." -ForegroundColor Red
    exit 1
}

$benches = @(
    "examples\fibonacci.idk",
    "examples\recursion_deep.idk",
    "examples\performance_stress.idk",
    "examples\loop_nested.idk",
    "examples\mutual_recursion.idk"
)

Write-Host "=================================================="
Write-Host " INDONESIAKU BENCHMARK  ($(Get-Date))"
Write-Host "=================================================="
foreach ($b in $benches) {
    if (-Not (Test-Path $b)) { continue }
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    & $exe $b > $null 2>&1
    $sw.Stop()
    "{0,-38} {1,8:N3} s" -f (Split-Path $b -Leaf), ($sw.Elapsed.TotalSeconds)
}
Write-Host "=================================================="
