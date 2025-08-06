move all build/libdeps folders/files into correct folders

manually replace all `TFT_eSPI`, `XPT2046_Touchscreen`, and `lvgl` files (open the folder of each respective folder name and move all the files inside, do not just move the folder it will reinstall it and make it a default preset)

move `lv_conf.h` into libdeps folder **(DO NOT MOVE IT INTO ANY LIBRARIES)**

move `User_Setup.h` into TFT_eSPI