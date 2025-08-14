## Setting Up Dependencies

To ensure the project builds correctly, follow these steps to use the provided versions of the required libraries:

1. **Replace library files**

   - Navigate to `dependencies/required_libraries`.
   - You will find the folders `lvgl` and `TFT_eSPI`.
   - Copy the contents of these folders and **replace** the corresponding folders in your local PlatformIO library directory:
     ```
     .pio/libdeps/esp32dev/
     ```
     &gt; ⚠️ Make sure you overwrite the existing files in these folders with the ones provided.

2. **Update TFT_eSPI configuration**

   - Copy `User_Setup.h` from `dependencies` into:
     ```
     .pio/libdeps/esp32dev/TFT_eSPI/
     ```
   - Replace the existing `User_Setup.h` file in that folder.

3. **Update LVGL configuration**
   - Copy `lv_conf.h` from `dependencies` into your local PlatformIO library directory:
     ```
     .pio/libdeps/esp32dev/
     ```
   - Ensure it sits at the same level as the `lvgl` folder, not inside it.

✅ After completing these steps, the project should use the provided library versions and configuration files, ensuring consistent builds for all users.
