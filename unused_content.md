# Ridge Racer Type 4 (PAL, `SCES-01706`) — Nieużywane funkcje i wycięty kontent

Pełna analiza krzyżowa przeprowadzona na pliku głównym `assets/PAL/main.exe` (`0x80010000..0x800AC000`), wszystkich **9 unikalnych overlayach wykonywalnych** w `disc/PAL/R4.BIN` (`659, 660, 661, 662, 663, 665, 666, 667, 668`, ładowanych pod `0x80114878+`), wszystkich 950 wpisach kontenera `R4.BIN`, pliku `disc/PAL/FILE.DAT` oraz zdekompilowanym kodzie źródłowym w `src/`.

## Podsumowanie liczbowe martwego kodu

Każda funkcja w `configs/PAL/functions.txt` (1341 funkcji w `main.exe`) oraz w manifestach overlayów `configs/PAL/overlays/` (343 funkcje w overlayach) została sprawdzona pod kątem:
- bezpośrednich wywołań i skoków (`jal`, `j`, rozgałęzienia międzyfunkcyjne),
- pośredniego ładowania adresów (`lui` + `addiu`/`ori` w `.text` oraz w overlayach),
- 32-bitowych wskaźników w `.text`, `.rodata`, `.data` i we wszystkich obrazach overlayów,
- przejścia przez *fall-through* z poprzedzającej funkcji (np. `FloodFill4bpp` -> `FloodFill4bppStep`),
- przechodniej eliminacji martwego kodu (funkcje wywoływane wyłącznie przez inne martwe funkcje).

| Kategoria kodu | Funkcje z 0 odniesień | Funkcje martwe przechodnio | Funkcje w martwych slotach tablic | Łącznie martwych funkcji |
| :--- | :---: | :---: | :---: | :---: |
| **Rezydentny kod gry Namco** (`0x80012208..0x8007D718`, z 801) | **52** | **+4** (`ValidateRecordPassword`, `CodeCharsToByte`, `UpdateVisibleViewRecords`, `QuadOffscreenTestVerts1`) | **+3** (`LoadMenuSoundStep`, `LoadMenuSoundVabStep`, `MenuSceneNoop`) | **59** |
| **Kod overlayów Namco** (`0x80114878+`, z 343 w 9 overlayach) | **13** | **+1** (`ovl668_mdec_read_status`) | **0** *(+ 2 zepsute wskaźniki w `ModeGroup1FuncTable[19..20]`)* | **14** |
| **Biblioteki SDK Sony PsyQ** (`0x8007D718..0x8009DF14`, z 540) | **127** | **+64** | **0** | **191** |
| **Razem (`main.exe` + 9 overlayów)** | **192** | **+69** | **+3** | **264** |

---

## 1. Wycięty kontent i nieużywane zasoby na płycie (`R4.BIN` i `FILE.DAT`)

### 1.1 Kompletna, zlokalizowana na Europę aplikacja PocketStation („R4 Garage Applet”), nadal ładowana do RAM-u (`R4.BIN` wpis `664`)

Wpis **664** (`GetR4FileIndex(1, 6)`, sektory `22670..22678`, rozmiar **`16 384` bajty** = `0x4000`, czyli 2 bloki karty pamięci PS1) w `disc/PAL/R4.BIN` nie jest overlayem MIPS — to **kompletny program wykonywalny `MCX0` (ARM7TDMI) na PocketStation**:

- **Nagłówek karty pamięci i tytuł Shift-JIS (`0x00..0x43`)**:
  - `0x00..0x03`: `53 43 11 02` (`"SC\x11\x02"` — sygnatura nagłówka PS1 Memory Card: magic `"SC"`, flaga ikony `0x11` = 1 statyczna klatka ikony, rozmiar `0x02` = 2 bloki karty pamięci).
  - `0x04..0x43`: 64-bajtowy tytuł zapisany w pełnoszerokim kodowaniu Shift-JIS:
    ```text
    82 6f 82 8f 82 83 82 8b 82 85 82 94 82 72 82 94 82 81 82 94 82 89 82 8f 82 8e 81 40
    82 71 82 53 81 40 82 66 82 81 82 92 82 81 82 87 82 85 81 40 82 60 82 90 82 90 82 8c
    82 85 82 94 81 40 81 40
    ```
    - **Transkrypcja Shift-JIS**: **`「ＰｏｃｋｅｔＳｔａｔｉｏｎ　Ｒ４　Ｇａｒａｇｅ　Ａｐｐｌｅｔ　　」`**
    - **Odpowiednik ASCII**: **`"PocketStation R4 Garage Applet  "`**
- **Nagłówek `MCX0` i ikony (`0x52..0x1FF`)**:
  - `0x52..0x5F`: `4d 43 58 30 01 00 00 00 00 00 00 02 00 02` (`"MCX0"` — nagłówek programu wykonywalnego PocketStation).
  - `0x60..0xFF`: 16-kolorowa paleta CLUT (`0x60..0x7F`) oraz 4-bitowa ikona 16×16 (`0x80..0xFF`) dla menu karty pamięci PS1 z napisem **„R4”** u góry i **„PDA”** na dole.
  - `0x180..0x1FF`: Monochromatyczna 1-bitowa ikona 32×32 na ekran LCD PocketStation z napisem **„R4 / Pocket / Station”**:
    ```text
     3: .###########.....############...
     4: .############...#############...
     5: .#####....####.#####....#####...
     8: .#############.################.
    10: .#####.######...........#####...
    12: .#####...######.........#####...
    15: .....###.........#........#.....
    17: .....#.#.###.###.#.#.###..#.....
    19: .....###.#.#.#...##..###..#.....
    21: .....#...###.###.#.#.###..##....
    23: ....###..#.......#..#...........
    25: ....#....#..###..#..#.###.##....
    27: ......#..#..###..#..#.#.#.#.#...
    29: ....###..##.###..##.#.###.#.#...
    ```
- **Kod ARM7TDMI zlokalizowany dla wersji PAL (`0x200..0x39B8`)**:
  - Zawiera skompilowany kod maszynowy ARM7TDMI pozwalający przeglądać i wymieniać przez podczerwień 320 samochodów z garażu R4.
  - Pod offsetem **`0x1D68`** wewnątrz obrazu `664` zaszyty jest europejski identyfikator głównego zapisu gry **`"BESCESP01706RR4_GAME"`** — dowodzi to, że Namco specjalnie przekompilowało binarkę ARM apletu PocketStation pod europejskie wydanie `SCES-01706`, zanim Sony odwołało premierę PocketStation w Europie.
- **Jak `main.exe` w wersji PAL nadal ładuje i obsługuje wpis `664`**:
  1. Gdy gracz wybiera **„SAVE AND LOAD”** (`MainMenuChoice == 5`) w menu głównym, funkcja `LoadOpeningMovieTimStep` w `src/main/PAL/game/data/ldscn1.c` wywołuje `CdFileReadStart(GetR4FileIndex(1, LoadIndex + 1), LoadCursor)` dla `LoadIndex = 5`, co **za każdym razem wczytuje wpis `664` (`16 384` B) z płyty CD do `CdDirectoryBuffer` (`0x80114878`)**. Ponieważ menu karty pamięci znajduje się w rezydentnym `main.exe` (nie wymaga overlaya), bufor `CdDirectoryBuffer` służył jako bufor tymczasowy na obraz apletu PocketStation.
  2. W sekcji `.data` `main.exe` pod adresem `0x800A45A0` nadal znajduje się `CardFileNameTemplate = "BESCESP01706RR4_PDA"` (obok `CardSaveFileName = "BESCESP01706RR4_GAME"` pod `0x800A457C`).
  3. W `src/main/PAL/game/menu/menumode.c` (`StepCardFileSlotSelection`), dla `MenuStateIndex == 0x3A`, kod nadal ustawia:
     ```c
     CardPdaModeFlag = 1;
     CardTransferMode = 0x20;
     CopyStringClearTail(CardTargetFileName, CardFileNameTemplate);
     ```
  4. W `src/main/PAL/game/menu/savemenu.c` (`CardSaveStep`), gdy `CardTransferMode != 0x10` (czyli `0x20` dla PDA), funkcja nadal kopiuje `0x4000` bajtów apletu z `CdDirectoryBuffer` do `SaveBuffer` i ustawia zapis 2 bloków (`CardSaveBlockCount = 2`):
     ```c
     *(CardFileImage4000 *)SaveBuffer = *(const CardFileImage4000 *)CdDirectoryBuffer;
     ```
  5. **Sposób wycięcia w UI**: W `src/main/PAL/game/menu/loadmenu.c` (`CardMenuModeEnter`) menu karty pamięci inicjalizuje tylko 4 wiersze UI (`0x22..0x25`: Save, Load, Delete, Exit) zamiast 5, a w `src/main/PAL/game/menu/menustep.c` (`GetCardMenuInitialState` i `StepCardMenuStateMachine`) przypadek `CardMenuOperation == 4` został zastąpiony pustym `case 4: break;`.

---

### 1.2 8 nieużywanych tabel AI przeciwników dla trybu Extra Trial (`R4.BIN` wpisy `830, 832, 834, 836, 838, 840, 842, 844`)

