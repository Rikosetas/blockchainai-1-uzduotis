# Maisos generatorius

256 bitu maisos funkcija, parasyta C++ kalba. Versija v0.1 (pradine).
Isvestis: 64 hex skaitmenys. Algoritmas savas, ne standartines maisos kopija.

## Kompiliavimas

Visual Studio: atidaryti maisa/maisa.sln, Release / x64, Ctrl+F5.
g++: `g++ -O2 -std=c++17 maisa/maisa/maisa.cpp -o maisa`

## Naudojimas

```
maisa -f <failas>     failo turinio maisa (tikslus baitai)
maisa -t <tekstas>    teksto (UTF-8) maisa, be naujos eilutes
maisa -i              rankinis ivedimas (Enter naujos eilutes neitraukia)
```

## Algoritmas (v0.1)

4 lankai po 64 bitus. Kiekvienas baitas pozicijoje i maisomas lanke i mod 4:
h[lane] = (h[lane] XOR baitas) * P. Gale iterpiamas ilgis. Kodas: maisa/maisa/myhash.h.

Zinoma silpnybe: vieno baito pokytis paveikia tik viena is keturiu lanku, todel lavina
silpna. Tai bus taisoma v0.2.
