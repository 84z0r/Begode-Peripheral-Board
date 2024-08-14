## Open source custom firmware for begode black peripheral boards.
![lv](https://github.com/user-attachments/assets/e07fac78-8c38-41db-9dcb-4c1bcaffbb72)
- Supports automatic front light mode.
- Display and cooler are working the same way as on the stock firmware.
- 3 LED modes: rainbow (dynamic), red+white and off.
- Back LEDs blink red while braking.
- Highly customisable.

You need USB-to-TTL adapter to flash it.
- Connect 3.3V to VCC, GND to GND, RX (from adapter) to TX (on the board), TX (from adapter) to RX (on the board) and 3.3V to BOOT0.
- Use STM32CubeProgrammer or stm32flash to flash the firmware.

DO NOT use CUSTOM_FIRMWARE builds is you don't use Alexovik's custom firmware!