W `src/main/PAL/game/data/trkld1.c` funkcja `LoadCarSpecStep(int a0, int a1, int a2)` ładuje 2048-bajtową tabelę parametrów AI (`RaceSpecEntry30`) z kategorii 2 (`0x1E..0x7B`, wpisy `759..852`):
- `case 1` (Grand Prix, `0x1E..0x43` = wpisy `759..796`, 38 plików): indeksowane przez `CourseAssetGroupIds[CourseIndex] + ComDifficultyLevel + 0x1E` — wszystkie 38 osiągalne.
- `case 3` (VS Battle z COM, `0x44..0x63` = wpisy `797..828`, 32 pliki): `CourseIndex * 4 + ComDifficultyLevel + 0x44` (8 tras × 4 poziomy trudności).
- **`case 6` (Extra Trial, `0x64..0x73` = wpisy `829..844`, 16 plików = 8 tras × 2 sloty)**:
  ```c
  case 6:
      CourseAssetBaseFileIndex = GetR4FileIndex(2, (a0 * 2) + 0x64);
      break;
  ```
  Ponieważ `a0` (`CourseIndex` `0..7`) jest mnożone przez `2` bez dodania pod-indeksu `0/1`, gra ładuje wyłącznie **8 parzystych wpisów (`829, 831, 833, 835, 837, 839, 841, 843`)**.
  Wszystkie **8 nieparzystych wpisów (`830, 832, 834, 836, 838, 840, 842, 844`, `Cat 2, 0x65..0x73` nieparzyste)** zawiera odrębne, kompletne 527-bajtowe tabele `RaceSpecEntry30` dla tras `0..7` (z początkowym dystansem rubber-bandingu `10000` zamiast `1000..2000` oraz niższymi parametrami prędkości maksymalnej `w0 = 1424, 1241, 1424, 1460, 1387, 1350, 1350, 1971` wobec parzystych `1445, 1409, 1825, 1774, 1694, 1737, 1606, 2292`), co stanowi pozostałość po **wyciętym drugim wariancie/poziomie trudności przeciwnika w Extra Trial**.

---

### 1.3 Całe kategorie 3 i 4 w `GetR4FileIndex` (`R4.BIN` wpisy `900..927`, 196 608 bajtów) są w 100% nieużywane

W `src/main/PAL/game/data/cdfile1.c` funkcja `GetR4FileIndex` definiuje pięć kategorii (`0..4`), w tym:
- `case 3: return a1 + 0x384;` (`900..918`, 19 wpisów)
- `case 4: return a1 + 0x397;` (`919..927`, 9 wpisów)

W całym `main.exe` oraz we wszystkich 9 overlayach **nie istnieje ani jedno wywołanie `GetR4FileIndex(3, ...)` ani `GetR4FileIndex(4, ...)`**.
Porównanie binarne z pod-blokami nieparzystych kontenerów tras (`Entries 1, 3, 5, 7, 9, 11, 13, 15, 17`, ładowanych przez `SceneLoadStep` w `src/main/PAL/game/data/cdfile2.c`) wykazuje **100% zgodność bajt po bajcie**:

| Nieużywane wpisy `R4.BIN` | `GetR4FileIndex` | Rozmiar łączny | Dokładny odpowiednik wewnątrz kontenerów tras (`Entries 1..17` nieparzyste) | Odbiorca w `SceneLoadStep` (`cdfile2.c`) |
| :--- | :--- | :---: | :--- | :--- |
| **`900..908`** | `Cat 3, 0..8` | `22 528` B (9 plików) | **Identyczne z `sub[8]`** tras `0..8` (`1680, 1792, 1808, 1872, 2144, 2064, 976, 1280, 1648` B) | `SetSegmentVisibilityTable(s5)` |
| **`909..917`** | `Cat 3, 9..17` | `18 432` B (9 plików) | **Identyczne z `sub[9]`** tras `0..8` (`1108, 1468, 2340, 2532, 2920, 1580, 1268, 640, 1936` B) | `SetOctantLookupTable(s4)` |
| **`918`** | `Cat 3, 18` | `100 352` B (1 plik) | **Identyczny z `sub[10]`** trasy `7` (*Shooting Hoops*, wpis `15`, jedyna trasa z 11. pod-blokiem animowanych obiektów) | `InitAnimatedTrackObjects(arena)` |
| **`919..927`** | `Cat 4, 0..8` | `47 104` B (9 plików) | **Identyczne z `sub[7]`** tras `0..8` (`3652, 6076, 4108, 5428, 1912, 5632, 6436, 4756, 124` B) | `LoadTrackObjects(trackObjects, LoadCursor)` |

Są to pozostałości po wcześniejszym etapie potoku budowania zasobów, zanim tablice widoczności, oktantów i obiektów trasy zostały scalone bezpośrednio do kontenerów poszczególnych tras.

---

### 1.4 Nieużywana tekstura TIM w kategorii 1 (`R4.BIN` wpis `725`)

W kategorii 1 (`658..728`, indeksy `0x00..0x46`) każdy wpis jest używany **z wyjątkiem `0x43` (wpis `725`, 10 240 bajtów)**, położonego pomiędzy czcionką dialogową (`724`, `0x42`) a wielojęzycznymi tekstami opisów aut w garażu (`726`, `0x44`):
- Zawiera surową 4-bitową teksturę TIM (`magic = 0x10`, `flags = 0x08`) o wymiarach `128×128` pikseli z 16-kolorową liniową paletą skali szarości (`0x8000, 0x8421, ..., 0xFFFF` pod VRAM `(832, 255)`) oraz obrazem docelowym pod VRAM `(832, 0)` (`0x340, 0`), przedstawiającym dużą okrągłą sylwetkę emblematu z ozdobnymi obramowaniami u góry i u dołu.

---

### 1.5 Mapa wszystkich 950 wpisów `R4.BIN`, 34 duplikaty oraz plik `FILE.DAT`

#### Mapa alokacji `R4.BIN` (`0..949`)
| Zakres `R4.BIN` | Liczba | Funkcja indeksująca | Formuła | Przeznaczenie i status |
| :--- | :---: | :--- | :--- | :--- |
| **`0..17`** | 18 | `GetCourseFileIndex(c)` | `c * 2`, `c * 2 + 1` | **9 tras (`0..8`)**: parzyste = kontenery tekstur TIM (`LoadTimList`), nieparzyste = 10/11-częściowe kontenery sceny (`SceneLoadStep`). Trasa `8` (`16..17`) to scena Intro/Epilogu (`trkld8.c`). Wszystkie 18 używane. |
| **`18..337`** | 320 | `GetCarFileIndex(m, t, s, 0)` | `((t*4 + m)*20 + s) + 0x12` | **Bazowe modele i tekstury 320 aut (`variant == 0`)**: 4 zespoły × 4 producentów × 20 stadiów (`0..19`). Ładowane przez `CarLoadStep`. |
| **`338..657`** | 320 | `GetCarFileIndex(m, t, s, 1)` | `((t*4 + m)*20 + s) + 0x152` | **Warianty aut ze zmienionymi kołami (`variant != 0`)**: 8 wpisów (`357, 377, 397, 417, 437, 457, 477, 497`) to dokładne kopie aut zespołu PRC (`498, 501, 507, 514, 558, 561, 567, 574`). |
| **`658..728`** | 71 | `GetR4FileIndex(1, a1)` | `a1 + 0x292` (`0x00..0x46`) | **Overlaye, tekstury UI, aplet PocketStation, teksty 5-językowe, węzły tras**: Wpis `664` = wycięty **aplet PocketStation**; `669..672` = duplikaty overlayów; **wpis `725` (`0x43`) w 100% nieużywany**. |
| **`729..899`** | 171 | `GetR4FileIndex(2, a1)` | `a1 + 0x2D9` (`0x00..0xAA`) | **Banki VAB/SEQ, dźwięki silników, zasoby tras, tabele AI, modele aut menu (`879`), tekstury intro**: **8 wpisów (`830..844` nieparzyste) w 100% nieużywanych**; 15 duplikatów w `797..820`. |
| **`900..918`** | 19 | `GetR4FileIndex(3, a1)` | `a1 + 0x384` (`0x00..0x12`) | **W 100% nieużywane (0 wywołań)**: Kopie `SegmentVisibilityTable` (`900..908`), `OctantLookupTable` (`909..917`) i `AnimatedTrackObjects` (`918`). |
| **`919..927`** | 9 | `GetR4FileIndex(4, a1)` | `a1 + 0x397` (`0x00..0x08`) | **W 100% nieużywane (0 wywołań)**: Kopie `TrackObjectFile` (`919..927`) dla tras `0..8`. |
| **`928..949`** | 22 | `GetR4FileIndex(0, a1)` | `a1 + 0x3A0` (`0x00..0x15`) | **Obraz VRAM, tła ładowania tras, sprite'y UI i czcionki**: Wszystkie 22 używane (`936 == 937`). |

> **Pozostałość w nagłówku `R4.BIN` (`0xEE0`)**: Tuż za 951. offsetem sektorowym (`28776` pod `0xEDC`), w obszarze paddingu pierwszego sektora katalogu pod `0xEE0` (`CdFileSectorTable[951]`) znajduje się niezainicjalizowane słowo komendy GPU PS1: **`0x520FE000`** (`00 e0 0f 52`).

