# Nazwy zachowane w oryginalnych plikach PAL

Przegląd z 2026-11-03 obejmuje `assets/PAL/main.exe` oraz wszystkie 950 wpisów
`disc/PAL/R4.BIN`. Sprawdzono jawne ciągi ASCII i kandydatów UTF-16LE pod kątem
nazw źródeł, ścieżek, identyfikatorów wersji, asercji i diagnostyki.
Adresy poniżej są adresami VRAM; indeksy wpisów R4.BIN liczone są od zera.
Adres overlaya wymaga zawsze podania numeru modułu.

## Potwierdzone oryginalne nazwy plików źródłowych

Trzy nazwy występują dosłownie w identyfikatorach `$Id:` w mainie.
Przypisanie do części SDK wynika z otaczających komunikatów; same ciągi nie
podają ścieżki katalogowej ani granic oryginalnych jednostek translacji.

| Nazwa | Adres ciągu | Offset w EXE | Rewizja | Data w identyfikatorze | Kontekst |
|---|---|---|---|---|---|
| `intr.c` | `0x80011808` | `0x2008` | `1.75` | `1997/02/07 09:00:36` | Przerwania, timeout, DMA; obszar libetc |
| `bios.c` | `0x80011AD0` | `0x22D0` | `1.86` | `1997/03/28 07:42:42` | `CD_sync`, `CD_ready`, `CD_init`; obszar libcd |
| `sys.c` | `0x80011DB8` | `0x25B8` | `1.140` | `1998/01/12 07:52:27` | `ResetGraph`, `SetGraphDebug`, kolejka GPU; obszar libgpu |

Pełna treść identyfikatorów:

```text
$Id: intr.c,v 1.75 1997/02/07 09:00:36 makoto Exp $
$Id: bios.c,v 1.86 1997/03/28 07:42:42 makoto Exp yos $
$Id: sys.c,v 1.140 1998/01/12 07:52:27 noda Exp yos $
```

To daty zapisane w identyfikatorach rewizji, nie ustalone daty kompilacji gry.
`bios.c` w tym miejscu odnosi się do kodu obsługi CD, nie do obrazu BIOS konsoli.

## Identyfikatory z diagnostyki Namco i overlayów

Poniższe nazwy są zachowane dosłownie. Nie mają rozszerzeń źródeł: nie stanowią
dowodu istnienia plików o takich nazwach. Komunikaty `job_*` i `strNext()` mogą
wskazywać oryginalne nazwy procedur lub zadań, ale identyczność nazwy z funkcją
wypisującą komunikat wymaga osobnego potwierdzenia. W tabeli pominięto końcowe
znaki nowej linii.

| Overlay | Adres ciągu | Treść | Użycie / dowód w odtworzonym kodzie |
|---|---|---|---|
| 660 | `0x8011487C` | `job_ta_crs_info0` | [course_menu.c](src/overlays/PAL/time_attack_course_select/course_menu.c): przekazanie `time_attack_course_select_title` do `DebugPrintStub` |
| 661 | `0x80114888` | `job_vs_car_info0` | [func_80115DB8.c](src/overlays/PAL/versus_select/func_80115DB8.c): wypisanie `versus_select_car_info_job_str` |
| 662 | `0x8011487C` | `garage0` | [func_80114B9C.c](src/overlays/PAL/garage/func_80114B9C.c): wypisanie `garage_scene_name` |
| 662 | `0x80114888` | `job_garage_car_info0` | [func_801153B0.c](src/overlays/PAL/garage/func_801153B0.c): wypisanie `garage_car_info_scene_name` |
| 662 | `0x801148F0` | `enter SaveBack` | [func_8011A898.c](src/overlays/PAL/garage/func_8011A898.c): komunikat wejścia do operacji |
| 662 | `0x80114900` | `enter LoadBack` | [func_8011A988.c](src/overlays/PAL/garage/func_8011A988.c): komunikat wejścia do operacji |
| 663 | `0x8011487C` | `recode0` | [data.c](src/overlays/PAL/records/data.c): `records_scene_name`; zachowano oryginalną pisownię `recode` |
| 665 | `0x801148B4` | `Screen:%d, %d` | [func_80115798.c](src/overlays/PAL/options/func_80115798.c): wypisanie współrzędnych ekranu |
| 666 | `0x8011487C` | `job_ex_crs_info0` | [course_menu.c](src/overlays/PAL/extra_course_select/course_menu.c): wypisanie `extra_course_select_scene_name` |
| 668 | `0x8011487C` | `time out in strNext() %d` | [func_80114C3C.c](src/overlays/PAL/movie_playback/func_80114C3C.c): diagnostyka po nieudanym pobraniu danych odtwarzania STR |

