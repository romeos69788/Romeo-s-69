# Full copy: D:\Romeos Tsakas -> D:\ROMEOS 69\docs\archive\romeos-tsakas
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File .\docs\scripts\copy-romeos-tsakas-full.ps1

param(
    [string]$SourceRoot = "D:\Romeos Tsakas",
    [string]$DestRoot   = "D:\ROMEOS 69\docs\archive\romeos-tsakas",
    [switch]$WhatIf
)

$ErrorActionPreference = "Stop"

function Write-Step([string]$msg) {
    Write-Host "[COPY] $msg" -ForegroundColor Cyan
}

if (-not (Test-Path -LiteralPath $SourceRoot)) {
    Write-Host "Not found: $SourceRoot" -ForegroundColor Red
    exit 1
}

$romeos69 = "D:\ROMEOS 69"
if (-not (Test-Path -LiteralPath $romeos69)) {
    Write-Host "Not found: $romeos69" -ForegroundColor Red
    exit 1
}

Write-Step "Source: $SourceRoot"
Write-Step "Dest:   $DestRoot"

if ($WhatIf) {
    Write-Step "DRY-RUN only"
    $files = Get-ChildItem -LiteralPath $SourceRoot -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -notmatch '\\(\.git|node_modules|\.pio)\\' }
    $files | Select-Object -First 30 | ForEach-Object { Write-Host "  WOULD: $($_.FullName)" }
    Write-Host "Total files: $($files.Count)" -ForegroundColor Yellow
    exit 0
}

if (-not (Test-Path -LiteralPath $DestRoot)) {
    New-Item -ItemType Directory -Path $DestRoot -Force | Out-Null
}

Write-Step "Copy with robocopy..."
& robocopy $SourceRoot $DestRoot /E /XD .git node_modules .pio build dist __pycache__ /XF Thumbs.db desktop.ini /R:2 /W:2 /NFL /NDL /NJH /NJS /NC /NS
$rc = $LASTEXITCODE

if ($rc -ge 8) {
    Write-Host "robocopy failed (exit $rc)" -ForegroundColor Red
    exit $rc
}

$manifestPath = Join-Path $DestRoot "COPY-MANIFEST.txt"
$lines = @(
    "Full copy manifest",
    "Source: $SourceRoot",
    "Dest:   $DestRoot",
    "Date:   $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')",
    "Robocopy exit: $rc",
    ""
)
Get-ChildItem -LiteralPath $DestRoot -Recurse -File | ForEach-Object {
    $rel = $_.FullName.Substring($DestRoot.Length).TrimStart("\")
    $lines += $rel
}
$lines | Set-Content -LiteralPath $manifestPath -Encoding UTF8

$hpSrc = Get-ChildItem -LiteralPath $DestRoot -Recurse -Filter "HP-BOARD-LEITOURGIA.md" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($hpSrc) {
    $hpDst = Join-Path $romeos69 "docs\HP-BOARD-LEITOURGIA.md"
    Copy-Item -LiteralPath $hpSrc.FullName -Destination $hpDst -Force
    Write-Step "Updated docs\HP-BOARD-LEITOURGIA.md"
}

$fileCount = (Get-ChildItem -LiteralPath $DestRoot -Recurse -File).Count
Write-Host ""
Write-Host "Done. Files in archive: $fileCount" -ForegroundColor Green
Write-Host "Manifest: $manifestPath"
Write-Host ""
Write-Host "Next:" -ForegroundColor Yellow
Write-Host "  cd D:\ROMEOS 69"
Write-Host "  git add docs\archive\romeos-tsakas"
Write-Host "  git commit -m Full-copy-Romeos-Tsakas-archive"
Write-Host "  git push origin main"