#### 34 dokładne duplikaty wewnątrz `R4.BIN` (720 896 bajtów)
1. **4 zduplikowane overlaye wykonywalne (`659==670`, `660==669`, `661==671`, `667==672`, 57 344 B)**: `LoadOpeningMovieTimStep` ładuje overlaye z menu głównego według `MainMenuChoice + 1` (`659..667`), natomiast `LoadModeMenuAssetsStep` po powrocie z wyścigu ładuje je według `GameModeType + 0xB` (`669..672`).
2. **8 par zduplikowanych kontenerów aut specjalnych (`357==498`, `377==501`, `397==507`, `417==514`, `437==558`, `457==561`, `477==567`, `497==574`, 385 024 B)**: Sloty `stage == 19` (`0x13`, bonusowe auta specjalne / Pac-Man, które nie obsługują zmiany kół) w tabeli `variant != 0` dla zespołów 0 i 1 (`DRT` i `MMM`) wypełniono kopiami aut zespołu 2 (`PRC`).
3. **15 zduplikowanych tabel AI w trybie VS Battle (`797..820`, 30 720 B)**:
   - `[797, 801, 805, 809, 813, 817]` (trasy `0..5`, trudność `0`) są identyczne,
   - `[798, 802, 806, 810, 814, 818]` (trasy `0..5`, trudność `1`) są identyczne,
   - `[803, 807, 811, 815, 819]` (trasy `1..5`, trudność `2`) są identyczne,
   - `[808, 812]` oraz `[816, 820]` (trudność `3`) są identyczne.
4. **1 zduplikowany kontener TIM tła trasy (`936 == 937`, 14 336 B)**: Trasa 8 (scena intro/epilogu) używa kopii kontenera TIM trasy 7 (*Shooting Hoops*).

#### Plik `disc/PAL/FILE.DAT` (`27 648 000` bajtów)
- Leży na samym końcu ścieżki danych ISO9660 (`LBA 254070..267569`, 13 500 sektorów, tuż za `R4.STR;1` i przed 150-sektorowym postgapem).
- Składa się w **100% z bajtów zerowych (`0x00`)** i nie ma do niego żadnych odwołań w kodzie ani tablicach — pełni wyłącznie rolę pliku dopełniającego (dummy padding) na płycie CD-ROM.

---

## 2. Wycięte mechaniki i martwe podsystemy w kodzie (`main.exe` i overlaye)

### 2.1 Wycięty dekoder i walidator haseł rekordów Time Attack (`trkintp2.c`, `uicnt.c`)

W wydanej grze funkcja `EncodeRecordPassword` (`0x800433AC` w `src/main/PAL/game/course/trkintp2.c`) koduje 14-znakowe hasło ASCII (`PasswordChars`) na ekranie Records, służące do zgłaszania rekordów w konkursach Namco.
Tuż obok w `src/main/PAL/game/course/trkintp2.c` oraz `src/main/PAL/game/ui/uicnt.c` znajduje się **kompletny, nigdzie niewywoływany dekoder i weryfikator haseł**:
- **`DecodeRecordPassword`** (`0x80043508`, rozmiar `0x100`, `src/main/PAL/game/course/trkintp2.c`) — kopiuje 14-znakowy łańcuch do `PasswordChars`, wywołuje `ValidateRecordPassword`, a następnie rozpakowuje 5 parametrów:
  - `*a`: 4-bitowy indeks trasy (`PasswordBytes[0] & 0xF`),
  - `*b`: 10-bitowe ID samochodu (`((PasswordBytes[1] << 4) + (PasswordBytes[0] >> 4)) & 0x3FF`),
  - `*c`: 20-bitowy czas przejazdu (`(t >> 2) & 0xFFFFF`),
  - `*d`: 2-bitową flagę skrzyni biegów/trybu (`t & 3`),
  - `*e`: 4-bitową wartość dodatkową (`PasswordBytes[5] & 0xF`).
- **`ValidateRecordPassword`** (`0x8004326C`, rozmiar `0x1C0`, `src/main/PAL/game/course/trkintp2.c`) — dekoduje 7 par znaków przez `CodeCharsToByte`, odwraca szyfr Vigenère'a (`PasswordCipherKey`, modulo 11) przez `AddOrSubtractByte(*t, *r, 1)`, weryfikuje sumę kontrolną `ChecksumBytes(a0, 6)`, wykonuje `XorBytes37(a0, 6)` oraz sprawdza zakresy czasu (`<= 0x927BF`, czyli `599 999` ms = `9'59"999`), ID samochodu (`< 0x280` = 640) i pola dodatkowego (`< 6`).
- **`CodeCharsToByte`** (`0x800430BC`, rozmiar `0x1C`, `src/main/PAL/game/ui/uicnt.c`) — odwrotność `ByteToCodeChars` (`0x80043098`), składająca dwa znaki ASCII (`arg0 - 'A'`, `arg1 - 'K'`) z powrotem w 8-bitowy bajt.

### 2.2 Nieużywana kalkulacja mgły na `double` zaciągająca 4,6 KB biblioteki soft-float (`model.c`)

- **`SetFogNearFar`** (`0x8006F2E0`, rozmiar `0x310`, `src/main/PAL/game/gfx/model.c`) wylicza rejestry głębi mgły GTE `DQA` (`ctc2 $27`) i `DQB` (`ctc2 $28`) na podstawie odległości `a0`, `a1` i odległości rzutowania `a2` przy użyciu **64-bitowej zmiennoprzecinkowej arytmetyki programowej IEEE-754 `double`** (`__muldf3`, `__divdf3`, `__adddf3`, `__subdf3`, `__fixdfsi`, `__floatsidf`, `__gtdf2`, `__ltdf2`).
- W wydanej grze zastąpiono ją funkcją stałoprzecinkową `ApplyScaledFog` (`0x8006F5F0` w `model.c`).
- Ponieważ nie usunięto `SetFogNearFar` z `model.c`, linker dołączył do `main.exe` **całą 16-funkcyjną bibliotekę soft-float `double` (`0x8007D7D4..0x8007EA28`, 4 692 bajty: `__adddf3`, `_add_mant_d`, `_dbl_shift`, `__divdf3`, `_comp_mant`, `_dbl_shift_us`, `__fixdfsi`, `__floatsidf`, `__gtdf2`, `__ltdf2`, `_mainasu`, `__muldf3`, `_mul_mant_d`, `__negdf2`, `__subdf3`, `_err_math`)**, która nie ma żadnego innego wywołania w całej grze.

### 2.3 Stary, wieloczęściowy renderer samochodów z 3 poziomami LOD (`model.c`)

- **`DrawArticulatedCarModel`** (`0x8006F6F0`, rozmiar `0x9BC` = 2 492 bajty, `src/main/PAL/game/gfx/model.c`) to największa pojedyncza martwa funkcja w kodzie gry Namco.
- Implementuje starszy renderer samochodu złożonego z osobnych części (karoseria + 4 niezależne koła ze skrętem `wheelSteering17C` i ugięciem zawieszenia + cień), wybierający spośród 3 progów odległości LOD (`arg3`, `arg4`, `arg5`), indeksujący `CarBoundsTable[0]`, wywołujący `SelectModelBank` i wysyłający geometrię przez starszy potok `SubmitModel` (`0x8005F098`).
- W finalnej wersji R4 całe renderowanie aut zostało przeniesione do `DrawCarShowcaseModel`, `DrawPlayerCar` i `DrawRemoteCar` w `src/main/PAL/game/car/carwheel.c`, które renderują modele `.RRO` przez `DrawRroModel` (`0x80014A98`).

### 2.4 Wyłączony strumień komend `.RRO` typu 1 (`EmitRroRawTexturedQuads`) w `gtepoly1.c`

- **`EmitRroRawTexturedQuads`** (`0x80013D48`, rozmiar `0x320`, `src/main/PAL/game/gfx/gtepoly1.c`) transformuje i emituje nieoświetlone teksturowane czworokąty (`POLY_FT4`, komenda `0x2F000000`) ze strumienia modelu `.RRO`.
- W `DrawRroModel` (`0x80014A98` w `src/main/PAL/game/gfx/gtepoly1.c`) obsługa komendy `1` została jawnie wyłączona przez przesunięcie wskaźnika o `count * 6` słów:
  ```c
  switch (command) {
  case 1:
      commands += count * 6; // Pomija blok komend typu 1 zamiast wywołać EmitRroRawTexturedQuads!
      break;
  case 2:
      commands = EmitRroLitTexturedTris(commands, &work, count);
      break;
  ```

### 2.5 Nieużywany jednoprzebiegowy widok trasy oraz 2 KB martwego kodu GTE w `SubmitModel.s`

