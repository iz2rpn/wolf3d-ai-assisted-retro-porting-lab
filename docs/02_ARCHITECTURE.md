# 2. Architettura del port

## Flusso di avvio

1. EasyUI legge `/mnt/extsd/EasyUI.cfg`.
2. Carica `libwolf_autostart.so` come libreria di startup.
3. Il proxy carica l’originale `/res/lib/libzkgui.so` e inoltra la chiamata
   `onEasyUIInit`, quindi non salta l’inizializzazione hardware del produttore.
4. Un processo figlio attende otto secondi ed esegue `run_wolf3d.sh` con
   BusyBox.
5. Lo script sospende `zkgui` con `SIGSTOP`, imposta la CPU su `performance` e
   avvia Wolf3D.
6. All’uscita ripristina il governor e invia `SIGCONT` all’HMI.

Il firmware interno non viene modificato. Se la SD viene rimossa a display
spento, torna il normale avvio interno.

## Albero importante

```text
reconstructed_wolf3d/
├── docs/                       guida del port
├── patches/                    modifiche riproducibili agli upstream
├── port/z6s/
│   ├── autostart/              proxy EasyUI senza libc
│   ├── include/SDL_mixer.h     API minima richiesta da Wolf4SDL
│   └── src/
│       ├── sdl_mixer_z6s.cpp   mixer + tinyalsa sincrono
│       └── z6s_input.cpp       tastiera e touch evdev
├── scripts/
│   ├── fetch_sources.sh        clone, commit pin e patch
│   └── build_z6s.sh            cross-build completo
├── sdcard/                     contenuto da copiare sulla SD
└── third_party/
    ├── id-wolf3d/              sorgente DOS ufficiale, riferimento
    ├── wolf4sdl/               motore effettivamente compilato
    ├── SDL-1.2.15/             framebuffer Linux
    └── tinyalsa/               accesso PCM minimale
```

## Perché Wolf4SDL e non il sorgente DOS direttamente

Il repository id conserva il codice originale DOS, con assembly x86, VGA,
interrupt e Sound Blaster. È prezioso per studiare il motore, ma non è una base
direttamente eseguibile su ARM/Linux. Wolf4SDL ha già sostituito quei livelli
con C/C++ e SDL, lasciandoci concentrare sulle quattro interfacce specifiche
del dispositivo: video, input, audio e avvio.

## Commit fissati

| Progetto | Commit |
|---|---|
| Wolf4SDL | `3d41ccce8f8fecbed83aa9d8d42734c2c7e62374` |
| id Software wolf3d | `05167784ef009d0d0daefe8d012b027f39dc8541` |
| SDL 1.2 | `457d4e55ffe1b6ad4c4fa4559dbda8360bf8253d` |
| tinyalsa | `e43025bbf702eb7dd8edd48c1eb50530c60f1de8` |

Per ricreare `third_party` da zero si usa `scripts/fetch_sources.sh`. Lo script
si rifiuta di sovrascrivere un checkout posto su un commit diverso.
