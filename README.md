# Zadatak i cilj projekta

Zadatak ovog projekta bila je implementacija drivera u programskom jeziku C, specificiranog za framework ESP-IDF, koji omogućuje čitanje, slanje i obradu podataka, te osnovnu konfiguraciju mmWave senzora upravljanog ESP32 mikrokontrolerom. Takav driver nudi API za rad s mmWave senzorom koji je u svojem većem dijelu platform-independent.

Cilj je razviti driver koji je što je moguće više portabilan, pri čemu je platform-specific dio ograničen na minimalni sloj vezan uz ESP-IDF, dok je ostatak funkcionalnosti implementiran neovisno o platformi. Tako se većina koda može koristiti neovisno o platformi i mikrokontroleru koji bi izvršavao takav kod. Ovakav pristup olakšava testiranje, povećava ponovnu iskoristivost koda i značajno utječe na širinu polja na kojima se driver može primjenjivati.

# Opis razvijenog proizvoda

Razvijeni driver pruža mogućnosti inicijalizacije, pokretanja, zaustavljanja i deinicijalizacije sustava, te slanja upita (inquiry) i primanja podataka (konkretno, podataka o udaljenosti, kretanju i aktivnosti osobe u nekom prostoru) preko UART sučelja i to koristeći dvije vrste događaja (eventova) koji opisuju primljene podatke sa senzora - izvještaje (reports) i odgovore (responses). Driver za primanje podataka nudi hibridni model obavještavanja o novim događajima - putem registriranih callback funkcija i putem queue-a. Konfiguracija senzora (postavljanje modova, osjetljivosti (sensitivity), načina čitanja podataka, ali i osnovno konfiguriranje) vrši se preko vanjskog uniformnog API-ja.

Driver počiva na portabilnoj arhitekturi podijeljenoj na četiri glavna sloja. Svaki sloj ima važnu ulogu i samostalno obavlja određenu funkcionalnost sustava, ali slojevi su povezani i moraju komunicirati kako bi se uspješno prenijeli zahtjevi i podatci, te kako bi sustav normalno i očekivano funkcionirao.

Ta četiri glavna sloja su:
* **Platform sloj**
* **Hardware Abstraction Layer (HAL) sloj**
* **Mmwave core sloj**
* **Application sloj**

<br/>

**Platform sloj** - odgovoran za pristup hardware-u, implementira osnovne low-level funkcije (upravljanje dinamičkom memorijom, time, queue, eventi, taskovi, UART) koje ostali slojevi koriste pozivanjem platform sloja. API platform sloja je platform-independent, dok se njegova implementacija mijenja ovisno o platformi i trenutna je prilagođena ESP-IDF frameworku.

**HAL sloj** - "ljepilo sloj" - odgovoran za povezivanje ostalih slojeva. HAL sloj ponaša se kao sabirnica podataka i vlasnik je sve memorije i struktura podataka koje se koriste u driveru za obradu podataka. Kroz njega se odvija protok podataka od senzora, preko core sloja (parser) pa sve do aplikacijskog sloja. Također, služi kao podloga core i application sloju za spremanje i dohvaćanje podataka.

**Mmwave core sloj** - poznaje protokol i vrši sve operacije za koje je potrebno znanje protokola kojim mmwave senzor šalje podatke. Glavne funkcionalnosti su parsiranje podataka i stvaranje okvira za upite. Core sloj je potpuno platform-independent jer jedine funkcije koje ovise o platformi obavlja preko HAL callbackova.

**Application sloj** - krovni sloj koji služi za semantičko tumačenje podataka. Dakle, application sloj se koristi kao dekoder, tj. onaj koji "razumije" što bajtovi (posebno payload) iz okvira znače, te razdvaja odgovore na odgovore na upite (response) i aktivne izvještaje (report). Također, ovaj sloj izlaže API vanjskom korisniku - tj. on je izlazna točka drivera. Sam sloj sastoji se od više podslojeva - managera (upravlja application slojem i razgovara s HAL-om), dekodera podataka, izlaznog podsloja i config podsloja (samo definira konfiguracije za HAL i core). Svi ti podslojevi moraju biti prisutni kako bi application sloj normalno funkcionirao.

Dodatno, driver koristi pomoćni sloj koji samo definira konstante vezane uz konkretne pinove i specifikacije mikrokontrolera i njihovih komunikacijskih sustava - **board pomoćni sloj**.