- **`RenderPlayerCourseView`** (`0x8006ED4C`, rozmiar `0x58`, `src/main/PAL/game/camera/viewcam.c`) oraz wywoływane przez nią **`UpdateVisibleViewRecords`** (`0x8006E8F8`, rozmiar `0xD4`, `src/main/PAL/game/camera/viewini.c`) tworzą prostszy, jednoprzebiegowy potok renderowania trasy (`BuildViewSegmentObjectList` -> `UpdateVisibleViewRecords` -> `ApplySegmentVisibilityMask` -> `SetupViewLighting` -> `SetupViewMatrix` -> `SubmitCourseHorizon`), zastąpiony przez `RenderCourseViews` (`0x8006ECF0`) i `RenderMirroredCourseView` (`0x8006EDA4`).
- **`SubmitModel2`** (`0x8005F14C`, rozmiar `0xB4`, `asm/PAL/hasm/SubmitModel.s`) to nieużywany drugi punkt wejścia do `SubmitModel` (`0x8005F098`), korzystający z drugiej tablicy skoków `jtbl_8005F220` z zamienionymi wpisami 1 i 2 (`func_80064BB0` vs `func_80061B0C`).
- **`SubmitCourseModel`** (`0x8005FC74`, rozmiar `0x58`, `asm/PAL/hasm/SubmitModel.s`) to nieskalowany odpowiednik `SubmitCourseModel2` (`0x8005FCCC`), wywołujący `func_8005FE54` z `$t9 = 0`. Ponieważ w grze wywoływane jest wyłącznie `SubmitCourseModel2` (które wskakuje bezpośrednio w środek `func_8005FE54` pod adres `0x80060640` z `$t9 = 1`), **pierwsze `0x7EC` bajtów (`2 028` bajtów, `0x8005FE54..0x8006063F`) funkcji `func_8005FE54`** jest całkowicie nieosiągalnym martwym kodem GTE.
- **`DrawWireQuadVerts1`** (`0x8006D5A4`, rozmiar `0x4A0`, `asm/PAL/hasm/SubmitModel.s`) oraz **`QuadOffscreenTestVerts1`** (`0x8006DA44`, rozmiar `0xC8`) to jednoekranowe odpowiedniki używanych w trybie split-screen funkcji `DrawWireQuadVerts2` (`0x8006DB0C`) i `QuadOffscreenTestVerts2` (`0x8006DFCC`).
- **Nadmiarowy 17. wierzchołek i 2 nieużywane półsłowa w `Circle16Offsets` (`polytri.c`)**: W `DrawTireTracks` (`src/main/PAL/game/gfx/polytri.c`) tablica `Circle16Offsets` zawiera 17 par XY (34 półsłowa) zakończonych dwoma nieodczytywanymi półsłowami `(0, 0)`, a 17. zrzutowany wierzchołek jest zapisywany na scratchpadzie, ale nigdy nie jest emitowany jako trójkąt.

### 2.6 Zepsute wskaźniki do overlaya w `ModeGroup1FuncTable[19..20]` (`0x8009E8E8..0x8009E8EC`)

W tablicy wskaźników funkcji trybów `ModeGroup1FuncTable` (`0x8009E89C` w `main.exe`) wpisy `19` i `20` (odpowiadające `ModeGroup = 1, ModeIndex = 9`) zawierają nieaktualne adresy:
- `ModeGroup1FuncTable[19]` (`0x8009E8E8`) = **`0x80114888`**
- `ModeGroup1FuncTable[20]` (`0x8009E8EC`) = **`0x801148C4`** (różnica `0x3C`)

W overlayu `ovl659` (`grand_prix`), ładowanym pod `0x80114878`, na początku znajduje się `0x54` bajtów `.rodata`, a sekcja `.text` zaczyna się dokładnie `0x44` bajty dalej:
- `ovl659_exit_race` leży pod `0x801148CC` (`0x80114888 + 0x44`, rozmiar `0x3C`),
- `ovl659_update_race` leży pod `0x80114908` (`0x801148C4 + 0x44`).

W finalnej wersji wyścig Grand Prix przeniesiono pod `ModeIndex = 22` (`ModeGroup1FuncTable[46..47]` = `0x801148CC`, `0x80114908`), natomiast w dawnym slocie `ModeIndex = 9` pozostały stare wskaźniki sprzed dodania `0x44` bajtów `.rodata` do `ovl659`.

### 2.7 Martwy opcode 9 skryptu menu (`LoadMenuSoundStep`) oraz nieużywany drugi bank strumieni XA (`R4StrPath[1]`)

- **Opcode 9 w `MenuScriptOps` (`0x8009F840`, `src/main/PAL/game/menu/menu.c`)**: Dyspozytor `UpdateMenuScript` (`0x8003A544`) posiada 12-elementową tablicę `MenuScriptOps`, ale wszystkie 7 skryptów `MenuScripts` (`0x8009F8D0..0x8009F8EC`) używa wyłącznie opcode'ów `{0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11}`. Wskazywana przez `MenuScriptOps[9]` funkcja **`LoadMenuSoundStep`** (`0x8003AAE8`) oraz wywoływana przez nią zaślepka **`LoadMenuSoundVabStep`** (`0x8004E184`, `return 1;`) i sąsiednia **`LoadMenuSoundSeqStep`** (`0x8004E18C`, `return 1;`) są całkowicie martwe.
- **Drugi bank strumieni XA `R4StrPath[1]` (`0x800A07A4`, `src/main/PAL/game/sound/cdpos.c`)**: W `.data` pod `0x800A0784` zdefiniowana jest dwuelementowa tablica ścieżek `char R4StrPath[2][32]`, z obiema komórkami zawierającymi `"\\R4.STR;1"` (`0x800A0784` i `0x800A07A4`). Funkcja `CdAudioFindStreamFile` inicjalizuje oba banki `SoundState.streamBanks[0]` i `SoundState.streamBanks[1]` wartością `-1`, ale wyszukuje na płycie tylko `R4StrPath[0]`, pozostawiając `R4StrPath[1]` (`0x800A07A4`) oraz `streamBanks[1]` całkowicie nieużywane.

---

## 3. Pełna lista 59 martwych funkcji gry Namco w `main.exe`

### A. Debugowe UI, wskaźniki ładowania i wczesne ekrany wyników
| Adres VRAM | Rozmiar | Nazwa funkcji | Plik źródłowy | Opis działania i kontekst |
| :--- | :---: | :--- | :--- | :--- |
| `0x80012408` | `0x170` | `DrawLoadingTriangle` | `src/main/PAL/game/data/cdfile1.c` | Debugowy wskaźnik ładowania tuż za `InitCdFileDirectory`. Obraca `LoadingTriangleAngle` o `0x10` co klatkę i dodaje cieniowany metodą Gouraud trójkąt RGB (`LoadingIndicatorTriBuf[2]`, wierzchołki R/G/B w `(80, 128)`) do tablicy OT. |
| `0x80022448` | `0x194` | `DrawRaceTimesScreen` | `src/main/PAL/game/ui/hudmap.c` | Wczesny/debugowy ekran czasów po wyścigu na początku `hudmap.c`. Rysuje wyśrodkowane napisy `"TOTAL TIME"`, `"LAP TIME"`, `"PRESS START"` oraz wszystkie `LapTimes[0..NumLaps-1]`. |
| `0x8002AFA4` | `0x188` | `DrawArrowTrianglePair` | `src/main/PAL/game/ui/uipart1.c` | Rysuje parę trójkątnych strzałek UI lewo/prawo (`EmitPolyF3`) przy użyciu `BeamFadeStripHeights` i `UiBeamFadePalettePair`. |
| `0x8003153C` | `0x60` | `AddScreenBackdropTile` | `src/main/PAL/game/menu/menucalc.c` | Dodaje pełnoekranowy (`320×512`) ciemnoszary (`RGB 0x20, 0x20, 0x20`) prymityw `TILE` na głębokości OT `703`. |
| `0x800397A4` | `0x18` | `GetNameChar` | `src/main/PAL/game/menu/menuwork.c` | Prosty akcesor `*a0 = NameCharset[a1];` do tablicy znaków imienia gracza. |

### B. Wycięte / zastąpione przepływy menu, powtórek i kart pamięci
| Adres VRAM | Rozmiar | Nazwa funkcji | Plik źródłowy | Opis działania i kontekst |
| :--- | :---: | :--- | :--- | :--- |
| `0x8001F2CC` | `0x8` | `MenuSceneNoop` | `src/main/PAL/game/race/ranking.c` | Pusta funkcja (`jr $ra; nop`) wpisana wyłącznie w `MenuSceneEntries[1].init` (scena 1 nie jest używana przez żaden skrypt w `MenuScripts`). |
| `0x8003244C` | `0xAC` | `StepReplaySavePrompt` | `src/main/PAL/game/menu/menu8.c` | Obsługuje 2-opcjowy przełącznik (`MenuAutoAdvanceTimer` 0/1 przez D-pad Lewo/Prawo) i po wciśnięciu Start (`0x800`) przechodzi do menu karty pamięci (`ModeGroup = 1, ModeIndex = 0xF`). |
| `0x80078000` | `0xB8` | `CardSystemShutdown` | `src/main/PAL/game/data/cardini.c` | Zamyka wszystkie 8 zdarzeń BIOS karty pamięci (`HwCardEv*`, `SwCardEv*`) i wywołuje `_cardStopCom()` oraz `StopCARD()`. Jej brak wywołań osieraca funkcje PsyQ `StopCARD`, `StopCARD2`, `_ExitCard` i `_cardStopCom`. |
| `0x800786F8` | `0xCC` | `CardWriteFileAt` | `src/main/PAL/game/data/cardini.c` | Otwiera istniejący plik na karcie pamięci (`bu%1d%1d:%s`), przewija na `offset` (`FileSeek`) i zapisuje `bytes` (`FileWrite`). Gra używa zamiast tego `CardWriteData` (`0x80078610`), zapisującego zawsze od offsetu 0. |
| `0x80079ED8` | `0x120` | `BuildCardHeaderAnim` | `src/main/PAL/game/data/savedat.c` | Buduje nagłówek zapisu karty pamięci PS1 z `iconMode02 = 0x13` (**3-klatkowa animowana ikona**), odczytując trzy kolejne klatki 16×16 4bpp z VRAM (`frameX += 4`). Wydana gra wywołuje wyłącznie `BuildCardHeader` (`0x80079DEC`) ze statyczną 1-klatkową ikoną (`0x11`). |
| `0x8007AB8C` | `0x40` | `ExitCardMenuToMode` | `src/main/PAL/game/menu/loadmenu.c` | Ustawia `ModeGroup = 1`, `ModeIndex = (arg0 == -1) ? 3 : 0xF` oraz `SubmodeState = 0`. |
| `0x8007D558` | `0x8` | `CardSlotUiNoop` | `src/main/PAL/game/menu/menutra22.c` | Pusta zaślepka (`jr $ra; nop`) na końcu modułu UI slotów karty pamięci (`menutra22.c`). |

