NOVA3D Z6S - Wolf3D reconstructed package

1. Keep a backup of this card on the PC.
2. Insert the card with the LCD powered off.
3. Connect a passive speaker only to SPK+ and SPK-.
4. Connect the keyboard to a real USB-host/OTG port, not blindly to the ADB
   gadget port. USB-host operation still needs confirmation on the hardware.
5. Boot. Wolf3D starts after the vendor HMI initializes the display.

Startup behavior:
- with a keyboard, the original logo waits for a key;
- without a keyboard, the logo times out, the real main menu is shown for
  five seconds, and then a demo starts automatically;
- a key or touch during that menu preview opens the interactive menu.

Video always remains in the LCD native 480x272x32 mode. Wolf3D renders at
320x200 internally and scales complete opaque frames to full screen; logo,
menus and 3D gameplay never request different framebuffer resolutions.

The card includes WL1 and WL6 engines plus the owner's complete legal WL6 data
set, so the launcher selects the registered six-episode edition automatically.
The WL1 shareware set remains as a fallback. These commercial data files are
personal: do not redistribute this card image or wolf3d/data.

Emergency recovery:
- power off and remove the SD: the internal vendor HMI remains untouched;
- or over ADB run: sh /mnt/extsd/wolf3d/disable_autostart.sh

Logs are in /mnt/extsd/wolf3d/*.log.
Full documentation is kept in the PC project under docs/.