Upravo ova podjela na slojeve pruža modularnost i ponovnu iskoristivost koda, jer za prilagodbu za novu platformu, komunikacijsko sučelje ili protokol senzora potrebne su minimalne promjene koda. U tom slučaju potrebno je dodati nove implementacije platform API-ja, djelomične promjene na HAL sloju i drugačije konstante u board pomoćnom sloju. Svi ostali dijelovi sustava ostaju isti i ne zahtijevaju nikakve promjene.

Razvijeni driver je user-friendly i lagan za korištenje jer API koji komunicira s vanjskim programom ne zahtijeva nikakvo poznavanje implementacije sustava, iako se mora znati što koji podataka predstavlja (nema posebnih tipova za određene podatke senzora na application sloju).

# Tehničke značajke

Sustav je dizajniran kao portabilni C driver s izoliranim platform-independent dijelovima, što omogućuje lagano proširenje funkcionalnosti, ponovnu uporabu koda, ali i promjenu platforme.

Sustav počiva na platform sloju, čiji je API platform-independent, ali implementacija podložna platformi. Tu se većina promjena kod prelaska sustava na drugačiji mikrokontroler ili platformu mora obaviti. Platform sloj obavlja low-level funkcije kao što su: funkcije za upravljanje dinamičkom memorijom (alokacija i free), taskovi (stvaranje, brisanje, delay, promjena prioriteta), sinkronizacija taskova (mutex funkcije), queue (stvaranje, dodavanje, dohvaćanje i brisanje), uart (inicijalizacija, pisanje, čitanje, ISR, deinicijalizacija) i upravljanje eventima (struktura eventa, stvaranje eventa, čekanje na eventove, ISR event management, ...). Implementacija API-ja napravljena je samo za ESP32 Wroom mikrokontroler, a u slučaju promjene mikrokontrolera ili platforme implementacija se mora prilagoditi.

HAL sloj obavlja širok spektar funkcionalnost. Jedna od tih funkcija je vlasništvo nad sustavskim strukturama podataka (svi queue-ovi koje sustav koristi se stvaraju, upravljaju i brišu u HAL sloju). Sljedeća važna funkcionalnost koju on vrši je upravljanje dodjelom memorije i zaštita sustava od prevelike (štetne) alokacije. HAL sloj također vrši usklađivanje rada application sloja i core sloja, pošto on interno drži state machine koja predstavlja stanje sustava. HAL sloj je prvi koji neće dozvoliti neispravne prijelaze između stanja. Kako bi se omogućila što veća neovisnost application sloja i core sloja o platformi, HAL sloj pruža podlogu preko koje ti slojevi mogu obavljati operacije nad sustavskim strukturama, i to, konkretno korištenjem callbackova. Parseru u core sloju preko callbackova omogućava dohvaćanje podataka iz UART RX buffera, te spremanje parsiranih podataka, a application sloju dohvaćanje parsiranih podataka koje on treba dekodirati i prikazati. HAL sloj djelomično je neovisan o platformi - API mu je generaliziran, ali implementacija izravno koristi platform sloj i njegove mehanizme, te on izravno drži strukture podataka koje se razlikuju s obzirom na platformu.

Poseban dio drivera svakako predstavlja core sloj, koji implementira najvažniju funkcionalnost sustava - parser. Taj sloj potpuno je neovisan o platformi, te sve svoje funkcionalnosti dohvaćanja i spremanja podataka vrši preko HAL callbackova. Parser pamti stanje u kojem se je nalazio, stoga je pogodan za obradu tokova koji šalju djelomične okvire, ali i za parsiranje skupa okvira odjednom. Uz to, parser ima interne restart funkcije i mehanizme koji mu omogućuju oporavak u slučaju redundantnih bajtova na početku ili kraju toka podataka koji prima. Uz funkciju parsiranja, core sloj obavlja i funkcionalnost izgradnje okvira iz samo semantički korisnih podataka, što je iznimno važno za slanje upita na senzor.

