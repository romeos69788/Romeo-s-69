#Requires -Version 5.1
# UTF-8 BOM: fixes PowerShell 5.1 parsing when run from cmd.exe
# Sends WIFI_HOME_SET, SSID, password over serial to ESP (romeos_wifi_nvs).
param(
    [Parameter(Mandatory = $true)]
    [string] $Port,
    [string] $Ssid = "MERCUSYS_Romeos"
)

$ErrorActionPreference = "Stop"
$sp = $null

if (-not $env:ROMEOS_HOME_WIFI_PASS -or $env:ROMEOS_HOME_WIFI_PASS.Length -eq 0) {
    $prompt = "Wi-Fi password for SSID [" + $Ssid + "]: "
    $secure = Read-Host $prompt -AsSecureString
    $ptr = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($secure)
    try {
        $passPlain = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($ptr)
    }
    finally {
        [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($ptr) | Out-Null
    }
}
else {
    $passPlain = $env:ROMEOS_HOME_WIFI_PASS
    Write-Host "Using password from env ROMEOS_HOME_WIFI_PASS." -ForegroundColor DarkGray
}

try {
    $sp = New-Object System.IO.Ports.SerialPort $Port, 115200, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
    $sp.NewLine = "`n"
    $sp.ReadTimeout = 3000
    $sp.WriteTimeout = 3000
    $sp.Open()
    Write-Host ("Opened " + $Port + ". Sending WIFI_HOME_SET...") -ForegroundColor Cyan

    function Send-Line([string] $line) {
        $bytes = [Text.Encoding]::UTF8.GetBytes($line + "`n")
        $sp.Write($bytes, 0, $bytes.Length)
        $sp.BaseStream.Flush()
        Start-Sleep -Milliseconds 400
    }

    Send-Line "WIFI_HOME_SET"
    Send-Line $Ssid
    Send-Line $passPlain
    $passPlain = $null

    Start-Sleep -Milliseconds 800
    $buf = New-Object System.Text.StringBuilder
    $deadline = [DateTime]::UtcNow.AddSeconds(3)
    while ([DateTime]::UtcNow -lt $deadline) {
        while ($sp.BytesToRead -gt 0) {
            $null = $buf.Append([char]$sp.ReadChar())
        }
        Start-Sleep -Milliseconds 150
    }
    while ($sp.BytesToRead -gt 0) {
        $null = $buf.Append([char]$sp.ReadChar())
    }

    $out = $buf.ToString()
    if ($out.Length -gt 0) {
        Write-Host ""
        Write-Host "--- Serial reply ---" -ForegroundColor Green
        Write-Host $out
    }
    else {
        Write-Host ""
        Write-Host "(No reply text. Check firmware running and correct COM port.)" -ForegroundColor Yellow
    }

    Write-Host ""
    Write-Host "Done. Reset the ESP if Wi-Fi does not reconnect by itself." -ForegroundColor Cyan
}
catch {
    Write-Host ("Error: " + $_) -ForegroundColor Red
    Write-Host ("Close Serial Monitor or other apps using " + $Port) -ForegroundColor Yellow
    exit 1
}
finally {
    if ($sp -and $sp.IsOpen) {
        $sp.Close()
    }
}
