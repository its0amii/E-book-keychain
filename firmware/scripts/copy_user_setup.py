// scripts/copy_user_setup.py
// ============================================================
// PlatformIO pre-build script: copies the project's User_Setup.h
// into the TFT_eSPI library folder so the library picks up
// the pin map for the ESP32-C6 LCD board.
// ============================================================
import os, shutil

Import("env")

SRC  = os.path.join(env["PROJECT_DIR"], "include", "User_Setup.h")
DEST_DIR = os.path.join(env["PROJECT_DIR"], ".pio", "libdeps",
                        env["PIOENV"], "TFT_eSPI")
DEST = os.path.join(DEST_DIR, "User_Setup.h")

if not os.path.isfile(SRC):
    print("ERROR: include/User_Setup.h not found")
    env.Exit(1)

os.makedirs(DEST_DIR, exist_ok=True)
shutil.copyfile(SRC, DEST)
print("TFT_eSPI User_Setup.h -> " + DEST)
