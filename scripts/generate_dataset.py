import random, io, sys, os

SEED = 2026
N_LINES = 1200
OUT = os.path.join(os.path.dirname(__file__), "..", "data", "dataset.txt")

FRAGMENTS = [
    "Bloku grandiniu technologijos keicia pasitikejima skaitmeniniame pasaulyje.",
    "Maisos funkcija ivesti paverciai fiksuoto ilgio santrauka - skaitmeniniu pirstu atspaudu.",
    "Kriptografija saugo duomenis nuo priesu ir uztikrina ju vientisuma.",
    "Aciū, Lietuva! Šis tekstas turi lietuviu abecelės raides: ą č ę ė į š ų ū ž.",
    "Merkle medziai leidzia efektyviai patikrinti dideliu duomenu vientisuma.",
    "Determinizmas reiskia, kad tie patys baitai visada duoda ta pacia maisos reiksme.",
    "Lavinos efektas: mazas ivesties pokytis pakeicia apie puse isvesties bitu.",
    "Kolizijos neisvengiamai egzistuoja, bet gera funkcija jas paslepia.",
    "Pirmavaizdzio atsparumas: is maisos reiksmes sunku atkurti ivesti.",
    "Darbo irodymas remiasi tinkamumu galvosukiams (puzzle friendliness).",
]

def main():
    rng = random.Random(SEED)
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with io.open(OUT, "w", encoding="utf-8", newline="\n") as f:
        for i in range(N_LINES):
            k = rng.randint(1, 3)
            line = " ".join(rng.choice(FRAGMENTS) for _ in range(k))
            f.write(f"{i+1:04d} {line}\n")
    size = os.path.getsize(OUT)
    print(f"Sukurta: {OUT}")
    print(f"Eiluciu: {N_LINES}, baitu: {size}")

if __name__ == "__main__":
    main()
