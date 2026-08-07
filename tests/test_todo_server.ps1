# Integration test for the Todo-list REST HTTP server (Windows).
# Starts the server, drives it with curl over HTTP, asserts JSON responses,
# then shuts it down via /berhenti. Exits non-zero on any mismatch.

$ErrorActionPreference = "Continue"
$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

$exe = Join-Path $repo "indk.exe"
if (-Not (Test-Path $exe)) { Write-Host "indk.exe not found; build first." -ForegroundColor Red; exit 1 }

$port = 8138
$base = "http://127.0.0.1:$port"
$db = "data_todo_server.json"
Remove-Item $db -ErrorAction SilentlyContinue

$job = Start-Job -ScriptBlock {
    Set-Location $using:repo
    & '.\indk.exe' 'examples\todo_server.idk' $using:port 2>&1
}

function Cleanup {
    try { & curl.exe -s "$base/berhenti" > $null 2>&1 } catch {}
    Start-Sleep -Milliseconds 300
    Stop-Job $job -ErrorAction SilentlyContinue
    Remove-Job $job -Force -ErrorAction SilentlyContinue
    Remove-Item $db -ErrorAction SilentlyContinue
}

# Wait for readiness (up to ~5s).
$ready = $false
for ($i = 0; $i -lt 50; $i++) {
    try { & curl.exe -s "$base/todos" > $null 2>&1; if ($LASTEXITCODE -eq 0) { $ready = $true; break } } catch {}
    Start-Sleep -Milliseconds 100
}
if (-Not $ready) { Write-Host "Server tidak siap" -ForegroundColor Red; Cleanup; exit 1 }

$fail = 0
function Check($desc, $expected, $actual) {
    $a = ($actual | Out-String).Trim()
    if ($a -eq $expected) {
        Write-Host "  OK  $desc" -ForegroundColor Green
    } else {
        Write-Host "  FAIL $desc" -ForegroundColor Red
        Write-Host "       expected: [$expected]"
        Write-Host "       actual:   [$a]"
        $script:fail = 1
    }
}

Write-Host "== Todo REST server integration test =="

# Use temp files for JSON bodies to avoid PowerShell/curl quoting issues.
$body1 = New-TemporaryFile
$body2 = New-TemporaryFile
$bodyPut = New-TemporaryFile
Set-Content -Path $body1 -Value '{"teks":"Beli susu"}' -NoNewline -Encoding ascii
Set-Content -Path $body2 -Value '{"teks":"Cuci baju"}' -NoNewline -Encoding ascii
Set-Content -Path $bodyPut -Value '{"selesai":true}' -NoNewline -Encoding ascii

Check "GET /todos (kosong)" "[]" (& curl.exe -s "$base/todos")
Check "POST #1" '{"id":1,"teks":"Beli susu","selesai":false}' (& curl.exe -s -X POST --data-binary "@$body1" "$base/todos")
Check "POST #2" '{"id":2,"teks":"Cuci baju","selesai":false}' (& curl.exe -s -X POST --data-binary "@$body2" "$base/todos")
Check "GET /todos (2 item)" '[{"id":1,"teks":"Beli susu","selesai":false},{"id":2,"teks":"Cuci baju","selesai":false}]' (& curl.exe -s "$base/todos")
Check "GET /todos/1" '{"id":1,"teks":"Beli susu","selesai":false}' (& curl.exe -s "$base/todos/1")
Check "PUT /todos/1 (selesai)" '{"id":1,"teks":"Beli susu","selesai":true}' (& curl.exe -s -X PUT --data-binary "@$bodyPut" "$base/todos/1")
Check "DELETE /todos/2" '{"pesan":"dihapus"}' (& curl.exe -s -X DELETE "$base/todos/2")
Check "GET /todos (final)" '[{"id":1,"teks":"Beli susu","selesai":true}]' (& curl.exe -s "$base/todos")
Check "GET /todos/99 (404)" '{"kesalahan":"tidak ditemukan"}' (& curl.exe -s "$base/todos/99")

Remove-Item $body1, $body2, $bodyPut -ErrorAction SilentlyContinue

Cleanup

if ($fail -eq 0) {
    Write-Host "== ALL TODO SERVER TESTS PASSED ==" -ForegroundColor Green
    exit 0
} else {
    Write-Host "== TODO SERVER TESTS FAILED ==" -ForegroundColor Red
    exit 1
}
