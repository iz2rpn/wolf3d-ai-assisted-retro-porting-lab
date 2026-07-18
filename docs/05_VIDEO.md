# 5. Video: framebuffer, scala e byte alpha

## Scelta di risoluzione

Il pannello è sempre 480×272×32, mentre Wolf3D continua a renderizzare a
320×200 palettizzati. Calcolare direttamente la scena a 480×272 aumenterebbe i
pixel del 104% senza aggiungere dettaglio alle texture originali.

`VL_Z6SPresent()` converte e scala ogni fotogramma logico sull’intera superficie
fisica. Le mappe X/Y vengono calcolate una volta; il ciclo per pixel esegue poi
soltanto lookup di palette e copie. Un buffer intermedio da 522.240 byte evita
che lo scanout osservi un fotogramma costruito riga per riga.

Opzioni runtime:

```text
--resf 320 200 --bits 32
SDL_VIDEODRIVER=fbcon
SDL_FBDEV=/dev/fb0
SDL_FBCON_NO_CONSOLE=1
SDL_FB_BROKEN_MODES=1
Z6S_NATIVE_WIDTH=480
Z6S_NATIVE_HEIGHT=272
```

`SDL_FB_BROKEN_MODES=1` impedisce a SDL di provare risoluzioni alternative.
Il doppio buffer fbcon è disabilitato perché modificherebbe `yres_virtual`.
Logo, menu, dissolvenze e renderer 3D passano tutti dallo stesso presentatore;
le vecchie dissolvenze casuali che scrivevano direttamente nel framebuffer
logico sono sostituite da una presentazione completa immediata.

## Perché SDL fbcon è stato modificato

SDL 1.2 prova normalmente ad aprire una virtual console e a impostare la
tastiera in medium-raw mode. Il firmware non espone una VT Linux utilizzabile.
Con `SDL_FBCON_NO_CONSOLE=1`, `SDL_fbevents.c` salta quelle operazioni; l’input
arriva dal backend evdev indipendente.

## Il problema dello schermo nero

I dump di `/dev/fb0` contenevano RGB corretti, ma il pannello restava nero. Nel
formato vendor il byte alto del pixel a 32 bit viene interpretato come alpha;
il blitter palettizzato SDL lo lasciava a zero.

Ogni colore convertito dal nuovo presentatore applica direttamente:

```c
pixel |= 0xff000000U;
```

La copia finale rispetta `screen->pitch`, non presume righe contigue. La vecchia
`VL_Z6SForceOpaque()` resta solo come ripiego per formati video inattesi.

## Diagnostica sul dispositivo

Se il gioco è nero ma il processo vive:

```sh
/mnt/extsd/busybox pidof wolf3d-z6s-wl1 wolf3d-z6s-wl6
/mnt/extsd/busybox tail -100 /mnt/extsd/wolf3d/wolf3d.log
/mnt/extsd/busybox dd if=/dev/fb0 of=/mnt/extsd/wolf3d/fb-test.raw bs=522240 count=2
```

Conservare il dump prima di cambiare codice: distingue un problema di rendering
da un problema di scanout/alpha.
