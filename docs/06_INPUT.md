# 6. Tastiera USB e touch

## Tastiera

Il backend apre in non-blocking `/dev/input/event0`…`event15`, legge le
capability con `EVIOCGBIT` e riconosce una tastiera dalla presenza di `KEY_A` e
`KEY_ENTER`. Ripete la scansione ogni due secondi: si può collegare la tastiera
dopo l’avvio e si recupera automaticamente una disconnessione.

I tasti Linux vengono convertiti in eventi SDL 1.2 nello stesso thread del
gioco. Non ci sono thread, mutex o demoni esterni.

Comandi Wolf3D utili:

| Azione | Tasto |
|---|---|
| movimento | frecce |
| fuoco | Ctrl |
| apri/usa | Spazio |
| corsa | Shift |
| strafe | Alt + freccia |
| armi | 1…4 |
| menu/indietro | Esc |
| conferma | Invio |

Il log di rilevamento è `/mnt/extsd/wolf3d/input.log`.

## Avvio senza tastiera

Prima di ogni avvio il launcher cerca un handler `kbd` in
`/proc/bus/input/devices`:

- se lo trova, imposta `Z6S_DEMO_MODE=0` e il logo attende un input;
- se non lo trova, imposta `Z6S_DEMO_MODE=1`: logo per tre secondi, menu
  principale per cinque secondi, poi demo automatica;
- un input durante la visualizzazione temporizzata apre il menu interattivo.

Il motore continua comunque a scandire gli `event*` ogni due secondi. Una
tastiera collegata in ritardo viene quindi acquisita senza riavviare il gioco.

## Limite hardware USB

Il software è pronto per una tastiera evdev, ma la porta fisica deve operare in
modalità host e alimentare il dispositivo. ADB usa la direzione opposta. Se la
tastiera non appare, controllare via ADB:

```sh
cat /proc/bus/input/devices
ls -l /dev/input/event*
dmesg | tail -80
```

Se non compare un nuovo input device, non è un bug della mappatura Wolf3D:
servono la porta/cavo OTG corretti o un hub alimentato.

## Touch menu

Il touch viene riconosciuto dalle capability assolute `ABS_X/Y` oppure
`ABS_MT_POSITION_X/Y`. Minimo e massimo sono letti con `EVIOCGABS`, quindi il
codice non fissa una risoluzione raw Goodix.

Nei menu un elemento viene scelto direttamente solo dopo un tap rilasciato,
con movimento massimo 18 pixel e durata minore di due secondi. Non vengono
simulate frecce: questo elimina le selezioni fantasma dovute al jitter.

## Touch durante il gioco

La superficie 320×200 è divisa in una griglia invisibile 4×3:

| Riga | Zona 1 | Zona 2 | Zona 3 | Zona 4 |
|---|---|---|---|---|
| alta | arma 1 | arma 2 | arma 3 | arma 4 |
| media | sinistra | avanti | destra | usa |
| bassa | Esc | fuoco | indietro | Invio |

La tastiera resta sempre disponibile. `Z6S_TOUCH_INPUT=0` disabilita solo il
touch. Per pannelli montati diversamente esistono `Z6S_TOUCH_SWAP_XY`,
`Z6S_TOUCH_INVERT_X` e `Z6S_TOUCH_INVERT_Y`.
