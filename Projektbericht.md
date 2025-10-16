# Projektaufgabe Gammakorrektur (A208)

## 1. Problemstellung

Viele Anwendungen der digitalen Bildverarbeitung erfordern Bildoptimierungsschritte. Eine wichtige Methode ist die **Gammakorrektur**, welche die Helligkeit an die nichtlineare Wahrnehmung des menschlichen Auges anpasst. Das Hauptziel dieses Projekts besteht darin, ein Programm zu entwickeln, das zunaechst Farbbilder in Graustufen umwandelt und anschliessend die Gammakorrektur durchfuehrt.

---

## 2. Loesung

Unsere Loesung wurde in **C** und **Assembler** (x86-64) umgesetzt. Die Bildbearbeitungsalgorithmen verwenden **SSE-Erweiterungen** bis SSE4.2 fuer hohe Effizienz. **I/O-Operationen** und das **Rahmenprogramm** basieren auf **C17**.

### Dateiformat

Wir verwenden als Eingabe **PPM (Typ P6)**, das 24 Bit pro Pixel speichert. Als Ausgabe waehlen wir **PGM (Typ P5)**, das fuer Graustufen geeignet ist und in vielen Programmen direkt unterstuetzt wird.

---

## Aufgabe

Ziel der Projektarbeit ist ein Assembler-Algorithmus, der ein Bild in Graustufen konvertiert und Gammakorrektur durchfuehrt. Theoretische Ansaetze aus der Mathematik werden hierbei praxisnah genutzt. Dazu werden uebliche Kamerabilder als Eingabe verwendet und in x86-64-Assembly verarbeitet.

---

## Loesungskonzept

Unsere Umsetzung besteht aus drei Teilen:

1. **Rahmenprogramm:**  
   Bietet Konsolen-Interaktion und verschiedene Optionen wie Implementierungswahl, Graustufenkoeffizienten, Gamma-Wert und Benchmarking.

2. **Graustufen-/Gammakorrekturalgorithmus:**  
   Eine Funktion in Assembly, die Farbwerte in Graustufen umwandelt und Gammakorrektur anwendet.

3. **Exponentialfunktion:**  
   Berechnet \(a^b\) in x86-64-Assembly und dient der Gammakorrektur.

---

### Rahmenprogramm

**Erforderliche Argumente:**

- **-o \<Dateiname\>**  
  Ausgabedatei  

- **--gamma \<Float\>**  
  \(\gamma \in [0, \infty)\)  

- **\<Dateiname\>**  
  Eingabedatei  


**Optionale Argumente:**

- **-V \<Zahl\>**  
  Wählt die Implementierung, **-V 0** = Hauptimplementierung  

- **-B\<Zahl\>**  
  Misst die Laufzeit, optional Anzahl der Wiederholungen, z. B. **-B5**  

- **--coeffs \<a\>,\<b\>,\<c\>**  
  Koeffizienten für die Graustufenkonvertierung  

- **-h | --help**  
  Beschreibung aller Optionen  


---

### Graustufenkonvertierung

Für die Graustufenkonvertierung wird in **gamma_correct** der folgende Algorithmus verwendet:

1. **Vorverarbeitung:**  
   Die Pixelanzahl wird so angepasst, dass sie durch 4 teilbar ist.

2. **Laden der Farbwerte:**  
   In drei **XMM-Registern** werden jeweils 4 Farbwerte geladen, sodass jedes Register eine Farbe repräsentiert:
   - **R** (Rot)  
   - **G** (Grün)  
   - **B** (Blau)

3. **Anwendung der Graustufen- und Gammakorrektur-Formeln:**  
   Mithilfe von **SIMD-Instruktionen** werden die folgenden Berechnungen durchgeführt:

   Graustufenkonvertierung:
   \[
   Q(x, y)  = \frac{a \cdot R + b \cdot G + c \cdot B}{a + b + c}
   \]
   Gammakorrektur:
   \[
   Q'(x, y) = \left(\frac{Q(x, y)}{255}\right)^\gamma \cdot 255
   \]

4. **Schreiben der Ausgabewerte:**  
   Die berechneten **4 Graustufenwerte** werden in den Ausgabepuffer geschrieben.



---

### Exponentialfunktion

Fuer die Gammakorrektur wird in **power.S** mit **Single-Precision-Floats** gearbeitet: Basis und Exponent werden uebergeben, das Ergebnis als Float zurueckgegeben. Mathematisch lässt sich das folgendermaßen begründen:

\[
c = a^b = e^{b \ln(a)} = e^{b \ln(m \cdot 2^d)} = e^{b \bigl(\ln(m) + d \ln(2)\bigr)},
\]

wobei  
- \( e \) (in der Formel) die Eulersche Zahl ist,  
- \(\ln\) der natürliche Logarithmus,  
- \(m\) die Mantisse in der Gleitkommadarstellung von \(a\),  
- und \(d\) (als Exponent) hier zur Basis-2-Darstellung gehört.

Da \(a\) sehr groß sein kann, wird es in seine Float-Darstellung zerlegt, um den Wertebereich für \(\ln(\cdot)\) einzugrenzen. Der Ausdruck \(\ln(m)\), wobei \(m \in [1, 2)\), kann effizient durch ein Minimax-Polynom approximiert werden. Die Gleitkommapräzision (etwa \(10^{-7}\)) lässt sich als Summe von 7 Termen (siehe Abschnitt „Genauigkeit“) eines Minimax-Polynoms darstellen:

\[
\ln(x+1) = a_1 x + a_2 x^2 + a_3 x^3 + \dots + a_7 x^7.
\]

Weiterhin gilt:

\[
e^k = e^{n \ln(2) + r},
\]

wobei \(n\) eine ganze Zahl ist. Dann folgt:

\[
c = e^r \times 2^n.
\]

\(e^r\) kann durch **10 Glieder** der Maclaurin-Reihe approximiert werden:

\[
e^r = 1 + r + a_1 r^2 + a_2 r^3 + a_3 r^4 + a_4 r^5 + a_5 r^6 + a_6 r^7 + a_7 r^8 + a_8 r^9.
\]

### Genauigkeit

Unser Ansatz erreicht etwa \(10^{-7}\) Genauigkeit, sofern Basis und Exponent in \((0, 20)\) liegen. Da fuer die Gammakorrektur intern nur mit **255 multipliziert** und gerundet wird, haben wir in **Version V0** die Anzahl der Terme reduziert und damit die Laufzeit um rund 25% verkuerzt. Dies fuehrt zu einem maximalen Fehler von etwa **2,8%**, was fuer viele Anwendungen hinnehmbar ist.

---

## Benchmarking

Weitere Messungen zur Laufzeit und Genauigkeit der Implementierungen folgen im Abschnitt „Benchmarking“.  

**Getestet wurde auf folgendem System:**
- Intel i5-1135G7 Prozessor (4.20 GHz)  
- 8 GB Arbeitsspeicher  
- Arch Linux x86_64  
- Linux-Surface Kernel 6.12.7  

**Kompiliert mit:**
- GCC 14.2.1  
- Compiler-Option: `-O2`

**Berechnungen:**
- PPM-Eingabedatei der Größe `5184 x 3456`

