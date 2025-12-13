# build.ps1
# This script builds the PlatformIO project.
# Usage: ./build.ps1 <example_number> [action]
# Actions:
#   build  : Build only (default)
#   upload : Build and Upload
#   all    : Build, Upload, and Monitor

param(
    [Parameter(Mandatory=$false)]
    [int]$ExampleNumber,

    [Parameter(Mandatory=$false)]
    [string]$Action = "build"
)

# Check if PlatformIO CLI is installed
if (-not (Get-Command pio -ErrorAction SilentlyContinue)) {
    Write-Host "PlatformIO CLI is not found." -ForegroundColor Red
    Write-Host "Please install PlatformIO (e.g., pip install platformio) or ensure it's in your PATH." -ForegroundColor Red
    exit 1
}

# Ensure arguments are valid
if (-not $ExampleNumber) {
    Write-Host "Usage: ./build.ps1 <example_number> [action]" -ForegroundColor Yellow
    Write-Host "Actions: build (default), upload, all" -ForegroundColor Yellow
    Write-Host "Example: ./build.ps1 6 upload" -ForegroundColor Yellow
    Write-Host "Please provide an example number." -ForegroundColor Red
    exit 1
}

# Select Example
$selectScript = ".\scripts\select_example.ps1"
if (Test-Path $selectScript) {
    Write-Host "Selecting example $ExampleNumber..." -ForegroundColor Yellow
    & $selectScript $ExampleNumber
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Failed to select example. Aborting." -ForegroundColor Red
        exit $LASTEXITCODE
    }
} else {
    Write-Host "Error: Selection script '$selectScript' not found." -ForegroundColor Red
    exit 1
}

$envName = "esp-wrover-kit"
Write-Host "Building PlatformIO project (env: $envName)..." -ForegroundColor Green

# Determine command based on Action
if ($Action -eq "upload" -or $Action -eq "all") {
    Write-Host "Action: Build & Upload" -ForegroundColor Cyan
    pio run -e $envName -t upload
} else {
    Write-Host "Action: Build only" -ForegroundColor Cyan
    pio run -e $envName
}

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build/Upload failed!" -ForegroundColor Red
    exit $LASTEXITCODE
}

# If 'all', verify success and run monitor
if ($Action -eq "all") {
    Write-Host "Build/Upload succeeded. Starting Monitor..." -ForegroundColor Green
    pio device monitor
} else {
    Write-Host "Success!" -ForegroundColor Green
    if ($Action -ne "upload") {
        Write-Host "Next steps:" -ForegroundColor Yellow
        Write-Host "  To upload: ./build.ps1 $ExampleNumber upload" -ForegroundColor White
    }
    Write-Host "  To monitor: pio device monitor" -ForegroundColor White
}