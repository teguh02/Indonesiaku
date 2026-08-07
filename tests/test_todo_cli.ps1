# Integration test for the Todo-list JSON DB CLI (examples/todo_cli.idk) - Windows.
# Runs a full CRUD sequence across separate process invocations (real
# persistence) and asserts observable output. Exits non-zero on any mismatch.

$ErrorActionPreference = "Continue"
$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

$exe = Join-Path $repo "indk.exe"
if (-Not (Test-Path $exe)) { Write-Host "indk.exe not found; build first." -ForegroundColor Red; exit 1 }

$app = "examples\todo_cli.idk"
$db = "data_todo.json"
Remove-Item $db -ErrorAction SilentlyContinue

$fail = 0
function Check($desc, $expected, $actual) {
    $a = ($actual -join "`n").Trim()
    $e = $expected.Trim()
    if ($a -eq $e) {
        Write-Host "  OK  $desc" -ForegroundColor Green
    } else {
        Write-Host "  FAIL $desc" -ForegroundColor Red
        Write-Host "       expected: [$e]"
        Write-Host "       actual:   [$a]"
        $script:fail = 1
    }
}

Write-Host "== Todo CLI CRUD integration test =="

Check "tambah 1" "Ditambahkan." (& $exe $app tambah "Beli susu" 2>&1)
Check "tambah 2" "Ditambahkan." (& $exe $app tambah "Cuci baju" 2>&1)
Check "tambah 3" "Ditambahkan." (& $exe $app tambah "Belajar" 2>&1)

Check "daftar (3 item)" "[ ] #1 Beli susu`n[ ] #2 Cuci baju`n[ ] #3 Belajar" (& $exe $app daftar 2>&1)
Check "lihat 2" "[ ] #2 Cuci baju" (& $exe $app lihat 2 2>&1)

Check "selesai 2" "Ditandai selesai." (& $exe $app selesai 2 2>&1)
Check "lihat 2 setelah selesai" "[x] #2 Cuci baju" (& $exe $app lihat 2 2>&1)

Check "ubah 1" "Diubah." (& $exe $app ubah 1 "Beli roti" 2>&1)
Check "lihat 1 setelah ubah" "[ ] #1 Beli roti" (& $exe $app lihat 1 2>&1)

Check "hapus 3" "Dihapus." (& $exe $app hapus 3 2>&1)

Check "daftar akhir (2 item)" "[ ] #1 Beli roti`n[x] #2 Cuci baju" (& $exe $app daftar 2>&1)

Check "hapus id tak ada" "Kesalahan:`ntodo dengan id 99 tidak ditemukan" (& $exe $app hapus 99 2>&1)

Remove-Item $db -ErrorAction SilentlyContinue

if ($fail -eq 0) {
    Write-Host "== ALL TODO CLI TESTS PASSED ==" -ForegroundColor Green
    exit 0
} else {
    Write-Host "== TODO CLI TESTS FAILED ==" -ForegroundColor Red
    exit 1
}
