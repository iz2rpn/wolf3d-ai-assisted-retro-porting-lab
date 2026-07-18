# 9. Debug e ottimizzazione

## Ordine corretto di diagnosi

1. **Processo:** `pidof wolf3d-z6s-wl1 wolf3d-z6s-wl6`.
2. **Log avvio:** proxy e launcher.
3. **Dati:** estensione WL1/WL6 coerente con il profilo compilato.
4. **Video:** framebuffer e alpha.
5. **Input:** capability evdev e log dispositivi.
6. **Audio:** apertura PCM e underrun.

Cambiare un solo livello per volta rende le prove confrontabili.

## Comandi ADB utili

```sh
cat /proc/cpuinfo
cat /proc/meminfo
cat /proc/bus/input/devices
ls -l /dev/fb0 /dev/input/event* /dev/snd/*
cat /sys/class/graphics/fb0/virtual_size
cat /sys/class/graphics/fb0/bits_per_pixel
ps
dmesg | tail -100
```

Dopo ogni avvio controllare anche `/mnt/extsd/wolf3d/wolf3d.log`: la build
corretta deve riportare prima il framebuffer `480,272, 32bpp` e poi
`Z6S video scaler: 320x200 logical -> 480x272x32 native`. `audio.log` e
`input.log` vengono azzerati dal launcher, quindi descrivono soltanto l’ultima
prova.

## Scelte di performance già applicate

- 320×200 invece di 480×272 logici.
- scalatura full-screen con mappe X/Y pre-calcolate e framebuffer sempre
  480×272×32;
- nessuna scansione delle modalità fbcon e nessun cambio di `yres_virtual`;
- Sample rate 11025 Hz.
- Mixer a otto canali e un periodo alla volta.
- Nessun thread audio o input.
- Polling input non bloccante e scansione hot-plug ogni due secondi.
- `-Os`, ARMv5TE specifico e garbage collection delle sezioni.
- SDL senza X11, OpenGL, audio generico e CD-ROM.
- Governor `performance` soltanto mentre il gioco gira, poi ripristinato.

## Misurare prima di ottimizzare ancora

```sh
top
pid=$(pidof wolf3d-z6s-wl1 wolf3d-z6s-wl6)
cat /proc/${pid%% *}/status
```

Se il limite è CPU, ridurre effetti simultanei o mantenere 11025 Hz. Se il
limite è I/O, non aumentare i log nel frame loop. Se l’audio scatta ma la CPU è
libera, aumentare `--audiobuffer` a 1024; se il controllo diventa lento,
ritornare a 512.

## Warning di compilazione

Wolf4SDL è codice storico e GCC moderno segnala molte letture/scritture con
valore ignorato e formati datati. Non sono stati nascosti con `-w`: restano
visibili per un futuro hardening. Il build non usa `-Werror`, perché warning
preesistenti non devono impedire un port riproducibile.