Wpisy 669, 670, 671 i 672 są kopiami odpowiednio 660, 659, 661 i 667.
Powtórzone tam ciągi nie są niezależnymi znaleziskami.

## Diagnostyka bibliotek

Te ciągi dostarczają nazw operacji i wskazówek przy rozpoznawaniu kodu SDK,
lecz nie ujawniają dodatkowych nazw plików źródłowych. `\n` oznacza rzeczywisty
znak nowej linii w oryginale.

| Obraz | Adres | Ciąg |
|---|---|---|
| main | `0x80011698` | `SPU:T/O [%s]\n` |
| main | `0x800117F8` | `VSync: timeout\n` |
| main | `0x8001183C` | `unexpected interrupt(%04x)\n` |
| main | `0x80011878` | `DMA bus error: code=%08x\n` |
| main | `0x800118A8` | `CdInit: Init failed\n` |
| main | `0x80011B88` | `CdSearchFile: searching %s...\n` |
| main | `0x80011DF0` | `ResetGraph:jtb=%08x,env=%08x\n` |
| main | `0x80011E24` | `SetGraphDebug:level:%d,type:%d reverse:%d\n` |
| main | `0x80011F98` | `GPU timeout:que=%d,stat=%08x,chcr=%08x,madr=%08x\n` |
| main | `0x80011FD8` | `card read error\n` |
| 668 | `0x801148C0` | `MDEC_rest:bad option(%d)\n` |
| 668 | `0x801148DC` | `MDEC_in_sync` |
| 668 | `0x801148EC` | `MDEC_out_sync` |
| 668 | `0x801148FC` | `%s timeout:\n` |

Pisownia `MDEC_rest` jest zgodna z oryginalnym ciągiem; nie poprawiono jej na
domyślne `MDEC_reset`. `SIO console` pod `0x800120F8` jest zachowaną etykietą,
ale sam napis nie potwierdza obecności dostępnej konsoli debugowej w grze.

## Nazwy plików danych i zapisów

| Obraz | Adres | Nazwa | Rodzaj |
|---|---|---|---|
| main | `0x80010048` | `\R4.BIN;1` | Ścieżka ISO9660 kontenera danych |
| main | `0x800A0784`, `0x800A07A4` | `\R4.STR;1` | Ścieżka ISO9660 materiału STR, dwie kopie |
| main | `0x800115A4` | `BESCESP01706RR4MARK3` | Identyfikator pliku zapisu |
| main | `0x800115BC` | `BESCESP01706RR4MARK2` | Identyfikator pliku zapisu |
| main | `0x800115D4` | `BESCESP01706RR4MARK1` | Identyfikator pliku zapisu |

## Granice ustaleń

Nie znaleziono wiarygodnych nazw plików źródłowych Namco, pełnych ścieżek do
drzewa źródeł ani komunikatów asercji łączących plik źródłowy z numerem linii.
Krótkie dopasowania w rodzaju `P.R.C` i fragmenty teksturopodobnych danych
odrzucono jako dowód nazwy źródła. Kandydaci UTF-16LE nie dostarczyli kolejnych
wiarygodnych nazw.

Przegląd dotyczy jawnych ciągów w dwóch wskazanych plikach; nie dowodzi braku
takich informacji wewnątrz skompresowanych zasobów. Nie zmieniono nazw funkcji,
plików C ani granic TU na podstawie tych znalezisk.
