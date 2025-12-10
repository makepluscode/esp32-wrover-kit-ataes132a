# PowerShell script: Select and copy example
# Usage: .\scripts\select_example.ps1 <example_number>
# Example: .\scripts\select_example.ps1 1

# Set UTF-8 encoding for output
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

# Get example number from argument
if ($args.Count -eq 0) {
    Write-Host "Error: Example number required." -ForegroundColor Red
    Write-Host "Usage: .\scripts\select_example.ps1 <example_number>" -ForegroundColor Yellow
    exit 1
}

$ExampleNumber = [int]$args[0]

if ($ExampleNumber -lt 1 -or $ExampleNumber -gt 10) {
    Write-Host "Error: Example number must be between 1 and 10." -ForegroundColor Red
    exit 1
}

$exampleDir = Get-ChildItem -Path "examples" -Directory | Where-Object { $_.Name -match "^0?$ExampleNumber[_-]" } | Select-Object -First 1

if (-not $exampleDir) {
    Write-Host "Error: Example $ExampleNumber not found." -ForegroundColor Red
    Write-Host "Available examples:" -ForegroundColor Yellow
    Get-ChildItem -Path "examples" -Directory | Where-Object { $_.Name -match "^\d+" } | ForEach-Object {
        Write-Host "  - $($_.Name)" -ForegroundColor Cyan
    }
    exit 1
}

Write-Host "Selected example: $($exampleDir.Name)" -ForegroundColor Green
Write-Host "Directory: $($exampleDir.FullName)" -ForegroundColor Cyan
Write-Host ""

# Replace src/main.cpp with example's main.cpp
$srcMain = "src\main.cpp"
$exampleMain = Join-Path $exampleDir.FullName "main.cpp"

if (Test-Path $exampleMain) {
    Copy-Item -Path $exampleMain -Destination $srcMain -Force
    Write-Host "Replaced src/main.cpp with $($exampleDir.Name)/main.cpp" -ForegroundColor Green
    Write-Host ""
    Write-Host "Next steps:" -ForegroundColor Yellow
    Write-Host "  pio run -e esp-wrover-kit -t upload" -ForegroundColor White
    Write-Host "  pio device monitor" -ForegroundColor White
} else {
    Write-Host "Error: $exampleMain not found." -ForegroundColor Red
    exit 1
}
