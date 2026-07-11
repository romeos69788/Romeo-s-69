# Fix failed push: remove large zip/rar from last commit and push again
# Run: powershell -ExecutionPolicy Bypass -File .\docs\scripts\fix-push-large-files.ps1

$ErrorActionPreference = "Stop"
Set-Location "D:\ROMEOS 69"

Write-Host "[FIX] Undo last commit (keep files on disk)..." -ForegroundColor Cyan
git reset --soft HEAD~1

Write-Host "[FIX] Unstage large archives..." -ForegroundColor Cyan
$patterns = @("*.zip", "*.rar", "*.7z", "*.iso")
Get-ChildItem docs\archive\romeos-tsakas -Recurse -File -ErrorAction SilentlyContinue |
    Where-Object { $patterns -contains $_.Extension.ToLower() } |
    ForEach-Object {
        git reset HEAD $_.FullName 2>$null
        Write-Host "  skip: $($_.Name)"
    }

git pull origin main

Write-Host "[FIX] Commit without large files..." -ForegroundColor Cyan
git add .gitignore docs\archive\romeos-tsakas
git status --short | Select-Object -First 20
git commit -m "Full copy Romeos Tsakas archive (exclude zip/rar over GitHub limit)"

Write-Host "[FIX] Push..." -ForegroundColor Cyan
git push origin main

Write-Host ""
Write-Host "Done." -ForegroundColor Green
git log -1 --oneline
$fileCount = (Get-ChildItem docs\archive\romeos-tsakas -Recurse -File).Count
Write-Host "Local archive files (incl zip/rar): $fileCount"
Write-Host "Large zip/rar stay LOCAL only - not on GitHub."
