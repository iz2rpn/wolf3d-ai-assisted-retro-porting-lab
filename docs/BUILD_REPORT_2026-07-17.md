# Rapporto build — 17 luglio 2026

## Output prodotti

| Profilo | File | Byte | SHA-256 |
|---|---|---:|---|
| WL1 | `dist/z6s/wl1/wolf3d-z6s` | 1.000.384 | `D2131777479CD41DD9EEE3DAAFABE7B5D47C63282C1BA00938FA4E198932AE03` |
| WL6 | `dist/z6s/wl6/wolf3d-z6s` | 1.000.632 | `1488CCD1CD423FBE9359AB81630E52EFD4FBC9F7B7AF644D3F282FBB7D9D3931` |
| proxy | `dist/z6s/wl1/libwolf_autostart.so` | 67.856 | `0DF2C594009E7F4BE43CEFC874A88325C68A5ADC077DAD716F340507DDA1D5F3` |

## Verifiche completate

- Build WL1 e WL6 conclusi con GCC cross 15.2.0.
- Entrambi i giochi sono ELF32 ARMv5 EABI5 soft-float statici.
- Il proxy esporta i due hook EasyUI e importa solo `dlopen`/`dlsym`.
- `qemu-arm --help` riuscito per WL1 e WL6.
- WL1 e WL6 con i rispettivi dati reali e SDL dummy sono rimasti attivi per
  venti secondi ciascuno, fino al timeout intenzionale della prova.
- Entrambi stampano e usano `320x200 logical -> 480x272x32 native`; il percorso
  esercitato comprende l’avvio automatico della demo.
- Il presentatore 3D usa lo stesso scaler opaco di logo e menu, correggendo il
  nero osservato nella prima prova hardware.
- La build finale mostra il logo; senza handler tastiera mostra per cinque
  secondi il menu originale e poi entra nel ciclo automatico delle demo.
- Il launcher contiene entrambi i motori e seleziona WL6 soltanto con tutti gli
  otto file dati dell’edizione registrata.
- Tutti gli script del pacchetto passano `dash -n`.
- Le quattro patch upstream sono riproducibili; `fetch_sources.sh`
  riconosce correttamente che sono già applicate.
- Gli otto file WL6 della copia legale dell’utente sono completi, totalizzano
  2.295.389 byte e sono registrati singolarmente nel manifest.
- Il pacchetto `sdcard` contiene 401 file e 26.457.710 byte.
- Hash di gioco, proxy, BusyBox e dati principali ricontrollati dopo il package.

## Stato SD fisica

La nuova `D:` è stata identificata come unità rimovibile da 31,24 GB, già
FAT32 e vuota salvo `System Volume Information`; non è stato quindi necessario
formattarla. Dopo l’autorizzazione esplicita dell’utente è stato copiato tutto
il contenuto di `sdcard`.

La verifica finale, ripetuta dopo la correzione video/audio, ha riletto dalla
SD e confrontato SHA-256 e dimensione di tutti i file del pacchetto: 401 file,
26.457.710 byte, zero mancanti, zero dimensioni errate e zero hash differenti.
Sono stati preservati cinque file diagnostici generati sul dispositivo
(`audio.log`, `autostart.log`, `governors.before`, `input.log`, `wolf3d.log`),
che non appartengono al pacchetto sorgente.

La prima prova fisica ha confermato l’avvio del launcher, il touch Goodix e
l’apertura del PCM ALSA. Ha anche evidenziato l’immagine non a pieno schermo,
il 3D nero e un PCM da preparare esplicitamente: questi tre punti hanno guidato
la build ora presente sulla SD.

## Prove hardware pendenti

- conferma sul pannello del pieno schermo 480×272 e della fluidità della demo;
- enumerazione tastiera su una porta USB host/OTG;
- suono reale su altoparlante passivo collegato a `SPK+`/`SPK-`;
- conferma del ritorno all’HMI con `disable_autostart.sh`.
