# Πλήρης αντιγραφή D:\Romeos Tsakas → D:\ROMEOS 69\docs\archive\romeos-tsakas\
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File .\docs\scripts\copy-romeos-tsakas-full.ps1
#
# Dry-run (λίστα μόνο):
#   powershell -ExecutionPolicy Bypass -File .\docs\scripts\copy-romeos-tsakas-full.ps1 -WhatIf

param(
    [string]$SourceRoot = "D:\Romeos Tsakas",
    [string]$DestRoot   = "D:\ROMEOS 69\docs\archive\romeos-tsakas",
    [switch]$WhatIf
)

$ErrorActionPreference = "Stop"

function Write-Step($msg) { Write-Host "[COPY] $msg" -ForegroundColor Cyan }

if (-not (Test-Path $SourceRoot)) {
    Write-Host "Δεν βρέθηκε: $SourceRoot" -ForegroundColor Red
    exit 1
}

$romeos69 = "D:\ROMEOS 69"
if (-not (Test-Path $romeos69)) {
    Write-Host "Δεν βρέθηκε: $romeos69" -ForegroundColor Red
    exit 1
}

Write-Step "Πηγή:      $SourceRoot"
Write-Step "Προορισμός: $DestRoot"

if ($WhatIf) {
    Write-Step "DRY-RUN — δεν γίνεται copy"
    Get-ChildItem -Path $SourceRoot -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -notmatch '\\(\.git|node_modules|\.pio)\\' } |
        Select-Object -First 50 FullName |
        ForEach-Object { Write-Host "  WOULD: $($_.FullName)" }
    $total = (Get-ChildItem -Path $SourceRoot -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -notmatch '\\(\.git|node_modules|\.pio)\\' }).Count
    Write-Host "Σύνολο αρχείων (εκτός .git/node_modules/.pio): $total" -ForegroundColor Yellow
    exit 0
}

if (-not (Test-Path $DestRoot)) {
    New-Item -ItemType Directory -Path $DestRoot -Force | Out-Null
}

# robocopy: /E=subdirs incl empty, /XD=exclude dirs, /XF=exclude files, /R:2 /W:2 retries
# Exit codes 0-7 = success for robocopy
$robocopyArgs = @(
    $SourceRoot,
    $DestRoot,
    "/E",
    "/XD", ".git", "node_modules", ".pio", "build", "dist", "__pycache__",
    "/XF", "Thumbs.db", "desktop.ini",
    "/R:2",
    "/W:2",
    "/NFL",
    "/NDL",
    "/NJH",
    "/NJS",
    "/NC",
    "/NS"
)

Write-Step "Αντιγραφή με robocopy..."
& robocopy @robocopyArgs
$rc = $LASTEXITCODE

if ($rc -ge 8) {
    Write-Host "robocopy απέτυχε (exit $rc)" -ForegroundColor Red
    exit $rc
}

# Manifest
$manifest = @(
    "Full copy manifest",
    "Source: $SourceRoot",
    "Dest:   $DestRoot",
    "Date:   $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')",
    "Robocopy exit: $rc",
    ""
)

Get-ChildItem -Path $DestRoot -Recurse -File | ForEach-Object {
    $rel = $_.FullName.Substring($DestRoot.Length).TrimStart("\")
    $manifest += $rel
}

$manifestPath = Join-Path $DestRoot "COPY-MANIFEST.txt"
$manifest | Set-Content -Path $manifestPath -Encoding UTF8

# Ενημέρωση HP-BOARD αν υπάρχει στο archive
$hpSrc = Get-ChildItem -Path $DestRoot -Recurse -Filter "HP-BOARD-LEITOURGIA.md" -ErrorAction SilentlyContinue | Select-Object -First 1
if ($hpSrc) {
    $hpDst = Join-Path $romeos69 "docs\HP-BOARD-LEITOURGIA.md"
    Copy-Item -Path $hpSrc.FullName -Destination $hpDst -Force
    Write-Step "Ενημερώθηκε docs\HP-BOARD-LEITOURGIA.md"
}

$fileCount = (Get-ChildItem -Path $DestRoot -Recurse -File).Count
Write-Host ""
Write-Host "Ολοκληρώθηκε. Αρχεία στο archive: $fileCount" -ForegroundColor Green
Write-Host "Manifest: $manifestPath"
Write-Host ""
Write-Host "Επόμενο:" -ForegroundColor Yellow
Write-Host '  cd "D:\ROMEOS 69"'
Write-Host '  git add docs\archive\romeos-tsakas'
Write-Host '  git commit -m "Full copy D:\Romeos Tsakas archive"'
Write-Host '  git push'
