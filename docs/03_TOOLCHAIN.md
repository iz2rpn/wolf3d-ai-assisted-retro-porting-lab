# 3. Toolchain riproducibile

## Perché WSL

La toolchain deve produrre ARM EABI soft-float e collegare staticamente glibc.
Ubuntu fornisce direttamente `arm-linux-gnueabi`. WSL evita dipendenze dal PATH
di Windows e rende gli stessi comandi riutilizzabili su una macchina Linux.

Ambiente usato nella ricostruzione:

```text
WSL: Ubuntu 26.04 LTS
GCC cross: 15.2.0
binutils: 2.46
target: arm-linux-gnueabi
```

## Installazione

Da PowerShell amministrativo solo per la prima installazione:

```powershell
wsl --install -d Ubuntu-26.04 --no-launch
wsl -d Ubuntu-26.04 -u root -- apt-get update
wsl -d Ubuntu-26.04 -u root -- apt-get install -y gcc-arm-linux-gnueabi g++-arm-linux-gnueabi binutils-arm-linux-gnueabi libc6-dev-armel-cross make autoconf file patch git ca-certificates
```

QEMU è facoltativo ma consigliato:

```powershell
wsl -d Ubuntu-26.04 -u root -- apt-get install -y qemu-user
```

## Flag fondamentali

```text
-march=armv5te
-mtune=arm926ej-s
-mfloat-abi=soft
-Os
-ffunction-sections -fdata-sections
-Wl,--gc-sections
-static
```

`-Os` riduce codice e pressione sulla cache. Le sezioni separate permettono al
linker di eliminare funzioni non usate. Il link statico evita di dipendere dalle
librerie musl presenti nel dispositivo.

## Nota sui percorsi con spazi

Il progetto si trova in `progetti folli/lcd nova3d`. Il vecchio libtool di SDL
1.2 non sa configurarsi sotto un percorso con spazi. `build_z6s.sh` crea quindi
un alias e la build SDL sotto `/var/tmp/z6s-wolf3d-UID`; sorgenti, oggetti Wolf e
output restano nel progetto. Non è un workaround manuale da ripetere.

## Sorgenti puliti

Per un nuovo clone, eseguire da WSL:

```bash
bash scripts/fetch_sources.sh
```

Il clone viene effettuato con `core.autocrlf=false`. Il build normalizza inoltre
gli script autoconf di SDL se provengono da un checkout Windows CRLF.
