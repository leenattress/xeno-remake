Write-Host "Cleaning up build folder..." -foregroundcolor blue
$file = "./out"
if (test-path $file) {
    Remove-Item -path $file -recurse | Out-Null
}
Write-Host "Clean up complete!" -foregroundcolor green