Konačno - zadnji, aplikacijski sloj sam u sebi se ponaša kao manji sustav, pošto se sastoji od više dijelova. Najvažniji dio tog sloja je aplikacijski dekoder - dio application sloja koji je "pametan". On u sebi ima logiku koja se koristi za dekodiranje semantički korisnih podataka iz okvira i njihovo tumačenje. On razdvaja odgovore na upite (responses) od aktivnih izvještaja senzora (reports) te drži tipove podataka za prikaz i spremanje te uspješan prijenos tumačenja na vanjski program. Dekoder koristi makroe koji predstavljaju semantiku određenih vrsta podataka sa senzora te callbackove koje mu pruža manager sloj kako bi sam po sebi bio platform-independent. Application manager je upravljački dio application sloja. On izravno komunicira s HAL-om i preuzima podatke iz HAL-a čime ownership prelazi na njega, pa se on mora i pobrinuti da se ta memorija oslobodi. Ima vlastiti task koji upravlja pozivanjem dekodera. Application manager osigurava da se sustav može uspješno inicijalizirati, startati, zaustaviti i deinicijalizirati (uglavnom wrapper HAL-ovog lifecycle-a). Također, on pruža funkcije koje podsloj koji izlaže API prema van omata i poziva (generalizirani inquiry, inicijalizacija sustava, startanje sustava, zaustavljanje sustava). Application sloj nudi hibridni način rada po primljenom eventu - može se pollati iz queue (pa praktički čekati event), a može se i registrirati callback-ove koje manager poziva po dolasku novog eventa (poseban za odgovore, a poseban za izvještaje), pa on može biti obavijest da je event stigao. Važno za napomenuti je da callback funkcije ne smiju blokirati i obrađivati eventove, već samo služe kao obavijest o dolasku novog izvještaja ili odgovora.

# Arhitektura

Dijagram arhitekture sustava:
<br/>
![Arhitektura mmWave drivera](images/layers_arch.drawio.png)
<br/>
<br/>
Dijagram application sloja:
<br/>
![Dijagram application sloja](images/app_l.drawio.png)

# Upute za korištenje

