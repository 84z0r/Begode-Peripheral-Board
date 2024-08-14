## Open source custom firmware for begode black peripheral boards.
![lv](https://github.com/user-attachments/assets/e07fac78-8c38-41db-9dcb-4c1bcaffbb72)
- Supports automatic front light mode.
- Display and cooler are working the same way as on the stock firmware.
- 3 LED modes: rainbow (dynamic), red+white and off.
- Back LEDs blink red while braking.
- Highly customisable.

Download the latest firmware for your wheel model from the [Releases]([https://pages.github.com/](https://github.com/84z0r/Begode-Peripheral-Board/releases)) page.
CUSTOM_FIRMWARE builds are **only** for Alexovik's custom firmware users!
**Choose the regular build for the stock Begode firmware or Freestyler mods!**

You need USB-to-TTL adapter to flash it.
1) Disconnect the 6pin cable from the board.
2) Connect 3.3V to VCC, GND to GND, RX (from adapter) to TX (on the board), TX (from adapter) to RX (on the board) and 3.3V to BOOT0. It's better to connect BOOT0 first and VCC after that.
3) Use STM32CubeProgrammer or stm32flash to flash the firmware.