### C. Fizyka pojazdów, ścieżki AI i logika wyścigu
| Adres VRAM | Rozmiar | Nazwa funkcji | Plik źródłowy | Opis działania i kontekst |
| :--- | :---: | :--- | :--- | :--- |
| `0x80022C90` | `0x44` | `StepCarPhysics` | `src/main/PAL/game/car/carsetup.c` | Wrapper wywołujący kolejno `UpdateCarSteering`, `IntegrateCarMotion`, `UpdateCarTrackNode`, `ComputeWheelOffsets` i `ConstrainCarToTrack`. W grze `UpdateRaceCars` (`0x80034020` w `racemode.c`) wywołuje te 5 kroków osobno, przeplatając je logiką powtórek i kolizji. |
| `0x80026C80` | `0x2DC` | `FindTrackNodeScratch` | `src/main/PAL/game/car/aipath2.c` | Przeszukuje węzły trasy na zewnątrz (`x + n`, `x - n`) od bieżącego węzła koła przy użyciu GTE `NCLIP` (`RR_GTE_NCLIP3`), aby znaleźć czworokąt segmentu zawierający koło. Zastąpione przez `UpdateCarTrackNode`. |
| `0x800337A8` | `0x60` | `CopyGhostCarToSlot1` | `src/main/PAL/game/race/racemode.c` | Kopiuje strukturę `RaceVehicle` do `PlayerCars[1]`, zeruje górny bajt `flags1E4` i ustawia `RacePhysicsModeIndex = a0->physicsFlags1E0 + 2`. |
| `0x800353E4` | `0x94` | `PollLinkStatus` | `src/main/PAL/game/race/racegrid.c` | Odpytuje bity `7..8` rejestru `_sio_control(0, 0, 0)` i aktualizuje `LinkState` (`-1`, `1` lub zwraca `1`, gdy `t == 2 && LinkState == 3`). Zastąpione przez maszynę stanów w `racegrid.c` i `ovl667`. |
| `0x800365BC` | `0x28` | `SortRankAndShowResults` | `src/main/PAL/game/race/raceai2.c` | Wrapper wywołujący `SortRaceRankTable()` oraz `EnterRaceResultScreen()`. |
| `0x80038408` | `0xAC` | `PointInQuad` | `src/main/PAL/game/race/racemai2.c` | Sprawdza, czy spakowany punkt 2D `point` leży wewnątrz czworokąta `(corner0, corner1, corner3, corner2)` przez 4 wywołania `NormalClip` (`>= 0`). |
| `0x80048384` | `0x54` | `FindTrackNodeBehind` | `src/main/PAL/game/car/carcurv.c` | Cofając się po `TrackNodes` od węzła `a0`, odejmuje `p[a0].road.segmentLength` aż dystans `a1 <= 0` (odwrotność używanego `FindTrackNodeAhead` `0x80048330`). |

### D. System dekodowania haseł rekordów
| Adres VRAM | Rozmiar | Nazwa funkcji | Plik źródłowy | Opis działania i kontekst |
| :--- | :---: | :--- | :--- | :--- |
| `0x800430BC` | `0x1C` | `CodeCharsToByte` | `src/main/PAL/game/ui/uicnt.c` | Martwa przechodnio (wywoływana tylko przez `ValidateRecordPassword`). Dekoduje 2 znaki hasła na bajt. |
| `0x8004326C` | `0x1C0` | `ValidateRecordPassword` | `src/main/PAL/game/course/trkintp2.c` | Martwa przechodnio (wywoływana tylko przez `DecodeRecordPassword`). Deszyfruje i sprawdza sumę kontrolną 14-znakowego hasła. |
| `0x80043508` | `0x100` | `DecodeRecordPassword` | `src/main/PAL/game/course/trkintp2.c` | Bezpośrednio nieużywana. Rozpakowuje trasę, ID auta, czas okrążenia i flagi ze zwalidowanego hasła. |

### E. Grafika 3D, renderowanie trasy, oświetlenie i kamera
| Adres VRAM | Rozmiar | Nazwa funkcji | Plik źródłowy | Opis działania i kontekst |
| :--- | :---: | :--- | :--- | :--- |
| `0x80013D48` | `0x320` | `EmitRroRawTexturedQuads` | `src/main/PAL/game/gfx/gtepoly1.c` | Emitter nieoświetlonych czworokątów `POLY_FT4` dla modeli `.RRO` (pominięty w `DrawRroModel`). |
| `0x8003D01C` | `0xEC` | `SetLightColorsFromRow` | `src/main/PAL/game/gfx/gfxmtx.c` | Ładuje `LightMatrix`, `SetBackColor` i `SetColorMatrix` bezpośrednio z wiersza `index` tablicy `TrackLightTable` (zastąpione przez interpolowane oświetlenie `InterpolateTrackLighting`). |
| `0x80044064` | `0x3C` | `CountTrackEntries` | `src/main/PAL/game/gfx/dispenv.c` | Zlicza wpisy w `RaceContext.cameraNodes550` aż do wartownika `node32 == -1`. |
| `0x800440A0` | `0xDC` | `ApplyTrackLightPreset` | `src/main/PAL/game/gfx/dispenv.c` | Konfiguruje `SetSceneOverlayEnabled` i `SetSceneOverlayColor` na podstawie `RaceContext.lightPresets558[(selector - 1) % 4]` oraz ustawia `CarSlotBuffers[0..7].overlay70`. |
| `0x800448EC` | `0x34` | `InitTrackStartPos` | `src/main/PAL/game/gfx/gfxdraw.c` | Kopiuje współrzędne X/Z `TrackNodes[0]` do `SceneFogWorkVector[0..2]` i zeruje `SceneFogWorkVector[4..6]`. |
| `0x8004C998` | `0x244` | `EmitTexturedQuadPrim` | `src/main/PAL/game/course/trktext.c` | Emituje prymityw `POLY_FT4` z deskryptora `TexturedQuadDesc2C` do `CurrentDrawBuf->buffer.ordering70[a0->otIndex00]`. |
| `0x8004CBDC` | `0x40` | `InitTexturedQuadDesc` | `src/main/PAL/game/course/trktext.c` | Zeruje strukturę `TexturedQuadDesc2C` i ustawia `shadeTex04 = 1`, `r10 = g11 = b12 = 0x80`. |
| `0x8004CC1C` | `0x16C` | `AddHGradientRectPrim` | `src/main/PAL/game/course/trktext.c` | Emituje prostokąt `POLY_G4` z poziomym gradientem koloru między lewym (`arg1`) a prawym (`arg2`) bokiem (w grze używany jest tylko pionowy odpowiednik `AddVGradientRectPrim` `0x8004CD88`). |
| `0x8005B2E4` | `0x10` | `SetPresentationFullViewport` | `src/main/PAL/game/camera/campath.c` | Setter `RaceContext.controller3D8->fullViewport50 = arg0;`. |
| `0x8005B2F4` | `0x10` | `SetPresentationStencilEnabled` | `src/main/PAL/game/camera/campath.c` | Setter `RaceContext.controller3D8->stencilEnabled58 = arg0;`. |
| `0x8005F14C` | `0xB4` | `SubmitModel2` | `asm/PAL/hasm/SubmitModel.s` | Nieużywane drugie wejście do `SubmitModel` korzystające z `jtbl_8005F220`. |
| `0x8005FC74` | `0x58` | `SubmitCourseModel` | `asm/PAL/hasm/SubmitModel.s` | Nieskalowane wejście do wysyłania modelu trasy (wraz z `0x7EC` B martwego kodu GTE w `func_8005FE54` pod `0x8005FE54..0x8006063F`). |
| `0x8006D5A4` | `0x4A0` | `DrawWireQuadVerts1` | `asm/PAL/hasm/SubmitModel.s` | Jednoekranowy renderer czworokątów z linii (`LINE_F3`/`LINE_F4` przez GTE `RTPT`). |
| `0x8006DA44` | `0xC8` | `QuadOffscreenTestVerts1` | `asm/PAL/hasm/SubmitModel.s` | Martwa przechodnio (wywoływana tylko przez `DrawWireQuadVerts1`). Test przycinania 4 przetransformowanych wierzchołków. |
| `0x8006E8F8` | `0xD4` | `UpdateVisibleViewRecords` | `src/main/PAL/game/camera/viewini.c` | Martwa przechodnio (wywoływana tylko przez `RenderPlayerCourseView`). Dopisuje rekordy z `ExtraCourseViewTable` do `ActiveVisibleViews`. |
| `0x8006E9CC` | `0x8` | `CourseViewInitNoop` | `src/main/PAL/game/camera/viewini.c` | Pusta zaślepka (`jr $ra; nop`) na końcu `viewini.c`. |
| `0x8006ED4C` | `0x58` | `RenderPlayerCourseView` | `src/main/PAL/game/camera/viewcam.c` | Nieużywany jednoprzebiegowy potok renderowania widoku trasy gracza. |
| `0x8006F2E0` | `0x310` | `SetFogNearFar` | `src/main/PAL/game/gfx/model.c` | Nieużywana kalkulacja mgły na `double` (jedyny powód dołączenia 4,6 KB biblioteki soft-float). |
| `0x8006F6F0` | `0x9BC` | `DrawArticulatedCarModel` | `src/main/PAL/game/gfx/model.c` | Stary 3-poziomowy (LOD) wieloczęściowy renderer samochodów, zastąpiony przez modele `.RRO`. |
| `0x80070170` | `0x54` | `UnrelocateModelBank` | `src/main/PAL/game/gfx/model.c` | Odejmuje adres bazowy `base` od wskaźników w `RroModelBank`, zamieniając je z powrotem na offsety względne (odwrotność `RelocateModelBank` `0x80070118`). |
| `0x8007021C` | `0x18` | `SetVramImageSlot` | `src/main/PAL/game/gfx/model.c` | Zapisuje wskaźnik obrazu w `ModelBoneMatrixTable[a1] = a0;`. |
| `0x80070234` | `0x38` | `LoadVramImageSlot` | `src/main/PAL/game/gfx/model.c` | Wywołuje `LoadImage(OpeningLensFlareSprites, ModelBoneMatrixTable[a0]);`. |
| `0x80070400` | `0x40` | `RelocateCountedOffsetTable` | `src/main/PAL/game/gfx/poly1.c` | Relokuje tablicę `(*a0) * 8` 32-bitowych offsetów w miejscu i ustawia `ActivePolyStripPtr = a0 + 1`. |
| `0x80071338` | `0x10` | `GetSegmentVisibilityTableSize` | `src/main/PAL/game/gfx/poly1.c` | Zwraca `RelFileSegmentCount * 0x10`. |
| `0x800713D0` | `0x10` | `GetOctantLookupTableSize` | `src/main/PAL/game/gfx/poly1.c` | Zwraca `RelFileSegmentCount * 8`. |

