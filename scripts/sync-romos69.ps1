# Sync D:\ROMEOS 69 με GitHub

param(
    [string]$Branch = "cursor/alpha-rev-a-fw-fc5f"
)

$ProjectRoot = "D:\ROMEOS 69"

Write-Host "ROMEOS 69 sync → $ProjectRoot" -ForegroundColor Cyan

if (-not (Test-Path $ProjectRoot)) {
    Write-Host "Δημιουργία φακέλου και clone..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path $ProjectRoot -Force | Out-Null
    Set-Location "D:\"
    git clone https://github.com/romeos69788/Romeo-s-69.git "ROMEOS 69"
    Set-Location $ProjectRoot
    git checkout $Branch
    Write-Host "Έτοιμο." -ForegroundColor Green
    exit 0
}

Set-Location $ProjectRoot

if (-not (Test-Path ".git")) {
    Write-Host "Προσοχή: ο φάκελος δεν είναι git repo." -ForegroundColor Red
    Write-Host "Τρέξε πρώτα (backup τοπικά αρχεία αν χρειάζεται):"
    Write-Host '  cd "D:\ROMEOS 69"'
    Write-Host '  git init'
    Write-Host '  git remote add origin https://github.com/romeos69788/Romeo-s-69.git'
    Write-Host "  git fetch origin"
    Write-Host "  git checkout -b $Branch origin/$Branch"
    exit 1
}

Write-Host "Pull branch $Branch..." -ForegroundColor Yellow
git fetch origin
git pull origin $Branch

Write-Host "Τελευταία commit:" -ForegroundColor Green
git log -1 --oneline

Write-Host ""
Write-Host "Άνοιξε στο Cursor: File → Open Folder → D:\ROMEOS 69" -ForegroundColor Cyan
