# Measure central "21" in ui_reference.png; strip only digit pixels; keep degree symbol in bitmap.
param(
    [string]$Png = (Join-Path (Split-Path $PSScriptRoot -Parent) "ui_reference.png"),
    [string]$OutH = (Join-Path (Split-Path $PSScriptRoot -Parent) "include\ui_reference_main_dial_measure.h")
)

Add-Type -AssemblyName System.Drawing

function Get-Lum([int]$r, [int]$g, [int]$b) {
    return [double](0.299 * $r + 0.587 * $g + 0.114 * $b)
}

$path = (Resolve-Path $Png).Path
$src = [System.Drawing.Bitmap]::FromFile($path)
$imgW = $src.Width
$imgH = $src.Height

# Inner search area (central dial dark arc + text only)
$roiL = 235
$roiT = 168
$roiR = 405
$roiB = 302

# Cream text on charcoal: luminance well above background
$brightThresh = 118.0

$wRoi = $roiR - $roiL + 1
$colSum = New-Object int[] $wRoi

for ($x = $roiL; $x -le $roiR; $x++) {
    $xi = $x - $roiL
    for ($y = $roiT; $y -le $roiB; $y++) {
        $c = $src.GetPixel($x, $y)
        if ((Get-Lum $c.R $c.G $c.B) -ge $brightThresh) {
            $colSum[$xi]++
        }
    }
}

$maxCol = ($colSum | Measure-Object -Maximum).Maximum
if ($maxCol -lt 2) {
    $src.Dispose()
    throw "No bright text found in ROI (maxCol=$maxCol). Lower brightThresh."
}

# Trim columns to where text actually is
$cut = [Math]::Max(1, [int]($maxCol * 0.08))
$xMin = $roiL
for ($i = 0; $i -lt $colSum.Length; $i++) {
    if ($colSum[$i] -ge $cut) { $xMin = $roiL + $i; break }
}
$xMax = $roiR
for ($i = $colSum.Length - 1; $i -ge 0; $i--) {
    if ($colSum[$i] -ge $cut) { $xMax = $roiL + $i; break }
}

# Row span for bright pixels in [xMin..xMax]
$minY = [int]::MaxValue
$maxY = [int]::MinValue
for ($x = $xMin; $x -le $xMax; $x++) {
    for ($y = $roiT; $y -le $roiB; $y++) {
        $c = $src.GetPixel($x, $y)
        if ((Get-Lum $c.R $c.G $c.B) -ge $brightThresh) {
            if ($y -lt $minY) { $minY = $y }
            if ($y -gt $maxY) { $maxY = $y }
        }
    }
}
if ($minY -gt $maxY) {
    $src.Dispose()
    throw "Could not find vertical span for text."
}

# Recompute column sums only in [minY..maxY]
for ($i = 0; $i -lt $colSum.Length; $i++) { $colSum[$i] = 0 }
for ($x = $roiL; $x -le $roiR; $x++) {
    $xi = $x - $roiL
    for ($y = $minY; $y -le $maxY; $y++) {
        $c = $src.GetPixel($x, $y)
        if ((Get-Lum $c.R $c.G $c.B) -ge $brightThresh) {
            $colSum[$xi]++
        }
    }
}

$maxCol = ($colSum | Measure-Object -Maximum).Maximum
$valleyTh = [Math]::Max(1, [int]($maxCol * 0.18))
$splits = New-Object System.Collections.Generic.List[int]
for ($i = 1; $i -lt $colSum.Length - 1; $i++) {
    if ($colSum[$i] -le $valleyTh) {
        $left = $colSum[$i - 1]
        $right = $colSum[$i + 1]
        if ($left -gt $valleyTh -and $right -gt $valleyTh) {
            [void]$splits.Add($roiL + $i)
        }
    }
}

$span = $xMax - $xMin + 1
if ($splits.Count -ge 2) {
    $xSplit1 = $splits[0]
    $xSplit2 = $splits[1]
} elseif ($splits.Count -eq 1) {
    $xSplit1 = $splits[0]
    $xSplit2 = [int]($xMin + $span * 0.66)
} else {
    $xSplit1 = [int]($xMin + $span / 3)
    $xSplit2 = [int]($xMin + 2 * $span / 3)
}

$digit2L = $xMin
$digit2R = [Math]::Min($xSplit1 - 1, $xMax)
$digit1L = $xSplit1
$digit1R = [Math]::Min($xSplit2 - 1, $xMax)
$degL = $xSplit2
$degR = $xMax

# Background colour (median of dark pixels in ROI)
$br = New-Object System.Collections.Generic.List[int]
$bg = New-Object System.Collections.Generic.List[int]
$bb = New-Object System.Collections.Generic.List[int]
for ($x = $roiL; $x -le $roiR; $x++) {
    for ($y = $roiT; $y -le $roiB; $y++) {
        $c = $src.GetPixel($x, $y)
        if ((Get-Lum $c.R $c.G $c.B) -lt $brightThresh) {
            [void]$br.Add($c.R)
            [void]$bg.Add($c.G)
            [void]$bb.Add($c.B)
        }
    }
}
function Median-List($lst) {
    $a = @($lst | Sort-Object)
    $n = $a.Count
    if ($n -eq 0) { return 55 }
    return $a[[int]($n / 2)]
}
$mr = Median-List $br
$mg = Median-List $bg
$mb = Median-List $bb