### Preduvjeti:
* ESP-IDF verzija 5.3.4
* Python 3.11 +
* osnovno poznavanje ESP-IDF projekata i UART komunikacije
* ESP32 mikrokontroler (implementirano za Wroom, za druge vrste promijeniti board datoteke)
* mmWave senzor Seeed studio MODEL: MR24HPC1
* FreeRTOS (uključen u ESP-IDF)
* Silicon Labs CP210x USB to UART Bridge driver (preuzeti s: https://www.silabs.com/software-and-tools/usb-to-uart-bridge-vcp-drivers)
* VSCode

### Smještaj korisničkog programa i integracija s driverom:

Prvenstveno, potrebno je izvršiti `git clone https://github.com/MFucek67/ProjektR.git` u root ESP-IDF projekta kako bi se dohvatio driver kod s remote repositoryja u lokalni.

Korisnički program najbolje je dodati kao posebnu komponentu u components te pozvati iz main, ali može se ići i s varijantom gdje se korisnički program piše izravno u main.c.

Kako bi se koristile API funkcionalnosti mapa u kojoj se korisnički program nalazi mora imati CMakeLists.txt s `REQUIRES app` (dakle, važno je navesti da komponenta koja predstavlja korisnički program uključuje app komponentu). Uz to, korisnički program mora uvesti header javnog API-ja (`#include app/app_mmwave.h`). Tek tada će linker znati prepoznati driver API koji se koristi.

Driver se koristi isključivo pozivima API funkcija iz app_mmwave.h - to je podsloj aplikacijskog sloja koji izlaže API drivera prema van.

### Korisnička konfiguracija:

Ako se koristi neka druga vrsta ESP32 (ne ESP32 Wroom), potrebno je izmijeniti UART konfiguraciju. Ona se nalazi u board komponenti. Konfiguracije UART veze nalaze se u datoteci `board_mmwave_uart_config.h`, a brojevi UART sučelja i pripadni TX/RX pinovi u datoteci `esp32_board.h`.

### Lifecycle sustava:

Sustav koji driver opisuje ponaša se kao state machine. Kako bi se driver (i sustav koji on predstavlja) ispravno koristio, važno se je pridržavati lifecycle-a drivera i ne pozivati funkcije izvan stanja u kojima su one dopuštene.

Ispravan lifecycle drivera:

1. Inicijalizacija sustava:
Korisnik poziva `mmwave_init()` čime se inicijalizira UART (postavkama konfiguracije iz board komponente), HAL i stvaraju se njegove interne strukture podataka. U ovom koraku korisnik još ne smije slati ili primati podatke. Jedina moguća stanja u koja se smije prijeći iz ovog koraka su RUNNING stanje (pozivom `mmwave_start()` funkcije) i UNINIT stanje (pozivom `mmwave_deinit()` funkcije).

2. Startanje sustava:
Korisnik poziva `mmwave_start()` čime se pokreću HAL taskovi, pokreću ISR eventi i pokreće rad parsera. Nakon ovog koraka korisnik smije slati i primati podatke. Jedino moguće stanje u koje se smije prijeći iz ovog koraka je STOP stanje (pozivom `mmwave_stop()` funkcije).

3. Zaustavljanje sustava:
Korisnik poziva `mmwave_stop()` što redom zaustavlja ISR, te receiver i sender taskove i na koncu zaustavlja rad parsera. U ovom koraku korisnik opet ne smije slati ili primati podatke. Jedina moguća stanja u koja se smije prijeći iz ovog koraka su DEINIT stanje (pozivom `mmwave_deinit()` funkcije) i RUNNING stanje (pozivom `mmwave_start()` funkcije).

4. Deinicijalizacija sustava:
Korisnik poziva `mmwave_deinit()` čime se deinicijalizira UART, HAL i brišu se njegove interne strukture podataka. U ovom koraku korisnik ne smije slati ili primati podatke. Jedino moguće stanje u koje se smije prijeći iz ovog koraka je INIT stanje (pozivom `mmwave_init()` funkcije).

### Slanje i primanje podataka:

Funkcije kojima se vrši slanje i primanje podataka smiju se pozivati isključivo u `APP_SENSOR_RUNNING` stanju.

Funkcije za primanje podataka preko eventova rade na principu pollinga.

Njihovi potpisi su:
* `bool mmwave_poll_response(DecodedResponse* out, uint32_t timeout_ms);` - za primanje response eventa (odgovor senzora)
* `bool mmwave_poll_report(DecodedReport* out, uint32_t timeout_ms);` - za primanje report objekta (izvještaj senzora)

Funkcije vraćaju true ako su event-ovi uspješno pollani, a false ako nisu.

API nudi brojne funkcije za slanje podataka (upiti, tj. inquiries). Njihove potpise može se pronaći u komponenti app u headeru `app_mmwave.h` (sve počinju s `app_inquiry_...`).

### Ograničenja i napomene:

Ograničenja:
* Koristi se jedan mmWave senzor
* Podrška samo za UART komunikaciju
* Nema automatskog detektiranja UART-a - korisnik postavke UART sučelja mora sam unijeti u board komponentu
* Maksimalan broj frame-ova koji queue prima je ograničen
* Ograničena je veličina payloada koji se može prenijeti u frame-u
* Funkcije za UART upgrade nisu implementirane

Napomene:
* Lifecycle ostalih komponenti je interno kontroliran - korisnik kontrolira lifecycle samo preko vanjskog API-ja
* Sve memorijske alokacije i oslobođenja heapa UNUTAR SUSTAVA vodi HAL, korisnik o njima ne mora brinuti
* Preporučuje se da korisnik ne mijenja platform, HAL, mmWave core i application slojeve, samo board i vanjsku aplikaciju

# Testiranje

Testiranje drivera izvršeno je pisanjem i pokretanjem zasebnih mock testova za HAL i mmWave core sloj koji simuliraju rad tih slojeva drivera zasebno.

Konačan test drivera i komunikacije između slojeva izvršen je pisanjem vanjskog programa (integracijski test cijelog driver sustava) koji poziva application sloj API koji je predviđen da se poziva izvana.

Integracijski test pokriva funkcionalnosti:
* Inicijalizacija sustava i svih podslojeva
* Pokretanje sustava
* Registriranje vanjskog callbacka na evente
* Slanje upita (inquiries) preko UART TX
* Primanje odgovora na upite
* Primanje reportova senzora
* Zaustavljanje rada sustava
* Deinicijalizacija sustava i svih podslojeva

Integracijsko testiranje je izvršeno korištenjem pravog mmWave senzora proizvođača Seeed studio MODEL: MR24HPC1 i mikrokontrolera ESP32 Wroom. Ispravno, odnosno neispravno ponašanje registrirano je ispisom na serijski monitor.

Ograničenja testiranja:
* Test je skroz jednostavan i ne pokriva većinu rubnih slučajeva
* Nema automatiziranih unit testova
* Sustav je testiran u kratkom vremenskom razdoblju - ne daje nam uvid kako bi se sustav ponašao nakon dužeg rada
* Nije testirano na više ESP32 varijanti

### Pokretanje mock testova i integracijskog testa:

Potrebno je fizički spojiti ESP32 Wroom UART NUM2 pinove s odgovarajućim pinovima mmWave core Seeed studio MODEL: MR24HPC1 senzora, te potom spojiti ESP32 Wroom mikrokontoroler na računalo putem USB2.0 to USB micro B kabla.

**VAŽNO:** TX pin ESP32 -> RX pin senzora, RX pin ESP32 -> TX pin senzora

U main.c datoteci u folderu main potrebno je u funkciji `void app_main(void)` zakomentirati one testove koji se ne žele izvršiti (može se paralelno izvršiti i dva, pa čak i sva tri testa).

Funkcije testova su:
1. HAL mock test - funkcija `hal_mmwave_run_test()`
2. mmWave core mock test - funkcija `mmwave_core_run_test()`
3. integracijski test cijelog driver sustava - funkcija `app_mmwave_run_test()`

Kada se odrede testovi ili test koji se želi izvršiti, potrebno je pokrenuti **Build, Flash and Monitor**.

Prikazana su dva najčešća načina pokretanja naredbe Build, Flash and Monitor:

Pokretanje naredbe iz VSCode s instaliranom ekstenzijom ESP-IDF publishera Espressif Systems:
1. Provjeriti da je ESP-IDF ekstenzija instalirana i aktivna
2. Otvoriti ESP-IDF terminal u VSCode (View -> Command Palette -> ESP-IDF: Open ESP-IDF Terminal)
3. Postaviti se u root projekta
4. Provjeriti da je ESP32 spojen preko USB-a: `idf.py monitor`
5. Pokrenuti komandu za Build, Flash and Monitor: `idf.py build flash monitor` ili stisnuti na shortcut za komandu (vatrica u izborniku alata u footeru VSCode-a)

# Zaključak i budući rad

## Zaključak:
U ovom projektu razvijen je slojeviti driver za mmWave senzor s jasnom podjelom odgovornosti na slojeve. Arhitekturom drivera postignuto je da se što veći dio koda može ponovno koristiti i da što veći dio koda bude neovisan o platformi. Cilj rada - razvoj što portabilnijeg, što lakše testabilnog i što više platform-independent koda je uspješno ostvaren.

## Prijedlozi za budući rad:
Razvijeni driver je samo osnova i pruža mogućnost nadogradnje sustava u puno dodatnih smjerova. Nadogradnje i poboljšanja mogući su u funkcionalnosti, skalabilnosti i sigurnosti te portabilnosti.

Što se tiče funkcionalnosti najbolje nadogradnje bile bi podrška za više mmWave senzora, podrška za više komunikacijskih sučelja (ne samo UART, nego npr. SPI, I2C, itd.) te automatsko prepoznavanje pinova i broja sučelja za komunikaciju.

U području skalabilnosti i sigurnosti, sustav se može unaprijediti povećanjem broja testova i njihovim automatiziranjem, testiranjem na dugotrajni rad te testiranjem ranjivih dijelova koda i njegovom optimizacijom.

Da bi driver bio portabilniji, moguće je određene dijelove koda još dodatno generalizirati i apstrahirati te testirati na drugim platformama.

# Literatura

Korištena literatura:
<br/>
[1] Espressif ESP-IDF API Reference - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/index.html
<br/>
[2] Seeed studio 24GHz mmWave Sensor User Manual and User Protocol - https://www.seeedstudio.com/24GHz-mmWave-Sensor-Human-Static-Presence-Module-Lite-p-5524.html?srsltid=AfmBOophlyqjC5LkABsoruUA5EAaO8AEiMufJYZ0wpPq9n-xyLOR4dGm
<br/>
[3] Doxygen docs - https://www.doxygen.nl/manual/index.html
<br/>
[4] draw.io - https://www.drawio.com/
<br/>
[5] Analog Devices Analog Dialogue DEC 2020 VOL 54
<br/>
[6] embedUR - Embedded System Ecosystem - https://www.embedur.ai/embedded-system-ecosystem/
<br/>
[7] Embedded RELATED.com - Creating a Hardware Abstraction Layer (HAL) in C - https://www.embeddedrelated.com/showarticle/1596.php