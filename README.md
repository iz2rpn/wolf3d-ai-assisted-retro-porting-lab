# Wolf3D per NOVA3D Z6S — progetto ricostruito

Questo repository ricostruisce in modo riproducibile il port di Wolfenstein 3D
per il display NOVA3D Z6S. Il target è Linux ARMv5 soft-float, framebuffer
480×272, input evdev e uscita ALSA. Il gioco renderizza a 320×200 per contenere
CPU e RAM.

## Stato

- **Verificato su PC:** sorgenti fissati a commit precisi, cross-compilazione,
  ELF32 ARM EABI5 soft-float statico, avvio e caricamento dati WL1 sotto QEMU.
- **Recuperato dal dispositivo precedente:** framebuffer, touch Goodix e PCM
  ALSA erano funzionanti con la precedente build.
- **Da verificare sul display:** nuova build, altoparlante fisico e modalità
  USB host/OTG per la tastiera. Il connettore usato per ADB potrebbe essere solo
  USB gadget e non alimentare una tastiera.

Dopo la prima prova hardware è stato corretto il percorso video: il motore
renderizza sempre a 320×200, mentre un presentatore Z6S scala ogni fotogramma
all’unica modalità fisica 480×272×32. Logo, menu e scena 3D usano ora lo stesso
percorso, senza cambi di risoluzione e senza il nero durante le demo.

## Compilazione rapida

Da PowerShell, dopo aver installato WSL Ubuntu 26.04:

```powershell
wsl -d Ubuntu-26.04 -u root -- apt-get update
wsl -d Ubuntu-26.04 -u root -- apt-get install -y gcc-arm-linux-gnueabi g++-arm-linux-gnueabi binutils-arm-linux-gnueabi libc6-dev-armel-cross make autoconf file patch git
wsl -d Ubuntu-26.04 -u root -- bash "/mnt/c/Users/iz2rp/progetti folli/lcd nova3d/reconstructed_wolf3d/scripts/build_z6s.sh" wl1
```

`wl1` produce il binario per i dati shareware recuperati. `wl6` compila il
profilo completo. Il pacchetto locale contiene ora gli otto file WL6 forniti
dall’utente dalla propria copia originale legale.

## Sequenza di avvio sul display

- Con tastiera USB rilevata: appare il logo originale e un tasto prosegue verso
  le schermate e il menu originali.
- Senza tastiera: il logo resta visibile per tre secondi, compare il vero menu
  principale per cinque secondi e poi parte automaticamente una demo. Al
  termine, il ciclo menu/demo continua.
- Un tasto o un tocco durante l’attesa del menu lo rende subito interattivo;
  la tastiera può essere collegata anche dopo l’avvio.

Il launcher sceglie `WL6` perché trova tutti gli otto file dati `.WL6`; se in
futuro uno mancasse, tornerebbe ai dati shareware `WL1` senza rinominarli o
mescolare edizioni incompatibili.

Output:

- `dist/z6s/wl1/wolf3d-z6s`
- `dist/z6s/wl6/wolf3d-z6s`
- `dist/z6s/wl1/libwolf_autostart.so`
- `sdcard/`, immagine logica pronta da copiare sulla SD

## Documentazione

1. [Hardware e prove](docs/01_HARDWARE_E_PROVE.md)
2. [Architettura e sorgenti](docs/02_ARCHITETTURA.md)
3. [Toolchain](docs/03_TOOLCHAIN.md)
4. [Build e verifica](docs/04_BUILD_E_VERIFICA.md)
5. [Video](docs/05_VIDEO.md)
6. [Tastiera e touch](docs/06_INPUT.md)
7. [Audio e altoparlante](docs/07_AUDIO.md)
8. [Installazione e recupero SD](docs/08_INSTALLAZIONE_SD.md)
9. [Debug e ottimizzazione](docs/09_DEBUG_E_PERFORMANCE.md)
10. [Metodo per altri giochi](docs/10_PORTING_ALTRI_GIOCHI.md)

I sorgenti originali sono conservati separatamente in `third_party`. Wolf4SDL
è il port moderno usato per costruire il gioco; il repository ufficiale id è
incluso come riferimento storico. I dati commerciali WL6 presenti in questa
copia di lavoro appartengono all’utente: non pubblicare né redistribuire la
cartella `WOLF3D` o il pacchetto `sdcard/wolf3d/data`.