### F. Dźwięk, CD-Audio i kontroler DualShock
| Adres VRAM | Rozmiar | Nazwa funkcji | Plik źródłowy | Opis działania i kontekst |
| :--- | :---: | :--- | :--- | :--- |
| `0x8003AAE8` | `0x30` | `LoadMenuSoundStep` | `src/main/PAL/game/menu/menu.c` | Martwy handler skryptu menu (`MenuScriptOps[9]`), niewywoływany przez żaden skrypt w `MenuScripts[0..6]`. |
| `0x8004B738` | `0xA8` | `PadRangeCheckSet` | `src/main/PAL/game/system/pad.c` | Setter wibracji DualShocka ze sprawdzaniem zakresów: wypisuje `"Controller no error! - %d\n"` (`a >= 2`) oraz `"Actuator no error! - %d\n"` (`b >= 2`), po czym ustawia `PadPulseState[a].delay10[b] = d` i `limit00[b] = c`. |
| `0x8004C098` | `0x54` | `EnablePadAnalogMode` | `src/main/PAL/game/gfx/color.c` | Wywołuje `PadSetMainMode(i * 16, 1, 0)`, jeśli `PlayerControllers[i].kind1C != 0`, aby programowo przełączyć kontroler w tryb analogowy. |
| `0x8004D7C8` | `0x10` | `GetRaceLoadCursor` | `src/main/PAL/game/data/trkld1.c` | Zwraca `RaceLoadCursor`. |
| `0x8004E184` | `0x8` | `LoadMenuSoundVabStep` | `src/main/PAL/game/data/trkld1.c` | Wywoływana wyłącznie przez martwe `LoadMenuSoundStep`; zaślepiona do `return 1;`. |
| `0x8004E18C` | `0x8` | `LoadMenuSoundSeqStep` | `src/main/PAL/game/data/trkld1.c` | Nieużywana zaślepka (`return 1;`) tuż za `LoadMenuSoundVabStep`. |
| `0x8004EE90` | `0x74` | `SetReverb` | `src/main/PAL/game/sound/sndvab.c` | Konfiguruje typ i głębokość pogłosu SPU (`SsUtReverbOff`, `SsUtSetReverbType`, `SsUtReverbOn`). Gra używa zamiast tego `InitReverb` (`0x8004EE20`) ze stałym presetem. |
| `0x80050844` | `0x50` | `SlotSetProgram` | `src/main/PAL/game/sound/sndsys.c` | Odczytuje/aktualizuje `SoundState.slots38[a3].programData3C.programs[a1][a0]`. |
| `0x80052D50` | `0x1C` | `CdAudioSetFilter` | `src/main/PAL/game/sound/cdstart.c` | Zleca zmianę pliku/kanału CD-XA przez ustawienie `SoundState.filter266C = a0; SoundState.action2670 = 6; SoundState.step2678 = 0;`. |
| `0x80052D6C` | `0x3C` | `CdAudioFilterPending` | `src/main/PAL/game/sound/cdstart.c` | Zwraca wartość niezerową, jeśli operacja zmiany filtra CD-XA jest w toku (`phase2674`, `step2678` lub `flag2680`). |
| `0x80053CD4` | `0x28` | `SeqMusicPause` | `src/main/PAL/game/sound/fxplay.c` | Wywołuje `SsSeqPause(ActiveSfxSeqHandle)`. Jej brak wywołań osieraca funkcje PsyQ `SsSeqPause` i `_SsSndSetPauseMode`. |
| `0x80053CFC` | `0x28` | `SeqMusicResume` | `src/main/PAL/game/sound/fxplay.c` | Wywołuje `SsSeqReplay(ActiveSfxSeqHandle)`. Jej brak wywołań osieraca funkcje PsyQ `SsSeqReplay` i `_SsSndSetReplayMode`. |

---

## 4. Pełna lista 14 martwych funkcji w overlayach (`src/overlays/PAL/`)

| Overlay | Adres VRAM | Rozmiar | Nazwa funkcji | Plik źródłowy | Opis działania |
| :--- | :--- | :---: | :--- | :--- | :--- |
| **ovl661** (`versus`) | `0x80117340` | `0x3C` | `ovl661_select_scene` | `src/overlays/PAL/versus/preview.c` | Ustawia `CurrentSceneId = (MenuTransitionVariant != 0) ? 0xE : 0xD;`. W `ovl660` odpowiednik `ovl660_select_scene` jest wywoływany przez `MenuSceneEntries[10].init`, natomiast `ovl661_select_scene` nie jest nigdzie podpięte. |
| **ovl662** (`garage`) | `0x80115474` | `0x54` | `ovl662_reset_team_car_preview` | `src/overlays/PAL/garage/team_car_select.c` | Resetuje stan podglądu wyboru auta w garażu (`SubmodeState = 0; MenuAnimStep = 0; ...`) po naciśnięciu przycisku `Select` (`Pad0Triggers & 0x100`). |
| **ovl667** (`serial_link`) | `0x80114950` | `0x4C` | `ovl667_reset_sio_ports` | `src/overlays/PAL/serial_link/sio.c` | Wywołuje `_sio_control(2, 1, 0)` oraz `_sio_control(1, 2, 0)` w celu zresetowania portu szeregowego Link Cable. |
| **ovl667** (`serial_link`) | `0x80115048` | `0x10` | `ovl667_get_sio_error_count` | `src/overlays/PAL/serial_link/sio.c` | Zwraca `ovl667_sio_error_count` (licznik błędów transmisji SIO inkrementowany w `ovl667_sio_interrupt_handler`, który przez to jest zmienną wyłącznie zapisywaną!). |
| **ovl667** (`serial_link`) | `0x80115058` | `0x10` | `ovl667_get_sio_status_flag` | `src/overlays/PAL/serial_link/sio.c` | Zwraca `ovl667_sio_status_flag`. |
| **ovl667** (`serial_link`) | `0x80115494` | `0xEC` | `ovl667_sync_link_display` | `src/overlays/PAL/serial_link/sio.c` | Pomocnik synchronizacji klatki zamieniający `CurrentDrawBuf`/`CurrentDispBuf` i wywołujący `DrawSync(0)`, `ovl667_exchange_link_packets()`, `VSync(2)` oraz `PutDispEnv`/`PutDrawEnv`. |
| **ovl667** (`serial_link`) | `0x80117EE8` | `0x38` | `ovl667_select_scene` | `src/overlays/PAL/serial_link/session_menu.c` | Ustawia `CurrentSceneId = (MenuTransitionVariant != 0) ? 0x10 : 0xF;`. Analogicznie do `ovl661_select_scene`, nigdy niepodpięte do `MenuSceneEntries`. |
| **ovl668** (`movie_playback`) | `0x801162CC` | `0x8C` | `ovl668_mdec_get_env` | `src/overlays/PAL/movie_playback/mdec.c` | Odczytuje status sprzętowy MDEC przez `ovl668_mdec_read_status()` i wypełnia strukturę `MdecEnv`. |
| **ovl668** (`movie_playback`) | `0x80116358` | `0x98` | `ovl668_mdec_put_env` | `src/overlays/PAL/movie_playback/mdec.c` | Zapisuje strukturę `MdecEnv` do rejestrów MDEC. |
| **ovl668** (`movie_playback`) | `0x801163F0` | `0x18` | `ovl668_mdec_buf_size` | `src/overlays/PAL/movie_playback/mdec.c` | Wyciąga rozmiar strumienia `(bs[0] >> 16) << 1` z nagłówka MDEC. |
| **ovl668** (`movie_playback`) | `0x80116498` | `0x3C` | `ovl668_mdec_in_sync` | `src/overlays/PAL/movie_playback/mdec.c` | Czeka na zakończenie DMA0 (MDEC-in) z komunikatem `ovl668_mdec_in_timeout_msg` (`"MDEC_in_sync"`). |
| **ovl668** (`movie_playback`) | `0x801164D4` | `0x3C` | `ovl668_mdec_out_sync` | `src/overlays/PAL/movie_playback/mdec.c` | Czeka na zakończenie DMA1 (MDEC-out) z komunikatem `ovl668_mdec_out_timeout_msg` (`"MDEC_out_sync"`). |
| **ovl668** (`movie_playback`) | `0x80116510` | `0x18` | `ovl668_mdec_in_callback` | `src/overlays/PAL/movie_playback/mdec.c` | Ustawia `ovl668_mdec_state.inCallback00 = callback;`. |
| **ovl668** (`movie_playback`) | `0x8011688C` | `0x20` | `ovl668_mdec_read_status` | `src/overlays/PAL/movie_playback/mdec.c` | Martwa przechodnio (wywoływana tylko przez `ovl668_mdec_get_env`). Odczytuje `*ovl668_mdec1_ctrl_ptr`. |

