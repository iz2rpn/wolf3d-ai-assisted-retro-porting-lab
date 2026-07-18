# 4. Build e verifica

## Profili dati

```bash
bash scripts/build_z6s.sh wl1
bash scripts/build_z6s.sh wl6
```

- `wl1`: Wolfenstein 3D shareware 1.4 (`*.WL1`), corrisponde ai dati recuperati.
- `wl6`: edizione completa (`*.WL6`). Gli otto dati originali sono stati
  forniti dall’utente dalla propria copia legale. Non basta mai rinominare WL1.

Il pacchetto SD contiene entrambi gli eseguibili. `run_wolf3d.sh` seleziona
`wolf3d-z6s-wl6` solo quando sono presenti `AUDIOHED`, `AUDIOT`, `GAMEMAPS`,
`MAPHEAD`, `VGADICT`, `VGAGRAPH`, `VGAHEAD` e `VSWAP` con estensione `.WL6`;
altrimenti avvia `wolf3d-z6s-wl1` con il set shareware completo.

## Cosa costruisce lo script

1. SDL 1.2 statico con fbcon, timer e simboli joystick richiesti dal motore.
2. Wolf4SDL con profilo versione e macro `Z6S_TARGET`.
3. Backend evdev Z6S.
4. Mixer minimale e soli moduli PCM di tinyalsa.
5. Link statico e rimozione simboli.
6. Proxy EasyUI `-nostdlib`.
7. Controlli `readelf` automatici.

## Controlli ELF attesi

```bash
arm-linux-gnueabi-readelf -h dist/z6s/wl1/wolf3d-z6s
arm-linux-gnueabi-readelf -d dist/z6s/wl1/wolf3d-z6s
arm-linux-gnueabi-readelf -Ws dist/z6s/wl1/libwolf_autostart.so
```

Il gioco deve mostrare `Version5 EABI, soft-float ABI` e **nessuna sezione
dinamica**. Il proxy deve avere come indefiniti soltanto `dlopen` e `dlsym` e
deve esportare `onEasyUIInit` e `onEasyUIDeinit`.

## Test QEMU effettuati

Il comando `qemu-arm wolf3d-z6s --help` ha eseguito correttamente il parser ARM.
I test con driver video dummy e dati reali WL1 e WL6 hanno mantenuto ciascun
profilo attivo per venti secondi. L’assenza di `/dev/snd` sotto QEMU viene
gestita in modo degradato: viene stampato un errore audio, ma dati e gioco
continuano a partire.

La variante Z6S non usa `--nowait` nello script della SD: così il logo originale
non viene saltato. `Z6S_DEMO_MODE=1`, impostato quando non è rilevata una
tastiera, limita l’attesa del logo, mostra il menu per cinque secondi e avvia
una demo; `Z6S_DEMO_MODE=0` conserva l’attesa interattiva originale.

Esempio:

```bash
cd sd_full_backup_20080411/wolf3d/data
timeout 5 env SDL_VIDEODRIVER=dummy HOME=/tmp/z6s-qemu \
  Z6S_FORCE_AUDIO=0 Z6S_FORCE_OPAQUE=0 qemu-arm \
  /percorso/dist/z6s/wl1/wolf3d-z6s \
  --resf 320 200 --bits 32 --nowait --nodblbuf --configdir /tmp/z6s-qemu
```

QEMU non valida framebuffer vendor, Goodix, USB host o amplificatore. Queste
sono prove hardware separate, descritte nei capitoli dedicati.

La prova ripetibile aggiornata è raccolta in `scripts/test_qemu.sh`. Controlla
il parser di entrambe le build e mantiene sia WL1 sia WL6 per 20 secondi in
modalità senza tastiera, intenzionalmente senza passare `--nowait`.
