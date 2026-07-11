$ErrorActionPreference = "Stop"
$root = Join-Path $PSScriptRoot "..\src\squareline\images"
$files = Get-ChildItem (Join-Path $root "ui_img_*.c")

function Convert-ImageFile([string]$path) {
    $text = [System.IO.File]::ReadAllText($path)
    $pattern = '(?s)(const\s+LV_ATTRIBUTE_MEM_ALIGN\s+uint8_t\s+\w+_data\[\]\s*=\s*\{)(.*?)(\}\s*;)'
    $m = [regex]::Match($text, $pattern)
    if (-not $m.Success) { Write-Host "skip $path"; return }

    $hexMatches = [regex]::Matches($m.Groups[2].Value, '0x([0-9a-fA-F]+)')
    $n = $hexMatches.Count
    if (($n % 4) -ne 0) { throw "$path : $n bytes not x4" }

    $parts = New-Object System.Collections.Generic.List[string]
    for ($i = 0; $i -lt $n; $i += 4) {
        $b = [Convert]::ToInt32($hexMatches[$i].Groups[1].Value, 16)
        $g = [Convert]::ToInt32($hexMatches[$i + 1].Groups[1].Value, 16)
        $r = [Convert]::ToInt32($hexMatches[$i + 2].Groups[1].Value, 16)
        $a = [Convert]::ToInt32($hexMatches[$i + 3].Groups[1].Value, 16)
        $full = ((($r -shr 3) -shl 11) -bor (($g -shr 2) -shl 5) -bor ($b -shr 3)) -band 0xFFFF
        $lo = $full -band 0xFF
        $hi = ($full -shr 8) -band 0xFF
        $aa = $a -band 0xFF
        $parts.Add("0x{0:X2}" -f $lo)
        $parts.Add("0x{0:X2}" -f $hi)
        $parts.Add("0x{0:X2}" -f $aa)
    }

    $perLine = 24
    $indent = "    "
    $sb = [System.Text.StringBuilder]::new()
    for ($j = 0; $j -lt $parts.Count; $j += $perLine) {
        $take = [Math]::Min($perLine, $parts.Count - $j)
        $chunk = $parts.GetRange($j, $take)
        [void]$sb.Append($indent)
        [void]$sb.AppendLine(($chunk -join ",") + ",")
    }

    $newText = $text.Substring(0, $m.Groups[1].Index + $m.Groups[1].Length) +
        "`n" + $sb.ToString() +
        $text.Substring($m.Groups[3].Index)

    if ($newText -ne $text) {
        [System.IO.File]::WriteAllText($path, $newText, [System.Text.UTF8Encoding]::new($false))
        $newBytes = $parts.Count
        Write-Host "OK $(Split-Path $path -Leaf): $n -> $newBytes bytes"
    }
}

foreach ($f in $files) { Convert-ImageFile $f.FullName }
