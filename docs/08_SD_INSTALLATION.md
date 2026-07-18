# 8. SD installation and recovery

## Public versus local package

The tracked `sdcard/` directory is a legal launcher/configuration skeleton. It
does not include game data, generated ARM binaries, recovered vendor resources,
or a BusyBox executable. A local deployable package is created only after the
user supplies lawfully redistributable dependencies and their own compatible
game data.

BusyBox is GPLv2 software: if you redistribute a binary, satisfy its exact
license/source obligations. The recovered device binary is intentionally not
part of this public repository.

## Prepare a local card

1. Preserve a verified backup of the existing card.
2. Build both required profiles and populate the ignored local `sdcard/` tree.
3. Check the destination drive identity and capacity; never assume a drive
   letter from documentation.
4. Use FAT32 if that is what the appliance firmware requires.
5. Copy the *contents* of `sdcard/` to the card root.
6. Re-read hashes from the destination before ejecting.

Example PowerShell command after replacing placeholders:

```powershell
robocopy "<repository>\sdcard" "<sd-drive>\" /E /COPY:DAT /DCOPY:DAT
```

A complete private deployment typically contains:

```text
<sd-root>/EasyUI.cfg
<sd-root>/busybox                         user-supplied licensed dependency
<sd-root>/wolf3d/run_wolf3d.sh
<sd-root>/wolf3d/wolf3d-z6s-wl1          generated locally
<sd-root>/wolf3d/wolf3d-z6s-wl6          generated locally
<sd-root>/wolf3d/libwolf_autostart.so     generated locally
<sd-root>/wolf3d/data/...                 user-supplied legal game data
```

Do not add this completed private layout to Git.

## First boot

Insert the SD while the device is powered off. EasyUI initializes first; the
launcher starts after approximately eight seconds. Without a keyboard, the game
shows its original logo, reaches the menu, and enters demo mode after five
seconds. A tap during the menu window switches to interactive touch operation.

Inspect these logs separately:

```text
/mnt/extsd/wolf3d/autostart.log
/mnt/extsd/wolf3d/wolf3d.log
/mnt/extsd/wolf3d/input.log
/mnt/extsd/wolf3d/audio.log
/mnt/extsd/wolf3d/performance.log
```

## Disable or test once through ADB

```sh
sh /mnt/extsd/wolf3d/disable_autostart.sh
reboot
```

This creates `wolf3d/stop`, ends the game, resumes the HMI, and selects the safe
configuration. To re-enable:

```sh
sh /mnt/extsd/wolf3d/enable_autostart.sh
reboot
```

With autostart disabled, use a one-shot run:

```sh
sh /mnt/extsd/wolf3d/test_once.sh
```

## Emergency recovery

Power off, remove the SD, and power on. The installation does not replace
internal libraries. Never remove the card while it is mounted or being written.

For an update, copy new files to temporary names, verify hashes, and replace the
executables only while the device is powered off. Keep the previous known-good
build until the hardware test completes.
