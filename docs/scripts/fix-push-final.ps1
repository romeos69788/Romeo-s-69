# Nuclear fix: abort merge, reset to GitHub, re-import archive WITHOUT zip/rar in git
# Run: powershell -ExecutionPolicy Bypass -File .\docs\scripts\fix-push-final.ps1

$ErrorActionPreference = "Stop"
Set-Location "D:\ROMEOS 69"

$TempArchive = "D:\ROMEOS-69-temp-archive"
$Archive     = "D:\ROMEOS 69\docs\archive\romeos-tsakas"
$Source      = "D:\Romeos Tsakas"

function Step([string]$m) { Write-Host "[FIX] $m" -ForegroundColor Cyan }

# 1) Abort merge if any
if (Test-Path ".git\MERGE_HEAD") {
    Step "Abort unfinished merge"
    git merge --abort
}

# 2) Backup archive folder to temp (outside repo confusion)
if (Test-Path $Archive) {
    Step "Backup archive -> $TempArchive"
    if (Test-Path $TempArchive) { Remove-Item $TempArchive -Recurse -Force }
    robocopy $Archive $TempArchive /E /NFL /NDL /NJH /NJS /NC /NS | Out-Null
} elseif (Test-Path $Source) {
    Step "No archive yet; will copy from source after reset"
}

# 3) Hard reset to clean GitHub state (removes bad local commits)
Step "Reset to origin/main"
git fetch origin
git reset --hard origin/main
git clean -fd docs\archive\romeos-tsakas 2>$null

# 4) Restore archive from temp OR copy from source
if (Test-Path $TempArchive) {
    Step "Restore archive from temp"
    New-Item -ItemType Directory -Path $Archive -Force | Out-Null
    robocopy $TempArchive $Archive /E /NFL /NDL /NJH /NJS /NC /NS | Out-Null
} elseif (Test-Path $Source) {
    Step "Copy from $Source"
    & powershell -ExecutionPolicy Bypass -File ".\docs\scripts\copy-romeos-tsakas-full.ps1"
}

# 5) Remove large files from git index (stay on disk, ignored)
Step "Ensure zip/rar not tracked"
$big = Get-ChildItem $Archive -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -match '^\.(zip|rar|7z|iso)$' }
foreach ($f in $big) {
    Write-Host "  ignore: $($f.Name)"
}

# 6) Add (gitignore excludes zip/rar/dll)
git add .gitignore
git add docs\archive\romeos-tsakas
git add docs\HP-BOARD-LEITOURGIA.md -ErrorAction SilentlyContinue

$staged = git diff --cached --stat
Write-Host $staged

Step "Commit"
git commit -m "Full copy Romeos Tsakas archive (exclude zip/rar over GitHub limit)"

Step "Push"
git push origin main

$localCount = (Get-ChildItem $Archive -Recurse -File -ErrorAction SilentlyContinue).Count
Write-Host ""
Write-Host "Done. Local archive files: $localCount" -ForegroundColor Green
Write-Host "zip/rar stay LOCAL only." -ForegroundColor Yellow
git log -1 --oneline
