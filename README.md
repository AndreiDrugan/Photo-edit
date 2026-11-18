# Procesor de Imagini BMP & Interpretor de Comenzi

Program scris în C pentru manipularea, compresia și procesarea imaginilor în format BMP necomprimat (24/32 biți)[cite: 1, 6]. Pixelii sunt reținuți în memorie sub forma unei matrici tridimensionale alocate dinamic (`unsigned char ***img` de dimensiuni $N \times M \times 3$), canalele de culoare fiind reprezentate în ordinea standard RGB (`0 = R`, `1 = G`, `2 = B`)[cite: 1].

---

## Funcționalități Implementate

Programul suportă atât operații interactive aplicate direct peste imaginea de test (`alpaca.bmp`), cât și un **interpretor de comenzi** care gestionează simultan mai multe imagini în memorie pe bază de etichete (`[tag]`) citite dintr-un fișier text (`command.txt`):

1. **Bounding Box (Opțiunea 1)**
   - Desenează un contur dreptunghiular de culoare roșie (`RGB: 255, 0, 0`) delimitat de punctele opuse $(x_1, y_1)$ și $(x_2, y_2)$[cite: 1, 2].
   - Conține verificări de limită (`clamping`) pentru a preveni accesările în afara matricii dacă sunt introduși indici negativi sau mai mari decât dimensiunile imaginii.

2. **Crop / Decupare (Opțiunea 2)**
   - Trunchiază imaginea la dreptunghiul delimitat de coordonatele specificate de utilizator[cite: 1, 2].
   - Alocă o nouă matrice de dimensiuni corespunzătoare ($new\_N \times new\_M \times 3$), copiază pixelii vizați și eliberează complet din memorie vechea matrice.

3. **Filtru Grayscale (Opțiunea 3)**
   - Convertește imaginea în nuanțe de gri prin înlocuirea canalelor $(R, G, B)$ ale fiecărui pixel cu media lor aritmetică $\left\lfloor \frac{R+G+B}{3} \right\rfloor$[cite: 1, 2].

4. **Format Pătrat / Padding Margini Albe (Opțiunea 4)**
   - Adaptează imaginea pentru un format pătrat (tipic pentru postări Instagram)[cite: 1].
   - Calculează diferența dintre înălțime și lățime și adaugă simetric benzi albe (`RGB: 255, 255, 255`) pe axa mai scurtă pentru a egala proporțiile[cite: 1, 2].

5. **Paste / Suprapunere Imagini (Opțiunea 5)**
   - Lipește o imagine sursă peste o imagine destinație pornind de la coordonatele $(x, y)$, realizând decupare automată (_clipping_) dacă imaginea lipită depășește marginile cadrului de bază.

6. **Interpretor de Comenzi & Compresie RLE (Opțiunea 6)**
   - Citește și execută secvențial comenzi din `command.txt` asupra mai multor imagini identificate prin tag-uri (`Image images[100]`).
   - Include codificare și decodificare binară **Run-Length Encoding (RLE)** pentru compresia secvențelor consecutive de pixeli identici.

---

## Structura Proiectului

- `main.c` – Conține funcțiile I/O pentru fișiere BMP (`bmp_read`, `bmp_write`), funcțiile de prelucrare grafică, compresia RLE (`rle_encode`, `rle_decode`) și parserul de comenzi.
- `alpaca.bmp` – Imaginea de intrare implicită utilizată pentru testarea operațiilor interactive[cite: 1, 2].
- `command.txt` – Fișierul text cu lista de instrucțiuni executate de interpretor (Opțiunea 6).
- `output.bmp` – Imaginea rezultată în urma rulării operațiilor interactive (Opțiunile 1–5)[cite: 2].

---

## Compilare și Rulare

Pentru compilare folosind `gcc`:

```bash
gcc -Wall -Wextra -std=c11 main.c -o procesor_imagini
```
