# Script untuk testing output spesifik
# File: tests/test_output.ps1

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "   TESTING OUTPUT SPESIFIK INDONESIAKU" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Expected outputs untuk setiap file
$expected = @{
    "hello.idk" = "Halo, dunia!"
    "variabel.idk" = "x \+ y ="
    "fibonacci.idk" = "Fibonacci"
    "prima.idk" = "adalah bilangan prima"
    "kalkulator.idk" = "Kalkulator Sederhana"
}

$totalTests = $expected.Count
$passedTests = 0
$failedTests = 0

foreach ($file in $expected.Keys) {
    $filePath = ".\examples\$file"
    
    if (-Not (Test-Path $filePath)) {
        Write-Host "⚠️  $file tidak ditemukan, skip..." -ForegroundColor Yellow
        continue
    }
    
    Write-Host "Testing: $file" -ForegroundColor Cyan
    
    try {
        $output = & .\indk.exe $filePath 2>&1 | Out-String
        
        if ($output -match $expected[$file]) {
            Write-Host "  ✅ OK - Output sesuai harapan" -ForegroundColor Green
            $passedTests++
        } else {
            Write-Host "  ❌ GAGAL - Output tidak sesuai" -ForegroundColor Red
            Write-Host "  Expected pattern: $($expected[$file])" -ForegroundColor Yellow
            Write-Host "  Actual output:" -ForegroundColor Yellow
            Write-Host $output.Substring(0, [Math]::Min(100, $output.Length)) -ForegroundColor Gray
            $failedTests++
        }
    }
    catch {
        Write-Host "  ❌ ERROR: $_" -ForegroundColor Red
        $failedTests++
    }
    
    Write-Host ""
}

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "Passed: $passedTests / $totalTests" -ForegroundColor White
Write-Host "=============================================" -ForegroundColor Cyan

if ($failedTests -eq 0) {
    Write-Host "🎉 SEMUA OUTPUT TEST BERHASIL!" -ForegroundColor Green
    exit 0
} else {
    exit 1
}
