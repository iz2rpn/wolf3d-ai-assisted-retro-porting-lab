# 1. Hardware, ABI e livello di certezza

## Identità del target

I dati seguenti derivano dal backup della SD, dai log ADB e dai binari recuperati:

| Voce | Valore | Stato |
|---|---|---|
| CPU | ARM926EJ-S, ARMv5TEJ | recuperato e coerente con l’ELF |
| Kernel | Linux 3.10.65 | recuperato dal dispositivo |
| Userland | musl nel processo vendor | verificato sulle dipendenze recuperate |
| Display fisico | 480×272, 32 bpp, pitch 1920 | verificato nel vecchio port |
| Framebuffer | `/dev/fb0`, 1.044.480 byte mappati | verificato nel vecchio port |
| Touch | Goodix, allora `/dev/input/event1` | verificato nel vecchio port |
| Audio | `/dev/snd/pcmC0D0p`, 11025 Hz stereo | aperto dal vecchio port |
| Controllo | ADB via USB | verificato in precedenza |

Il framebuffer mappato è grande esattamente due frame da 480×272×4 byte. Non
bisogna però assumere che il secondo frame sia sempre selezionabile con un’API
standard: il port usa SDL fbcon e lascia al driver vendor la scansione.

## ABI da rispettare

Il gioco è costruito così:

```text
ELF32 little-endian
Machine: ARM
EABI: Version 5
Float ABI: soft-float
ISA: ARMv5TE
Link: statico
```

Un binario `armhf` non va bene: ARM926 non possiede la VFP richiesta
dall’hard-float ABI. Per questo la tripla corretta è `arm-linux-gnueabi`, non
`arm-linux-gnueabihf`.

Il proxy EasyUI è diverso dal gioco: viene caricato dentro il processo musl del
firmware. È compilato con `-nostdlib` e usa solo due simboli dinamici, `dlopen`
e `dlsym`; le operazioni di processo passano da syscall ARM dirette.

## Recupero effettuato

Il backup completo locale contiene 803 file e 47.249.216 byte. Il confronto
SHA-256 tra la sorgente recuperata e la copia locale ha dato 803 corrispondenze
su 803. Il vecchio eseguibile, i dati WL1, le risorse EasyUI e i log sono quindi
conservati fuori dal nuovo pacchetto e non sono stati sovrascritti.

## Prima prova fisica e correzioni

La prima build ricostruita è partita sul display. I log hanno confermato il
touch Goodix su `/dev/input/event1` e l’apertura di
`/dev/snd/pcmC0D0p` a 11025 Hz stereo. Sul pannello si sono però osservati una
finestra 320×200 non estesa, il nero all’avvio della demo e un errore
`File descriptor in bad state` alla prima scrittura PCM.

La build successiva mantiene una sola modalità fisica 480×272×32, presenta
logo, menu e gioco 3D tramite lo stesso scaler opaco e chiama `pcm_prepare`
prima della riproduzione, con un solo tentativo controllato di recupero audio.
Il launcher registra inoltre dimensione virtuale e profondità del framebuffer
prima di avviare il gioco.

## Ancora da verificare

- La build corretta deve essere riprovata sul display per confermare pieno
  schermo e fluidità reali.
- I pin `SPK+` e `SPK-` indicano verosimilmente un’uscita a ponte amplificata,
  ma impedenza e potenza non sono documentate nel materiale recuperato.
- Va identificata una porta realmente USB host/OTG. La presenza di ADB prova
  la modalità device/gadget, non la modalità host.
- Il nome `event1` non è fissato nel codice: il nuovo backend esamina
  `event0`…`event15`, così touch e tastiera possono cambiare numero.
