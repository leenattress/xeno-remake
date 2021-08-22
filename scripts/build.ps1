Write-Host "Building ROM..." -foregroundcolor blue
docker run --rm -v ${pwd}:/src sgdk
Write-Host "Build complete!" -foregroundcolor green