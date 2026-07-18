# 7. Audio e collegamento altoparlante

## Architettura

SDL_mixer completo è sovradimensionato per questo target e il vecchio SDL non
riusciva a creare un thread audio affidabile. Il port implementa soltanto le API
usate da `id_sd.cpp`:

- caricamento WAV PCM S16;
- otto canali di effetti con panning;
- callback musica AdLib/OPL;
- gruppi e canali riservati;
- saturazione del mix a 16 bit.

`Mix_Z6S_Pump()` genera un periodo nel main loop. tinyalsa apre:

```text
/dev/snd/pcmC0D0p
11025 Hz
stereo S16 little-endian
128 frame per periodo
4 periodi
```

Il buffer runtime è 512 byte: 128 frame × 2 canali × 2 byte. Questo mantiene
bassa la latenza e limita memoria/CPU. Se il PCM manca, il gioco continua senza
audio invece di arrestarsi.

La prima prova hardware ha mostrato che il driver Linux 3.10 lasciava il PCM
nello stato `SETUP`: l’apertura riusciva, ma il primo `WRITEI` rispondeva
`EBADFD`. Il backend ora esegue `pcm_prepare()` esplicitamente dopo l’apertura e
tenta una singola preparazione di recupero dopo un errore di scrittura. Se
anche il secondo tentativo fallisce, disabilita l’audio per quella sessione:
così un driver guasto non può rallentare il rendering con errori continui.

Log: `/mnt/extsd/wolf3d/audio.log`.

## Collegamento fisico sicuro

Usare **un altoparlante passivo** tra `SPK+` e `SPK-`, rispettando l’impedenza
indicata sulla scheda o nella documentazione del modulo. Non collegare nessuno
dei due pin a GND. Non collegare cuffie, ingresso AUX o la massa di un
amplificatore esterno: un’uscita SPK+/SPK- è spesso a ponte e può danneggiarsi.

Poiché impedenza e potenza non sono ancora note, iniziare con il volume software
basso e una prova breve. Se non esiste una marcatura affidabile, misurare o
identificare prima il chip amplificatore.

## Sequenza di test

1. Avviare senza altoparlante e verificare `audio.log`.
2. Cercare la riga `ALSA opened ... rate=11025 ... period_frames=128`.
3. Spegnere completamente.
4. Collegare l’altoparlante passivo a `SPK+`/`SPK-`.
5. Riaccendere e provare prima un effetto breve, poi la musica.

Errori `pcm_writei` persistenti vengono registrati una sola volta per non
consumare la SD.
Click o musica spezzata indicano underrun: provare 1024 byte e 22050 Hz solo
dopo aver misurato il carico, perché raddoppiano lavoro e latenza.
