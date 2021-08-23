Write-Host "Attempting to run ROM in emulator..." -foregroundcolor blue
$gensPath = "./scripts/emulator/gens.exe"
$romPath = "./../../out/rom.bin"

if (-not (Test-Path -Path $gensPath -PathType Leaf)) {
    throw "Gens emulator '$gensPath' not found"
}

Set-Alias Gens $gensPath

# Get-Process | Where-Object {$_.Path -like $gensPath} | Stop-Process -WhatIf
Get-Process | Where-Object {$_.Path -like '*gens.exe*'} | Stop-Process -Force -processname {$_.ProcessName}

Gens $romPath
Write-Host "Running!" -foregroundcolor green

exit