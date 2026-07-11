# Import script: D:\Romeos Tsakas → D:\ROMEOS 69
# Usage: .\import-from-romeos-tsakas.ps1 [-WhatIf]

param(
    [string]$SourceRoot = "D:\Romeos Tsakas",
    [string]$DestRoot = "D:\ROMEOS 69",
    [switch]$WhatIf
)

$ErrorActionPreference = "Stop"

function Write-Step($msg) { Write-Host "[ROMEOS] $msg" -ForegroundColor Cyan }

if (-not (Test-Path $SourceRoot)) {
    Write-Host "Δεν βρέθηκε: $SourceRoot" -ForegroundColor Red
    Write-Host "Άλλαξε -SourceRoot ή βεβαιώσου ότι ο δίσκος D: είναι διαθέσιμος."
    exit 1
}

if (-not (Test-Path $DestRoot)) {
    Write-Step "Δημιουργία $DestRoot"
    if (-not $WhatIf) { New-Item -ItemType Directory -Path $DestRoot -Force | Out-Null }
}

$dirs = @(
    "docs\archive\romeos-tsakas",
    "docs\hardware\fan-motor",
    "docs\schematics",
    "docs\conversations",
    "secrets"
)
foreach ($d in $dirs) {
    $p = Join-Path $DestRoot $d
    if (-not (Test-Path $p) -and -not $WhatIf) {
        New-Item -ItemType Directory -Path $p -Force | Out-Null
    }
}

# Known single files (exact mapping)
$KnownFiles = @(
    @{
        Src  = "κατασκευή αντλίας θερμότητας νερού νερού\HP-BOARD-LEITOURGIA.md"
        Dest = "docs\HP-BOARD-LEITOURGIA.md"
    }
)

$manifest = @()
$manifest += "ROMEOS 69 import manifest"
$manifest += "Source: $SourceRoot"
$manifest += "Dest:   $DestRoot"
$manifest += "Date:   $(Get-Date -Format 'yyyy-MM-dd HH:mm')"
$manifest += ""

$copied = 0
$skipped = 0

foreach ($map in $KnownFiles) {
    $src = Join-Path $SourceRoot $map.Src
    $dst = Join-Path $DestRoot $map.Dest
    if (Test-Path $src) {
        Write-Step "Known: $($map.Src) -> $($map.Dest)"
        if (-not $WhatIf) {
            $dstDir = Split-Path $dst -Parent
            if (-not (Test-Path $dstDir)) { New-Item -ItemType Directory -Path $dstDir -Force | Out-Null }
            Copy-Item -Path $src -Destination $dst -Force
        }
        $manifest += "OK  $($map.Src) -> $($map.Dest)"
        $copied++
    } else {
        $manifest += "MISS $($map.Src)"
        $skipped++
    }
}

# Extensions to collect
$extensions = @("*.md", "*.txt", "*.pdf", "*.kicad_sch", "*.kicad_pcb", "*.sch", "*.brd", "*.pcb", "*.csv", "*.json", "*.ini", "*.png", "*.jpg", "*.jpeg")

Write-Step "Σάρωση $SourceRoot ..."

Get-ChildItem -Path $SourceRoot -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object {
    $rel = $_.FullName.Substring($SourceRoot.Length).TrimStart("\")
    $name = $_.Name.ToLower()
    $ext = $_.Extension.ToLower()

    # Skip huge/build folders
    if ($rel -match '\\(node_modules|\.git|\.pio|build|dist)\\') { return }

    $destSub = "docs\archive\romeos-tsakas\$rel"

    # Route by keywords
    if ($name -match 'hp-board|leitourgia|heat.?pump|αντλ') {
        $destSub = "docs\$($_.Name)"
    }
    elseif ($name -match 'fan|ventil|motor|μοτέρ|βεντιλ|ανεμιστ') {
        $destSub = "docs\hardware\fan-motor\$($_.Name)"
    }
    elseif ($name -match 'schematic|sch|pcb|brd|kicad') {
        $destSub = "docs\schematics\$rel"
    }
    elseif ($name -match 'synig|συζητ|chat|cursor|conversation') {
        $destSub = "docs\conversations\$($_.Name)"
    }
    elseif ($name -match 'secret|mqtt|\.env|password|credential') {
        $destSub = "secrets\$($_.Name)"
    }
    elseif ($name -match 'alpha|motherboard|μητρ') {
        $destSub = "alpha\_imported\$rel"
    }
    elseif ($name -match 'beta|outdoor') {
        $destSub = "beta\_imported\$rel"
    }
    elseif ($name -match 'display|οθόν|thermostat') {
        $destSub = "display\_imported\$rel"
    }
    elseif ($ext -notin @(".md", ".txt", ".pdf", ".kicad_sch", ".kicad_pcb", ".sch", ".brd", ".pcb", ".ini", ".json", ".csv", ".png", ".jpg", ".jpeg")) {
        return
    }

    $dst = Join-Path $DestRoot $destSub
    $dstDir = Split-Path $dst -Parent

    if (-not $WhatIf) {
        if (-not (Test-Path $dstDir)) { New-Item -ItemType Directory -Path $dstDir -Force | Out-Null }
        if (-not (Test-Path $dst)) {
            Copy-Item -Path $_.FullName -Destination $dst -Force
            $manifest += "COPY $rel -> $destSub"
            $copied++
        } else {
            $manifest += "EXISTS $rel"
            $skipped++
        }
    } else {
        $manifest += "WOULD $rel -> $destSub"
        $copied++
    }
}

$manifestPath = Join-Path $DestRoot "docs\archive\romeos-tsakas\IMPORT-MANIFEST.txt"
if (-not $WhatIf) {
    $manifest | Set-Content -Path $manifestPath -Encoding UTF8
}

Write-Host ""
Write-Host "Ολοκληρώθηκε. Copied/would: $copied  Skipped/exists: $skipped" -ForegroundColor Green
Write-Host "Manifest: $manifestPath"
Write-Host ""
Write-Host "Επόμενο: άνοιξε D:\ROMEOS 69 στο Cursor και έλεγξε docs\conversations\ + docs\hardware\fan-motor\" -ForegroundColor Yellow
