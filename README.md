# Procesor de Imagini BMP

Program scris în C pentru manipularea și aplicarea de transformări geometrice și încadrări pe imagini în format BMP necomprimat (24/32 biți). Pixelii sunt reținuți în memorie sub forma unei matrici tridimensionale alocate dinamic (`unsigned char ***img` de dimensiuni $N \times M \times 3$), canalele de culoare fiind reprezentate în ordinea standard RGB (0 = R, 1 = G, 2 = B).

---

## Funcționalități Implementate

Programul citește imaginea `alpaca.bmp`, așteaptă selecția unei operații din consolă și salvează rezultatul în `output.bmp`:

1. **Bounding Box (Opțiunea 1)**
   * Desenează un contur dreptunghiular de culoare roșie (`RGB: 255, 0, 0`) delimitat de punctele opuse $(x_1, y_1)$ și $(x_2, y_2)$.
   * Conține verificări de limită (`clamping`) pentru a preveni accesările în afara matricii dacă sunt introduși indici negativi sau mai mari decât dimensiunile imaginii.

2. **Crop / Decupare (Opțiunea 2)**
   * Trunchiază imaginea la dreptunghiul delimitat de coordonatele specificate de utilizator.
   * Alocă o nouă matrice de dimensiuni corespunzătoare ($new\_N \times new\_M \times 3$), copiază pixelii vizați și eliberează complet din memorie vechea matrice.

3. **Format Pătrat / Padding Margini Albe (Opțiunea 4)**
   * Adaptează imaginea pentru un format pătrat (tipic pentru postări Instagram).
   * Calculează diferența dintre înălțime și lățime și adaugă simetric benzi albe (`RGB: 255, 255, 255`) pe axa mai scurtă pentru a egala proporțiile.

---

## Structura Proiectului

* `Proiect_poze.c` – Conține logica funcțiilor I/O pentru fișiere BMP (`bmp_read`, `bmp_write`), funcțiile de prelucrare grafică și funcția `main` pentru dirijarea comenzilor.
* `alpaca.bmp` – Imaginea de intrare utilizată pentru testare.
* `output.bmp` – Imaginea rezultată în urma rulării operației alese.

---

## Compilare și Rulare

Pentru compilare folosind `gcc`:

```bash
gcc -Wall -Wextra -std=c11 Proiect_poze.c -o procesor_imagini
```

Rularea programului:

```bash
./procesor_imagini
```

---

## Ghid de Utilizare

La rulare, introduceți numărul operației dorite și valorile necesare:

* **Pentru Bounding Box (`1`):**  
  Introduceți `1`, apoi coordonatele punctelor opuse: `x1 y1 x2 y2` (unde $x$ corespunde coloanei și $y$ liniei).  
  *Exemplu:*  
  ```text
  1
  100 50 300 250
  ```

* **Pentru Decupare / Crop (`2`):**  
  Introduceți `2`, urmat de coordonatele zonei de decupat: `x1 y1 x2 y2`.  
  *Exemplu:*  
  ```text
  2
  50 50 200 200
  ```

* **Pentru Format Pătrat (`4`):**  
  Introduceți direct opțiunea `4` (marginile albe se adaugă automat pe latura mai scurtă).