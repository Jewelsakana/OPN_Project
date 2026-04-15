# --- Build and test script ---
chcp 65001 > $null
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

Write-Host "--- Building main project ---" -ForegroundColor Cyan
mingw32-make all

if ($LASTEXITCODE -ne 0) {
    Write-Host "Main project build failed. Tests aborted." -ForegroundColor Red
    exit $LASTEXITCODE
}

$objs = Get-ChildItem "build/*.o" | Where-Object { $_.Name -ne "main.o" } | ForEach-Object { "build/" + $_.Name }

Write-Host "`n--- Running tests ---" -ForegroundColor Cyan
$testFiles = Get-ChildItem "tests/test_*.cpp"

foreach ($file in $testFiles) {
    $testName = $file.BaseName
    Write-Host "Building: $testName..." -ForegroundColor Yellow
    
    g++ -std=c++17 -Wall -Iinclude -I. "$($file.FullName)" $objs -o "build/$testName.exe"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Running: $testName..." -ForegroundColor Green
        & "build/$testName.exe"
    } else {
        Write-Host "Build failed: $testName" -ForegroundColor Red
    }
    Write-Host "---------------------------------------"
}

Write-Host "`nAll tests completed." -ForegroundColor Cyan