# One-shot recovery: clean clone + copy Romeos Tsakas + push
# Run from ANY folder (uses D:\ paths):
#   powershell -ExecutionPolicy Bypass -File "D:\ROMEOS 69\docs\scripts\reset-and-import.ps1"
#
# If current repo is broken, run after downloading this file from GitHub,
# OR copy-paste the commands from docs/FIX-GIT-MERGE.md

$ErrorActionPreference = "Stop"
$RepoUrl = "https://github.com/romeos69788/Romeo-s-69.git"
$Target  = "D:\ROMEOS 69"
$Source  = "D:\Romeos Tsakas"
$Backup  = "D:\ROMEOS 69-backup-" + (Get-Date -Format "yyyyMMdd-HHmm")

function Step([string]$msg) { Write-Host "[RESET] $msg" -ForegroundColor Cyan }

if (-not (Test-Path -LiteralPath $Source)) {
    Write-Host "ERROR: Source not found: $Source" -ForegroundColor Red
    Write-Host "Create or fix path to Romeos Tsakas folder first."
    exit 1
}

$srcCount = (Get-ChildItem -LiteralPath $Source -Recurse -File -ErrorAction SilentlyContinue).Count
Write-Host "Source files in '$Source': $srcCount" -ForegroundColor Yellow
if ($srcCount -lt 2) {
    Write-Host "WARNING: Source folder looks empty. Continue anyway? (copy may do nothing)" -ForegroundColor Yellow
}

if (Test-Path -LiteralPath $Target) {
    Step "Backup broken/old folder -> $Backup"
    if (Test-Path -LiteralPath $Backup) {
        Remove-Item -LiteralPath $Backup -Recurse -Force
    }
    Rename-Item -LiteralPath $Target -NewName (Split-Path $Backup -Leaf)
}

Step "Clone $RepoUrl"
Set-Location D:\
git clone $RepoUrl "ROMEOS 69"
Set-Location $Target

git remote -v

Step "Run copy script"
& powershell -ExecutionPolicy Bypass -File ".\docs\scripts\copy-romeos-tsakas-full.ps1"

$archive = Join-Path $Target "docs\archive\romeos-tsakas"
$count = (Get-ChildItem -LiteralPath $archive -Recurse -File -ErrorAction SilentlyContinue).Count
Write-Host "Archive files: $count" -ForegroundColor $(if ($count -gt 5) { "Green" } else { "Red" })

if ($count -lt 2) {
    Write-Host "ERROR: Copy failed or source empty. Check D:\Romeos Tsakas" -ForegroundColor Red
    exit 1
}

Step "Git commit and push"
git add docs\archive\romeos-tsakas
if (Test-Path "docs\HP-BOARD-LEITOURGIA.md") { git add docs\HP-BOARD-LEITOURGIA.md }
git commit -m "Full copy Romeos Tsakas archive"
git push origin main

Write-Host ""
Write-Host "DONE. Backup of old folder: $Backup" -ForegroundColor Green
git log -1 --oneline