$bmp = New-Object System.Drawing.Bitmap $imgW, $imgH
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.DrawImageUnscaled($src, 0, 0)
$g.Dispose()
$src.Dispose()

$softTh = $brightThresh - 22
for ($x = $digit2L; $x -le $digit2R; $x++) {
    for ($y = $minY; $y -le $maxY; $y++) {
        $c = $bmp.GetPixel($x, $y)
        if ((Get-Lum $c.R $c.G $c.B) -ge $softTh) {
            $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $mr, $mg, $mb))
        }
    }
}
for ($x = $digit1L; $x -le $digit1R; $x++) {
    for ($y = $minY; $y -le $maxY; $y++) {
        $c = $bmp.GetPixel($x, $y)
        if ((Get-Lum $c.R $c.G $c.B) -ge $softTh) {
            $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(255, $mr, $mg, $mb))
        }
    }
}

$tmp = $path + ".tmp.png"
$bmp.Save($tmp, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
[System.IO.File]::Delete($path)
[System.IO.File]::Move($tmp, $path)

$c2x = [int](($digit2L + $digit2R) / 2)
$c1x = [int](($digit1L + $digit1R) / 2)
$cdx = [int](($degL + $degR) / 2)
$textH = $maxY - $minY + 1
$w2 = $digit2R - $digit2L + 1
$w1 = $digit1R - $digit1L + 1
$wd = $degR - $degL + 1
$gapPx = 4
$digitsBlockL = $digit2L
$digitsBlockR = $digit1R

$sb = [System.Text.StringBuilder]::new()
[void]$sb.AppendLine('/* Auto-generated by scripts/measure_and_strip_center_21.ps1 - do not edit */')
[void]$sb.AppendLine('#ifndef UI_REFERENCE_MAIN_DIAL_MEASURE_H')
[void]$sb.AppendLine('#define UI_REFERENCE_MAIN_DIAL_MEASURE_H')
[void]$sb.AppendLine('')
[void]$sb.AppendLine('#include <stdint.h>')
[void]$sb.AppendLine('')
[void]$sb.AppendLine('/* Full bright text bbox (21 + degree) in px */')
[void]$sb.AppendLine(("static const int16_t kUiMainDialTextLeft = {0};" -f $xMin))
[void]$sb.AppendLine(("static const int16_t kUiMainDialTextRight = {0};" -f $xMax))
[void]$sb.AppendLine(("static const int16_t kUiMainDialTextTop = {0};" -f $minY))
[void]$sb.AppendLine(("static const int16_t kUiMainDialTextBottom = {0};" -f $maxY))
[void]$sb.AppendLine(("static const int16_t kUiMainDialTextHeight = {0};" -f $textH))
[void]$sb.AppendLine('')
[void]$sb.AppendLine('/* Removed glyph ""2"" */')
[void]$sb.AppendLine(("static const int16_t kUiMainDialG2Left = {0};" -f $digit2L))
[void]$sb.AppendLine(("static const int16_t kUiMainDialG2Right = {0};" -f $digit2R))
[void]$sb.AppendLine(("static const int16_t kUiMainDialG2CenterX = {0};" -f $c2x))
[void]$sb.AppendLine(("static const int16_t kUiMainDialG2Width = {0};" -f $w2))
[void]$sb.AppendLine('')
[void]$sb.AppendLine('/* Removed glyph ""1"" */')
[void]$sb.AppendLine(("static const int16_t kUiMainDialG1Left = {0};" -f $digit1L))
[void]$sb.AppendLine(("static const int16_t kUiMainDialG1Right = {0};" -f $digit1R))
[void]$sb.AppendLine(("static const int16_t kUiMainDialG1CenterX = {0};" -f $c1x))
[void]$sb.AppendLine(("static const int16_t kUiMainDialG1Width = {0};" -f $w1))
[void]$sb.AppendLine('')
[void]$sb.AppendLine('/* Degree symbol (bitmap) */')
[void]$sb.AppendLine(("static const int16_t kUiMainDialDegLeft = {0};" -f $degL))
[void]$sb.AppendLine(("static const int16_t kUiMainDialDegRight = {0};" -f $degR))
[void]$sb.AppendLine(("static const int16_t kUiMainDialDegCenterX = {0};" -f $cdx))
[void]$sb.AppendLine(("static const int16_t kUiMainDialDegWidth = {0};" -f $wd))
[void]$sb.AppendLine('')
[void]$sb.AppendLine('/* LVGL integer setpoint: label top-left (Montserrat 48). Align right edge before degree. */')
[void]$sb.AppendLine(("static const int16_t kUiMainDialSetpointLabelX = {0};" -f $digitsBlockL))
[void]$sb.AppendLine(("static const int16_t kUiMainDialSetpointLabelY = {0};" -f $minY))
[void]$sb.AppendLine(("/* Target: label ends near x <= {0} (deg at {1}, gap {2}px) */" -f ($degL - $gapPx), $degL, $gapPx))
[void]$sb.AppendLine('')
[void]$sb.AppendLine('#endif')
[void]$sb.AppendLine('')

[System.IO.File]::WriteAllText($OutH, $sb.ToString(), [System.Text.UTF8Encoding]::new($false))

Write-Host "Text bbox: X $xMin..$xMax  Y $minY..$maxY  H=$textH"
Write-Host "2: $digit2L..$digit2R   1: $digit1L..$digit1R   deg: $degL..$degR"
Write-Host "Wrote $path and $OutH"
