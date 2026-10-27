# Modul pustaka/ harus ditemukan dari LOKASI INTERPRETER, bukan dari
# direktori kerja. Padanan Windows untuk tests/test_impor_lokasi.sh: indk.exe
# disalin ke tiga tata letak instalasi, lalu program dijalankan dari direktori
# kerja lain tanpa INDK_PATH. Kontrol negatif: tanpa pustaka/, impor gagal.

$ErrorActionPreference = "Stop"
$repo = Split-Path -Parent $PSScriptRoot
$bin = Join-Path $repo "indk.exe"
if (-not (Test-Path $bin)) {
    Write-Host "indk.exe belum dibangun." -ForegroundColor Red
    exit 1
}

$t = Join-Path ([System.IO.Path]::GetTempPath()) ("indk-lokasi-" + [System.Guid]::NewGuid().ToString("N"))
$kerja = Join-Path $t "kerja"
New-Item -ItemType Directory -Force -Path $kerja | Out-Null

$program = @'
impor pola
dari koleksi impor Penghitung
impor "pustaka/teks.idk"
impor jalur
cetak(pola.cari_semua("\\d+", "a1b22"))
cetak(Penghitung("aab")["a"])
cetak(teks_balik("abc"))
cetak(jalur.ASAL)
'@
[System.IO.File]::WriteAllText((Join-Path $kerja "program.idk"), $program)
[System.IO.File]::WriteAllText((Join-Path $kerja "jalur.idk"), "ASAL = `"lokal`"`n")
[System.IO.File]::WriteAllText((Join-Path $kerja "tanpa.idk"), "impor pola`n")
$harap = "[`"1`", `"22`"]`n2`ncba`nlokal"

$simpanPath = $env:INDK_PATH
Remove-Item Env:INDK_PATH -ErrorAction SilentlyContinue
$gagal = 0

function Jalankan($exe, $skrip) {
    # PowerShell 5.1 turns a native program's stderr into an error record;
    # with "Stop" that would abort the negative control, whose point IS stderr.
    $ErrorActionPreference = "Continue"
    Push-Location $kerja
    try {
        $keluar = & $exe $skrip 2>&1 | Out-String
    } finally {
        Pop-Location
    }
    return ($keluar -replace "`r`n", "`n").TrimEnd("`n")
}

function Uji($nama, $dirBin, $dirPustaka) {
    New-Item -ItemType Directory -Force -Path $dirBin | Out-Null
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dirPustaka) | Out-Null
    Copy-Item $bin (Join-Path $dirBin "indk.exe")
    Copy-Item (Join-Path $repo "pustaka") $dirPustaka -Recurse
    $keluar = Jalankan (Join-Path $dirBin "indk.exe") "program.idk"
    if ($keluar -eq $harap) {
        Write-Host "  ok   $nama" -ForegroundColor Green
    } else {
        Write-Host "  GAGAL $nama" -ForegroundColor Red
        Write-Host $keluar
        $script:gagal = 1
    }
}

try {
    Write-Host "Impor pustaka/ dari lokasi interpreter:"
    Uji "di sebelah binari (build repo)" (Join-Path $t "repo") (Join-Path $t "repo\pustaka")
    Uji "bin\ + pustaka\ (installer)" (Join-Path $t "app\bin") (Join-Path $t "app\pustaka")
    Uji "bin\ + share\indonesiaku\pustaka" (Join-Path $t "prefix\bin") (Join-Path $t "prefix\share\indonesiaku\pustaka")

    $tanpa = Join-Path $t "tanpa"
    New-Item -ItemType Directory -Force -Path $tanpa | Out-Null
    Copy-Item $bin (Join-Path $tanpa "indk.exe")
    $keluar = Jalankan (Join-Path $tanpa "indk.exe") "tanpa.idk"
    if ($keluar -match "Modul 'pola' tidak ditemukan") {
        Write-Host "  ok   tanpa pustaka\: impor gagal seperti seharusnya" -ForegroundColor Green
    } else {
        Write-Host "  GAGAL kontrol negatif: $keluar" -ForegroundColor Red
        $gagal = 1
    }
} finally {
    if ($null -ne $simpanPath) { $env:INDK_PATH = $simpanPath }
    Remove-Item -Recurse -Force $t -ErrorAction SilentlyContinue
}
exit $gagal
