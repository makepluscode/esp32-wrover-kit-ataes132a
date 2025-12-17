# Unit Testing Guide

This project uses the **Unity** framework for unit testing via PlatformIO.
Tests are designed to run on the **ESP32-WROVER-KIT** hardware to verify library functions like CRC calculation and packet marshaling.

## Prerequisites
- **PlatformIO Core (CLI)** or **PlatformIO IDE** installed.
- **ESP32-WROVER-KIT** connected via USB.
- **CRITICAL**: Ensure the serial port (e.g., `COM5`) is **NOT** currently in use by any other application (e.g., Serial Monitor, other terminals). The test runner needs exclusive access to the port to upload firmware and read results.

## Running Tests

Execute the following command in your terminal:

```bash
pio test -e esp-wrover-kit
```

### Expected Output
If successful, you will see output indicating the tests passed:
```text
test/test_aes132_comm/test_main.cpp:52:test_crc_calculation_sleep_command    [PASSED]
test/test_aes132_comm/test_main.cpp:53:test_crc_calculation_standby_command  [PASSED]
-----------------------
1 Tests 0 Failures 0 Ignored
OK
```

## Troubleshooting

### "Access is denied" or Serial Port Busy
If the upload fails or the test hangs connecting to the port:
1.  **Close all Serial Monitor instances** (in VSCode, click the trash icon on the terminal panel).
2.  Unplug and replug the USB cable.
3.  Run the test command again.

### Multiple Definition Errors (`setup` / `loop`)
The `platformio.ini` has been configured with `test_build_src = no` to prevent conflicts between the main application's `setup()` and the test runner's `setup()`. If you modify `platformio.ini`, ensure this setting remains.
