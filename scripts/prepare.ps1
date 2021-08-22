# unpack emulator if needed
$gensZip = "./scripts/emulator/gens.zip"
$gensExe = "./scripts/emulator/gens.exe"

if (Test-Path -Path $gensZip -PathType Leaf) {
    if (-not (Test-Path -Path $gensExe -PathType Leaf)) {
        Expand-Archive -Path $gensZip -DestinationPath scripts/emulator -Force
    } else {
        Write-Host "Emulator is present" -foregroundcolor yellow
    }
} else {
    throw "Gens emulator '$gensZip' zip not found"
}

$dockerOutput = docker images -q sgdk
if ($dockerOutput) { 

    Write-Host "SGDK Docker image is present" -foregroundcolor yellow

}
else {

    # pull down SGDK latest version
    Write-Host "Fetching SGDK source code..." -foregroundcolor blue
    git clone https://github.com/Stephane-D/SGDK.git

    # build docker image
    Write-Host "Building docker image..." -foregroundcolor blue
    Set-Location SGDK
    docker build -t sgdk .

    # remove SGDK
    Write-Host "Cleaning up SGDK source code..." -foregroundcolor blue
    $file = "SGDK"
    Set-Location ./..
    if (test-path $file) {
        Remove-Item -path $file -recurse -force
    }
}
Write-Host "Preperation complete!" -foregroundcolor green
