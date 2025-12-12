# build.ps1
# This script builds the PlatformIO project.
# Usage: ./build.ps1 <example_number> (selects example, copies to src/main.cpp, then builds)

param(
    [Parameter(Mandatory=$false)]
    [int]$ExampleNumber
)

# Check if PlatformIO CLI is installed
if (-not (Get-Command pio -ErrorAction SilentlyContinue)) {
    Write-Host "PlatformIO CLI is not found." -ForegroundColor Red
    Write-Host "Please install PlatformIO (e.g., pip install platformio) or ensure it's in your PATH." -ForegroundColor Red
    exit 1
}

# If an example number is provided, use the selection script to copy the example to src/main.cpp
if ($ExampleNumber) {
    $selectScript = ".\scripts\select_example.ps1"
    if (Test-Path $selectScript) {
        Write-Host "Selecting example $ExampleNumber..." -ForegroundColor Yellow
        # Execute the select_example.ps1 script
        & $selectScript $ExampleNumber

        if ($LASTEXITCODE -ne 0) {
            Write-Host "Failed to select example. Aborting build." -ForegroundColor Red
            exit $LASTEXITCODE
        }
    } else {
        Write-Host "Error: Selection script '$selectScript' not found." -ForegroundColor Red
        exit 1
    }
} else {
    Write-Host "Usage: ./build.ps1 <example_number>" -ForegroundColor Yellow
    Write-Host "Example: ./build.ps1 4 (to build example_04_random)" -ForegroundColor Yellow
    Write-Host "Please provide an example number to build." -ForegroundColor Red
    exit 1
}

# Build only the default environment (esp-wrover-kit) to avoid building all environments defined in platformio.ini
# This respects the "overwrite src/main.cpp" workflow.
$envName = "esp-wrover-kit"
Write-Host "Building PlatformIO project (env: $envName)..." -ForegroundColor Green
pio run -e $envName

if ($LASTEXITCODE -ne 0) {
    Write-Host "PlatformIO build failed!" -ForegroundColor Red
    exit $LASTEXITCODE
} else {
    Write-Host "PlatformIO build succeeded!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  To upload firmware: pio run -e $envName -t upload" -ForegroundColor White
    Write-Host "  To monitor device:  pio device monitor" -ForegroundColor White
}