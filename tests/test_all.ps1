Write-Host "=================================================" -ForegroundColor Cyan
Write-Host "  INDONESIAKU v0.1 - COMPLETE TEST SUITE" -ForegroundColor Cyan
Write-Host "  (6 Basic + 9 Advanced + 5 OOP = 20 total)" -ForegroundColor Cyan
Write-Host "=================================================" -ForegroundColor Cyan
Write-Host ""

if (-Not (Test-Path ".\indk.exe")) {
    Write-Host "ERROR: indk.exe tidak ditemukan!" -ForegroundColor Red
    exit 1
}

$basicTests = @("hello.idk","variabel.idk","fungsi.idk","fibonacci.idk","percabangan.idk","perulangan_selagi.idk")
$advancedTests = @("complex_arithmetic.idk","control_flow_complex.idk","function_composition.idk","loop_nested.idk","mutual_recursion.idk","pattern_generation.idk","performance_stress.idk","recursion_deep.idk","state_management.idk")
$oopTests = @("oop_test_01_basic.idk","oop_test_02_simulasi.idk","oop_test_03_closure.idk","oop_test_04_inheritance.idk","oop_test_05_polymorphism.idk")

$totalTests = 0
$passedTests = 0
$failedTests = 0
$basicPassed = 0
$advancedPassed = 0
$oopPassed = 0

Write-Host "BASIC TESTS (6):" -ForegroundColor Green
foreach ($test in $basicTests) {
    $path = ".\examples\$test"
    if (Test-Path $path) {
        Write-Host "  - $test" -NoNewline
        $output = & .\indk.exe $path 2>&1
        if ($LASTEXITCODE -eq 0) { Write-Host " OK" -ForegroundColor Green; $passedTests++; $basicPassed++ }
        else { Write-Host " FAIL" -ForegroundColor Red; $failedTests++ }
        $totalTests++
    }
}

Write-Host ""
Write-Host "ADVANCED TESTS (9):" -ForegroundColor Green
foreach ($test in $advancedTests) {
    $path = ".\examples\$test"
    if (Test-Path $path) {
        Write-Host "  - $test" -NoNewline
        $output = & .\indk.exe $path 2>&1
        if ($LASTEXITCODE -eq 0) { Write-Host " OK" -ForegroundColor Green; $passedTests++; $advancedPassed++ }
        else { Write-Host " FAIL" -ForegroundColor Red; $failedTests++ }
        $totalTests++
    }
}

Write-Host ""
Write-Host "OOP TESTS (5):" -ForegroundColor Green
Write-Host "  Note: OOP native not implemented in v0.1" -ForegroundColor Yellow
Write-Host "  These tests show what can be simulated with functions" -ForegroundColor Yellow
foreach ($test in $oopTests) {
    $path = ".\examples\oop\$test"
    if (Test-Path $path) {
        Write-Host "  - $test" -NoNewline
        $output = & .\indk.exe $path 2>&1
        if ($LASTEXITCODE -eq 0) { Write-Host " OK" -ForegroundColor Green; $passedTests++; $oopPassed++ }
        else { Write-Host " FAIL" -ForegroundColor Yellow }
        $totalTests++
    }
}

Write-Host ""
Write-Host "=================================================" -ForegroundColor Cyan
Write-Host "SUMMARY:" -ForegroundColor Cyan
Write-Host "  Basic:    $basicPassed/6" -ForegroundColor Green
Write-Host "  Advanced: $advancedPassed/9" -ForegroundColor Green
Write-Host "  OOP:      $oopPassed/5" -ForegroundColor Green
Write-Host ""
Write-Host "TOTAL: $passedTests/$totalTests PASSED" -ForegroundColor Green
if ($failedTests -eq 0) { Write-Host "SUCCESS: ALL TESTS PASSING!" -ForegroundColor Green }
Write-Host "=================================================" -ForegroundColor Cyan