---

## 5. Zaślepki (`Noop` / `Stub`), wywołania `DebugPrintStub` i martwe flagi diagnostyczne

### 5.1 Wykaz wszystkich funkcji-zaślepek (`size == 0x8`)
| Adres VRAM | Nazwa funkcji | Plik źródłowy | Zwracana wartość | Rola / wywołujący |
| :--- | :--- | :--- | :--- | :--- |
| `0x8001E84C` | `DebugPrintStub` | `src/main/PAL/game/gfx/clut.c` | `return 1;` | Produkcyjna zaślepka `printf`. Wywoływana **64 razy** w `main.exe`, **12 razy** w overlayach, zapisana w `GPU_printf` (`0x800ABABC`) i użyta jako callback w `InitGeom` (`0x8008AF80`). |
| `0x8001ECE0` | `InitFrameEnvStub` | `src/main/PAL/game/system/frmenv.c` | `void` | Wywoływana przy inicjalizacji środowiska klatki. |
| `0x8001F2CC` | `MenuSceneNoop` | `src/main/PAL/game/race/ranking.c` | `void` | Wpisana w `MenuSceneEntries[1].init` (nieużywana scena 1). |
| `0x8002E920` | `EndingModeNoop` | `src/main/PAL/game/menu/opening.c` | `void` | Wywoływana przez `UpdateMenuScript` (`menu.c`), gdy `MenuScriptIndex == 4` (skrypt napisów końcowych Grand Prix). |
| `0x80036818` | `RaceResultStepNoop` | `src/main/PAL/game/race/result4.c` | `return 0;` | Wywoływana przez `PostRaceScreenModeUpdate` w stanie `SubmodeState == 1`. |
| `0x80039980` | `CarPreviewStepNoop` | `src/main/PAL/game/menu/menuwork.c` | `return 0;` | Wywoływana przez `CarPreviewModeUpdate`. |
| `0x8003AAF0` | `MenuScriptWaitStep` | `src/main/PAL/game/menu/menu.c` | `void` | Handler oczekiwania w `MenuScriptOps[10]`. |
| `0x80044AD0` | `RenderDebugNoop` | `src/main/PAL/game/gfx/gfxdraw.c` | `void` | Wywoływana w każdej klatce na końcu `RenderRaceFrame` (`0x80044250`, tuż po `DrawLensFlareAndSun()`). W buildach debugowych rysowała diagnostykę 3D wyścigu na ekranie. |
| `0x8004DC84` | `LoadRaceSoundVabStep` | `src/main/PAL/game/data/trkld1.c` | `return 1;` | Wywoływana przez `LoadRaceStep` (`0x8004D7D8`); zaślepiona po scaleniu ładowania audio wyścigu do `LoadRaceAudioStep`. |
| `0x8004DC8C` | `LoadRaceSoundSeqStep` | `src/main/PAL/game/data/trkld1.c` | `return 1;` | Wywoływana przez `LoadRaceStep` tuż po `LoadRaceSoundVabStep`. |
| `0x8004E184` | `LoadMenuSoundVabStep` | `src/main/PAL/game/data/trkld1.c` | `return 1;` | Wywoływana wyłącznie przez martwe `LoadMenuSoundStep` (`MenuScriptOps[9]`). |
| `0x8004E18C` | `LoadMenuSoundSeqStep` | `src/main/PAL/game/data/trkld1.c` | `return 1;` | Nieużywana zaślepka w `trkld1.c`. |
| `0x8006E9CC` | `CourseViewInitNoop` | `src/main/PAL/game/camera/viewini.c` | `void` | Nieużywana zaślepka na końcu `viewini.c`. |
| `0x8007D558` | `CardSlotUiNoop` | `src/main/PAL/game/menu/menutra22.c` | `void` | Nieużywana zaślepka na końcu `menutra22.c`. |
| `0x800816F0` | `_SsTrapIntrStub` | `src/main/PAL/psyq/libsnd/vsync.c` | `void` | Pusta zaślepka przerwania w `libsnd`. |
| `0x80086EC8` | `_SpuCallbackStub` | `src/main/PAL/psyq/libspu/spucall.c` | `void` | Nieużywana zaślepka w `libspu`. |
| `0x801151B0` | `ovl663_records_noop` | `src/overlays/PAL/records/selection.c` | `void` | Wywoływana przez `ovl663_update_menu` w overlayu Records (`ovl663`). |

### 5.2 Wszystkie wywołania `DebugPrintStub` w kodzie gry Namco (`main.exe` i overlaye)

Na samym początku sekcji `.rodata` pliku `main.exe` pod `0x80010000` zachował się również dokładny znacznik czasu kompilacji buildu PAL: **`"14:13 3/18/99"`** (18 marca 1999, godz. 14:13).

#### Wywołania w rezydentnym kodzie gry (`main.exe`)
| Plik i linia | Funkcja wywołująca | Wywołanie | Ciąg formatujący i adres VRAM |
| :--- | :--- | :--- | :--- |
| `carlane.c:113` | `InitComCars` (`0x8001A6EC`) | `DebugPrintStub("InitComCar OK!\n", i);` | `"InitComCar OK!\n"` (`0x80010018`) |
| `cdfile1.c:80` | `InitCdFileDirectory` (`0x800122A8`) | `DebugPrintStub(NowLoadingFmt, R4BinPath, CdDirectoryBuffer);` | `"Now Loading [%s]->[0x%08x] ... "` (`0x80010028`) |
| `cdfile1.c:84` | `InitCdFileDirectory` (`0x800122A8`) | `DebugPrintStub(StrFileNotFoundFmt, R4BinPath);` | `"File not found: %s\n"` (`0x80010054`) |
| `cdfile1.c:95` | `InitCdFileDirectory` (`0x800122A8`) | `DebugPrintStub(..., sectors);` | `"read:%dsector\n"` (`0x80010068`) |
| `gtepoly1.c:813` | `DrawRroModel` (`0x80014A98`) | `return DebugPrintStub(UnknownRroTypeFmt, command, count);` | `"unknown RRO type %x\n"` (`0x800100B8`) |
| `sky.c:609` | `SubmitCourseHorizon` (`0x800183BC`) | `DebugPrintStub(UnknownSkyModeFmt, k, w);` | `"unknown mode %x\n"` (`0x800100D0`) |
| `frmenv.c:75` | `InitGameSystems` (`0x8001DFBC`) | `DebugPrintStub(StrTotalInitLog);` | `"total_init\n"` (`0x800102F4`) |
| `ranking.c:117` | `SplashModeEnter` (`0x8001F2D4`) | `DebugPrintStub(&StrTitle0Log);` | `"title0\n"` (`0x800103B8`) |
| `modeinit.c:97` | `TitleModeEnter` (`0x8002E9B0`) | `DebugPrintStub(StrMainMenu0Log);` | `"main_menu0\n"` (`0x800104A4`) |
| `modeinit.c:141` | `TitleMenuInit` (`0x8002EAF8`) | `DebugPrintStub(StrInitMainMenu0);` | `"init_main_menu0\n"` (`0x800104B0`) |
| `result4.c:208` | `NewCarModeEnter` (`0x800369E8`) | `DebugPrintStub(NewCar0Msg);` | `"newcar0\n"` (`0x800106EC`) |
| `result4.c:216` | `NewCarModeUpdate` (`0x80036A44`) | `DebugPrintStub(StrNewCarLog);` | `"newcar\n"` (`0x800106F8`) |
| `mnrows.c:174` | `InitMenuSceneLayout` (`0x8003AEC0`) | `DebugPrintStub(StrCdReadPtrFmt, LoadCursor);` | `"cd_read_ptr = 0x%lx\n"` (`0x8001075C`) |
| `gfxmtx.c:124` | `AllocBadgeRows` (`0x8003D274`) | `DebugPrintStub(StrSizeEqLdFmt, n);` | `"size = %ld\n"` (`0x80010874`) |
| `uidraw.c:132` | `AllocRect16Rows` (`0x8003E814`) | `DebugPrintStub(StrSizeEqLdFmt, n);` | `"size = %ld\n"` (`0x80010874`) |
| `uidraw.c:240` | `AllocUiPrimitiveBuffer` (`0x8003EC24`) | `DebugPrintStub(SizeLdFmt, bytes);` | `"size = %ld\n"` (`0x800108D8`) |
| `uiwin.c:186` | `AllocWindowWork` (`0x800405A4`) | `DebugPrintStub(SizeLdFmt, sz);` | `"size = %ld\n"` (`0x800108D8`) |
| `pad.c:308` | `PadRangeCheckSet` (`0x8004B738`, *martwa*) | `DebugPrintStub(StrControllerNoErrorFmt, a);` | `"Controller no error! - %d\n"` (`0x80011014`) |
| `pad.c:311` | `PadRangeCheckSet` (`0x8004B738`, *martwa*) | `DebugPrintStub(StrActuatorNoErrorFmt, b);` | `"Actuator no error! - %d\n"` (`0x80011030`) |
| `sndvab.c:26` | `UploadVabSoundBank` (`0x8004ED38`) | `DebugPrintStub(VabOpenHeadErrorStr);` | `"SsVabOpenHead Error!\n"` (`0x800110D8`) |
| `sndvab.c:32` | `UploadVabSoundBank` (`0x8004ED38`) | `DebugPrintStub(VabTransBodyErrorStr);` | `"SsVabTransBody Error\n"` (`0x800110F0`) |
| `cdmain.c:162` | `StepCdAudioStateMachine` (`0x800521E8`) | `DebugPrintStub(XaRepeatMsg, &SoundState);` | `"XaRepeat\n"` (`0x8001118C`) |

