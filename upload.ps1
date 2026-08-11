# STM32 UART (Serial) Upload Helper Script
# Usage: powershell -ExecutionPolicy Bypass -File .\upload.ps1

# 1. Resolve relative path for the build output (.elf)
# Using $PSScriptRoot makes it work regardless of where the project folder is located on a mentee's PC.
$elfFile = Join-Path $PSScriptRoot "Debug\yahboom_jga.elf"

if (-not (Test-Path $elfFile)) {
    Write-Error "Build output (.elf) not found!"
    Write-Host "Please build the project in STM32CubeIDE first." -ForegroundColor Yellow
    Write-Host "Expected path: $elfFile" -ForegroundColor Yellow
    exit 1
}

# 2. Automatically search for STM32_Programmer_CLI.exe in typical ST install directories
$searchPatterns = @(
    "C:\ST\STM32CubeCLT_*\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe",
    "C:\ST\STM32CubeIDE_*\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_*\tools\bin\STM32_Programmer_CLI.exe",
    "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
)

$cliPath = $null
foreach ($pattern in $searchPatterns) {
    # Resolve-Path handles wildcards like STM32CubeIDE_* automatically
    $resolved = Resolve-Path $pattern -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($resolved) {
        $cliPath = $resolved.Path
        break
    }
}

if (-not $cliPath) {
    Write-Error "STM32_Programmer_CLI.exe not found on your system!"
    Write-Host "Make sure STM32CubeIDE or STM32CubeCLT is installed in the default location (C:\ST or C:\Program Files)." -ForegroundColor Yellow
    exit 1
}

# 3. Auto-detect active COM ports
$ports = [System.IO.Ports.SerialPort]::GetPortNames()
if ($ports.Count -eq 0) {
    Write-Error "No active COM ports found!"
    Write-Host "Please check:" -ForegroundColor Yellow
    Write-Host "1. Is the board connected to the PC via USB cable?" -ForegroundColor Yellow
    Write-Host "2. Is the 12V battery connected and the board power switch turned ON?" -ForegroundColor Yellow
    Write-Host "3. Is the CH340 USB-to-Serial driver installed?" -ForegroundColor Yellow
    exit 1
}

# Select the first detected port (usually COM11 or whatever is active)
$comPort = $ports[0]

Write-Host "--------------------------------------------------------" -ForegroundColor Cyan
Write-Host "Programmer Tool Found: $cliPath" -ForegroundColor Gray
Write-Host "Detected COM Port    : $comPort" -ForegroundColor Green
Write-Host "Target Binary (.elf) : $elfFile" -ForegroundColor Green
Write-Host "--------------------------------------------------------" -ForegroundColor Cyan
Write-Host "[REQUIRED] Put the board into Bootloader mode:" -ForegroundColor Yellow
Write-Host "1. Press and HOLD the 'BOOT0' button on the board." -ForegroundColor Yellow
Write-Host "2. Press and RELEASE the 'RESET' (RST) button." -ForegroundColor Yellow
Write-Host "3. RELEASE the 'BOOT0' button." -ForegroundColor Yellow
Write-Host "When ready, press any key in this terminal to start upload..." -ForegroundColor Cyan
[void]$Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

Write-Host "`nStarting upload..." -ForegroundColor Green

# Run the programmer CLI
& $cliPath -c port=$comPort baud=115200 -w $elfFile -v

if ($LASTEXITCODE -eq 0) {
    Write-Host "`n========================================================" -ForegroundColor Green
    Write-Host "SUCCESS: Upload complete and verified!" -ForegroundColor Green
    Write-Host "Please press the 'RESET' button on the board to run the code." -ForegroundColor Green
    Write-Host "========================================================" -ForegroundColor Green
} else {
    Write-Warning "`nERROR: Upload failed. Please verify the bootloader mode button sequence and try again."
}
