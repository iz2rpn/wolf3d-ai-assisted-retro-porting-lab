# 10. Metodo per compilare altri giochi open source

## 1. Separare motore e dati

Prima di compilare, identificare:

- licenza del codice;
- licenza e formato degli asset;
- dipendenze grafiche/audio/input;
- architetture già supportate.

Un motore open source non rende automaticamente liberi ROM, WAD, PAK o dati
commerciali. Preferire giochi con shareware ufficiale o asset liberi.

## 2. Cercare un port Linux/SDL esistente

Portare il codice DOS originale di Wolf3D avrebbe richiesto riscrivere VGA,
assembly x86 e interrupt. Wolf4SDL aveva già isolato la piattaforma. Per un
nuovo gioco cercare una base SDL 1.2, framebuffer Linux o software renderer,
evitando motori che richiedono OpenGL ES moderno.

## 3. Definire il contratto del target

Riutilizzare questa scheda:

```text
CPU: ARMv5TE, soft-float
OS: Linux 3.10.65
libc app standalone: irrilevante se link statico
video: /dev/fb0, 480x272x32
input: evdev
audio: ALSA pcmC0D0p
budget consigliato: 320x200, 11/22 kHz, pochi thread
```

## 4. Costruire prima un binario statico minimo

Compilare un `--help` o una schermata piena, verificare con `readelf`, poi
aggiungere dati, input e audio. Se si aggiungono tutte le periferiche insieme,
un blocco nero non dice quale sottosistema è guasto.

## 5. Creare quattro adapter piccoli

Mantenere separati:

1. `platform_video` — modalità, pitch, pixel format, present;
2. `platform_input` — evdev e mapping;
3. `platform_audio` — formato PCM e pump;
4. `platform_boot` — launcher e recupero.

Il motore deve conoscere il meno possibile del dispositivo. Le macro target
vanno limitate ai punti d’integrazione, come `Z6S_TARGET` in questo progetto.

## 6. Rendere ogni modifica riproducibile

- fissare commit, non soltanto branch;
- salvare patch applicabili con `git apply`;
- mettere flag e lista sorgenti in uno script;
- verificare ABI automaticamente;
- annotare test riusciti e test ancora hardware.

## 7. Ottimizzare in ordine

1. ridurre risoluzione e profondità solo se necessaria;
2. ridurre sample rate e canali;
3. rimuovere backend inutili al link;
4. preferire main loop semplice a thread su firmware ridotti;
5. misurare CPU/RAM;
6. solo dopo intervenire sugli algoritmi del gioco.

## 8. Packaging sempre recuperabile

Non sostituire librerie in `/res` o `/bin`. Usare la SD, una configurazione
esterna e uno script di stop. Ogni port dovrebbe poter essere disabilitato via
ADB e semplicemente rimuovendo la SD a dispositivo spento.

## Checklist finale

- [ ] ELF ARMv5 soft-float corretto
- [ ] niente dipendenze dinamiche non disponibili
- [ ] dati legalmente posseduti e profilo corretto
- [ ] avvio QEMU o test `--help`
- [ ] framebuffer visibile e pitch rispettato
- [ ] tastiera rilevata in `/proc/bus/input/devices`
- [ ] touch disabilitabile
- [ ] PCM apre senza underrun continuo
- [ ] HMI e governor ripristinati in uscita
- [ ] procedura di recupero provata