#### Wywołania w overlayach (ujawniające wewnętrzne nazwy zadań deweloperów Namco)
| Overlay | Plik i linia | Funkcja wywołująca | Ciąg formatujący i adres VRAM |
| :--- | :--- | :--- | :--- |
| **ovl660** (`time_attack`) | `course_menu.c:71` | `ovl660_init_scene` (`0x80114C40`) | `"job_ta_crs_info0\n"` (`0x8011487C`) |
| **ovl661** (`versus`) | `preview.c:142` | `ovl661_init_menu` (`0x80115DB8`) | `"job_vs_car_info0\n"` (`0x80114888`) |
| **ovl662** (`garage`) | `progress_menu.c:65` | `ovl662_init_garage_scene` (`0x80114B9C`) | `"garage0\n"` (`0x8011487C`) |
| **ovl662** (`garage`) | `team_car_select.c:88` | `ovl662_init_team_car_choice` (`0x801153B0`) | `"job_garage_car_info0\n"` (`0x80114888`) |
| **ovl662** (`garage`) | `editor_lifecycle.c:205` | `ovl662_enter_editor_clean` (`0x8011C864`) | `"enter SaveBack\n"` (`0x801148F0`) |
| **ovl662** (`garage`) | `editor_lifecycle.c:218` | `ovl662_enter_editor_restore` (`0x8011C8E0`) | `"enter LoadBack\n"` (`0x80114900`) |
| **ovl663** (`records`) | `menu.c:36` | `ovl663_init_scene` (`0x80114A60`) | `"recode0\n"` (`0x8011487C` — literówka od *record*) |
| **ovl665** (`options`) | `screen_adjust.c:13` | `ovl665_init_screen_adjust` (`0x80117E08`) | `"Screen:%d, %d\n"` (`0x801148B4`) |
| **ovl666** (`extra_trial`) | `course_menu.c:53` | `ovl666_init_scene` (`0x80114B14`) | `"job_ex_crs_info0\n"` (`0x8011487C`) |
| **ovl668** (`movie_playback`) | `stream.c:119` | `ovl668_stream_next_frame` (`0x80114BE8`) | `"time out in strNext() %d\n"` (`0x8011487C`) |
| **ovl668** (`movie_playback`) | `mdec.c:81` | `ovl668_mdec_reset` (`0x801161BC`) | `"MDEC_rest:bad option(%d)\n"` (`0x801148C0`) |
| **ovl668** (`movie_playback`) | `mdec.c:146` | `ovl668_mdec_handle_timeout` (`0x80116528`) | `"%s timeout:\n"` (`0x801148FC`) |

### 5.3 Zablokowane flagi debugowe i liczniki telemetryczne
1. **`CD_debug` (`0x800A66C4` w `.data`, zainicjalizowane na `0`)**:
   - Odczytywane w 20 miejscach w `src/main/PAL/psyq/libcd/` (`bios_1.c`, `cdfile.c`, `dsfile.c`, `s_3.c` w warunkach `if (CD_debug > 0)` / `if (CD_debug >= 2)`).
   - Ponieważ funkcja ustawiająca `CdSetDebug` (`0x80091E28` w `s_4.c`) nie ma ani jednego wywołania, `CD_debug` wynosi zawsze `0`, a wszystkie 20 gałęzi diagnostycznych CD jest nieosiągalnych.
2. **`GraphDebugLevel` (`0x800A6898` w `.data`, zainicjalizowane na `0`)**:
   - Odczytywane w 14 miejscach w `src/main/PAL/psyq/libgpu/` (`if (GraphDebugLevel > 0)` / `if (GraphDebugLevel >= 2)`).
   - Funkcja `SetGraphDebug` (`0x80094BA4` w `e_1.c`) jest wywoływana tylko raz w całej grze z argumentem `0` (`SetGraphDebug(0)` w `InitGameSystems`, `frmenv.c:81`), przez co wszystkie gałęzie logujące `GraphDebugLevel > 0` są martwe.
3. **`ovl667_sio_error_count` (`0x8011A5A8` w `ovl667`)**:
   - Zerowany w `ovl667_init_serial_link` i inkrementowany przy błędach ramki/parzystości/nadpisania w `ovl667_sio_interrupt_handler` (`sio.c:216`), ale jego jedyny czytelnik `ovl667_get_sio_error_count` (`0x80115048`) jest martwą funkcją.

---

## 6. Podsumowanie 191 nieużywanych funkcji SDK Sony PsyQ (`0x8007D718..0x8009DF14`)

Spośród 540 funkcji bibliotek Sony PsyQ zlinkowanych do `main.exe`, **191 funkcji (35,4%)** jest nieosiągalnych (127 bezpośrednio + 64 przechodnio):
- **Programowa arytmetyka `double` (`libmath`/`libgcc`, 16 funkcji, `0x8007D7D4..0x8007EA28`)**: `__adddf3`, `_add_mant_d`, `_dbl_shift`, `__divdf3`, `_comp_mant`, `_dbl_shift_us`, `__fixdfsi`, `__floatsidf`, `__gtdf2`, `__ltdf2`, `_mainasu`, `__muldf3`, `_mul_mant_d`, `__negdf2`, `__subdf3`, `_err_math` — zaciągnięte wyłącznie przez martwe `SetFogNearFar` (`0x8006F2E0`).
- **Dźwięk (`libsnd` / `libspu`, 38 funkcji)**: m.in. `SsUtChangePitch`, `SsUtGetVabHdr`, `SsUtAllKeyOff`, `SsPitchFromNote`, `SsVabOpenHeadSticky`, `SsVabTransBodyPartly`, `SsVabClose`, `SsSeqPause`, `SsSeqReplay`, `SsSeqSkip`, `SsSetLoop`, `SsIsEos`, `SpuGetVoiceEnvelope`, `SpuGetVoicePitch`, `SpuGetVoiceVolume`, `SpuSetNoiseClock`, `SpuSetPitchLFOVoice`, `SpuSetTransferStartAddr`, `SpuRead`.
- **GTE (`libgte`, 40 funkcji)**: m.in. `InitClip`, `VectorNormal`, `VectorNormalS`, `VectorNormalSS`, `SquareRoot12`, `InvertMatrix`, `MatrixNormal_0`, `MatrixNormal_1`, `MatrixNormal_2`, `OuterProduct12`, `OuterProduct0`, `Lzc`, `RotTransPers`, `RotTrans`, `LocalLight`, `LightColor`, `DpqColorLight`, `DpqColor`, `Intpl`, `Square12`, `Square0`, `NormalColor`, `NormalColor3`, `NormalColorDpq`, `NormalColorDpq3`, `NormalColorCol`, `NormalColorCol3`, `ColorDpq`, `ColorCol`, `AverageZ3`, `AverageZ4`, `LoadAverageShort12`, `LoadAverageByte`, `LoadAverageCol` (Namco korzysta z własnych procedur GTE w `gtepoly1.c`, `gtepoly2.c` i `SubmitModel.s`).
- **CD-ROM (`libcd`, 23 funkcje)**: m.in. `CdInitSubFunc`, `CdStatus`, `CdMode`, `CdLastCom`, `CdReset`, `CdFlush`, `CdSetDebug`, `CdComstr`, `CdIntstr`, `CdGetSector2`, `CdDataSync`, `CdDataCallback`, `CdReadBreak`, `CdReadSync`, `CdReadCallback`, `CdRead2`, `StClearRing`, `StUngetRing`, `StSetChannel`, `StLockRing`.
- **GPU (`libgpu`, 26 funkcji)**: m.in. `GetGraphType`, `GetGraphDebug`, `DrawSyncCallback`, `LoadImage2`, `StoreImage2`, `ClearOTag`, `AddPrim`, `AddPrims`, `CatPrim`, `TermPrim`, `SetSemiTrans`, `SetShadeTex`, `SetPolyFT3`, `SetPolyGT3`, `SetPolyG4`, `SetPolyFT4`, `SetSprt8`, `SetSprt16`, `SetSprt`, `SetTile1`, `SetTile8`, `SetTile16`, `SetTile`, `SetLineF2`, `SetLineG2`, `SetLineF3`, `DumpDispEnv`.
- **Karta pamięci, kontroler i BIOS (`libcard` / `libpad` / `libapi`, 48 funkcji)**: m.in. `_card_info`, `_card_load`, `_card_auto`, `StopCARD`, `StopCARD2`, `_ExitCard`, `_cardStopCom`, 23 niskopoziomowe handlery pakietów `_cardCmd*`, a także `PadStopCom`, `PadEnableCom`, `PadGetState`, `PadInfoComb`, `PadSetActAlign`, `InitPAD`, `StartPAD`, `StopPAD`, `EnablePAD`, `DisablePAD`.
