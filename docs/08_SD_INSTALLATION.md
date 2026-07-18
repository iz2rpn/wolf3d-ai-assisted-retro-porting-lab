# 8. Installazione, aggiornamento e recupero

## Preparazione

La cartella `sdcard` contiene il layout finale. Prima di copiare:

1. formattare la nuova SD con FAT32;
2. verificare che la lettera sia davvero `D:`;
3. conservare il backup `sd_full_backup_20080411` sul PC;
4. copiare **il contenuto** di `sdcard`, non la cartella contenitore.

Da PowerShell:

```powershell
robocopy "C:\Users\iz2rp\progetti folli\lcd nova3d\reconstructed_wolf3d\sdcard" "D:\" /E /COPY:DAT /DCOPY:DAT
```

Il layout atteso è:

```text
D:\EasyUI.cfg
D:\busybox
D:\gzui\...
D:\wolf3d\wolf3d-z6s-wl1
D:\wolf3d\wolf3d-z6s-wl6
D:\wolf3d\libwolf_autostart.so
D:\wolf3d\data\VSWAP.WL1
D:\wolf3d\data\VSWAP.WL6
```

## Primo avvio

Inserire la SD a dispositivo spento. EasyUI inizializza il display; dopo circa
otto secondi parte lo script. Tastiera e altoparlante possono essere testati
separatamente. Consultare nell’ordine:

Senza tastiera, non è necessario toccare nulla: il gioco mostra il logo,
raggiunge il menu e dopo cinque secondi avvia la demo. Un tap durante quei
cinque secondi apre invece il menu utilizzabile col touch.

Poiché il set originale `.WL6` è completo, il launcher seleziona
automaticamente l’edizione registrata con tutti e sei gli episodi. I file
commerciali sulla scheda sono personali e non devono essere redistribuiti.

```text
/mnt/extsd/wolf3d/autostart.log
/mnt/extsd/wolf3d/wolf3d.log
/mnt/extsd/wolf3d/input.log
/mnt/extsd/wolf3d/audio.log
```

## Disattivazione via ADB

```sh
sh /mnt/extsd/wolf3d/disable_autostart.sh
reboot
```

Lo script crea `wolf3d/stop`, termina il gioco, riprende l’HMI e seleziona la
libreria vendor per l’avvio successivo. Per riattivare:

```sh
sh /mnt/extsd/wolf3d/enable_autostart.sh
reboot
```

## Prova singola

Con autostart disabilitato:

```sh
sh /mnt/extsd/wolf3d/test_once.sh
```

Il gioco non viene rilanciato dopo l’uscita e l’HMI viene ripresa.

## Recupero d’emergenza

Spegnere, rimuovere la SD e riaccendere. La configurazione e la libreria
interne non sono state modificate. Non rimuovere la SD mentre è in scrittura.

## Aggiornare solo la build

Ricompilare `wl1` e/o `wl6`, copiare sulla SD prima con nomi temporanei e poi,
a display spento, sostituire `wolf3d-z6s-wl1`, `wolf3d-z6s-wl6` e, se è
cambiato, `libwolf_autostart.so`. Conservare la versione precedente fino al
completamento del test hardware.
