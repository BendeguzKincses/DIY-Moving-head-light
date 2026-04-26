# Robotlámpa megvalósítása innovatív eszközökkel
**34. Országos Tudományos és Innovációs Olimpia (2025.04.07.)** **Készítette:** Kincses Bendegúz

---

## Tartalomjegyzék
1. [A robotlámpák felhasználása és az ötletem eredete](#a-robotlámpák-felhasználása-és-az-ötletem-eredete)
2. [Piackutatás és a projektem célja](#piackutatás-és-a-projektem-célja)
3. [Általános műszaki adatok](#általános-műszaki-adatok)
4. [Mechanikai megvalósítás](#mechanikai-megvalósítás)
5. [Elektronikai megvalósítás](#elektronikai-megvalósítás)
6. [Szoftveres megvalósítás](#szoftveres-megvalósítás)
7. [A projektem célja](#a-projektem-célja)

---

## A robotlámpák felhasználása és az ötletem eredete
A robotlámpák általánosságban, színpadi megvilágításra használt, fénytechnikai eszközök. Napjainkban rendkívül elterjedt eszközöknek számítanak, megtalálhatóak a vidéki színházaktól a legszínvonalasabb nemzetközi koncertekig bezáróan az összes színpadon. 

Számomra a színpadtechnika már általános iskolás korom óta felkeltette a figyelmem. Középiskolás koromban pedig kialakult bennem egy szűkebb érdeklődés a robotlámpák és azokban felhasznált technológiák iránt. Technikumi tanulóként elhatároztam, hogy próbára teszem a kitartásom és elkészítek egy a robotlámpák kategóriájába tartozó eszközt, a számomra elérhető eszközökkel és anyaggokkal, a lehető legmagasabb műszaki színvonalon. 

A projektem célja, hogy jómagam és a későbbiekben mások is képesek legyenek minél többet tanulni egy ilyen bonyolult eszköz megtervezéséből és megépítéséből.

## Piackutatás és a projektem célja
A színpadtechnikában használt eszközökön belül az általam megvalósított robotlámpa a *moving head spot light* kategóriába sorolható be. Ez egyike a piacon található robotlámpák három nagy kategóriájának, a *beam*, *spot* és *wash*-nak. Ezen belül a *moving head spot light with motorized zoom* kategóriába tartozik a projektem. 

Magyarul megfogalmazva: **mozgó fejű folt-pötty lámpa motorizált kicsinyítés-nagyítás funkcióval**. 
A "mozgó fejű" kifejezés arra utal, hogy a lámpának a fejét két tengely mentén, távezérléssel robotizáltan lehet forgatni. A "folt-pötty" kifejezés a lámpa vetítési képének a formájára utal, míg a motorizált kicsinyítés-nagyítás azt jelenti, hogy távezérléssel, motorizáltan lehet a vetített folt méretét változtatni.

Az eszközöm funkcióit tekintve nagyban hasonlít a piacon kapható azonos kategóriába sorolható robotlámpákkal. Például ugyanolyan figyelemre méltó fényerővel rendelkezik. Ezt a robotlámpát úgy terveztem, hogy minden egyes eleme alkalmas legyen a tanulásra. Az alkatrészek viszonylag könnyen elkészíthetőek és beszerezhetőek, az eszköz minden egyes eleme szétszedhető és újból összerakható, az áramkörök tervezése és kialakítása is tanuló barát, nem mellesleg könnyen javíthatóak.

## Általános műszaki adatok
Az általam megvalósított robotlámpa műszaki szempontból egy kimondottan összetett eszközt alkot. Sokféle alrendszer végzi el a feladatait egymással összhangban működve.

* **Az eszköz méretei:** szélesség = 300 mm, mélység = 260 mm, magasság = 414 mm
* **Az eszköz tömege:** 8 kg
* **Forgástengelyek száma:** 2
* **Forgástengelyek elfordulási szöge:** pan tengely = 360°, tilt tengely = 200°
* **Maximális fényteljesítmény:** 12 000 lm
* **Színek:** 14 féle különböző szín
* **Kisugárzási szög:** 5°-50°, motorizált
* **Vezérelhetőség:** DMX512, Előlapon található gombokkal manuálisan
* **Elektromos teljesítmény:** 200 W

## Mechanikai megvalósítás
Az eszközt a tervezés során először létrehoztam virtuális formában egy CAD tervező programban (SolidWorks). Az egész lámpa a legutolsó csavarig le van modellezve virtuálisan a vezetékezéstől eltekintve. 

**Felhasznált főbb anyagok:** 3 mm-es ABS műanyag lemez, PLA és PETG műanyagok, M6-os menetes rúd és M6-os anyacsavarok, M3-as méretű szerelékek, 3 mm-es nyír rétegelt lemez, 2 mm-es alumínium lemez.

A lámpa legtöbb alkatrésze FDM 3D nyomtatással készült el (PLA és PETG). A robotlámpa szerkezetének a vázát M6-os menetes rudak képezik, ezek alkotják azt az erős fém vázat, amin aztán a 3D nyomtatott statikai alkatrészek betölthetik a szerepüket. A nyír rétegelt lemezek precíz megmunkálása diódás lézervágóval történt (pl. a lámpafejben található világító egység felépítménye).

A tengelyeket léptetőmotorok forgatják, GT2-es fogasszíj és fogasszíj tárcsákon keresztül. A lámpa alsó részében található tengely alkotja a *pan* tengelyt és a lámpafejnél található tengely pedig a *tilt* tengelyt. 

## Elektronikai megvalósítás
A robotlámpában egy bonyolult elektromos rendszer teszi lehetővé az eszköz megfelelő üzemelését. Az elektromos rendszer felelős, hogy a beérkező DMX512-es jelet konvertálja a mikrokontroller által is feldolgozható típusú jellé.

Az eszközben két saját tervezésű nyomtatott áramköri panel (NYÁK) található (tervezve EasyEDA-ban):
1. **MySpotV3:** Ezen a panelen található egy ESP32-es mikrokontroller modul és egy ATmega328P mikrokontroller. Együttesen, egymással folyamatosan kommunikálva végzik el a robotlámpa üzemszerű működéséhez szükséges számításokat (pl. DMX jel dekódolása TTL jelszintre SN7517 IC-vel, léptetőmotorok vezérlése).
2. **MySpotV4:** Ezen egy ESP32-es mikrokontroller modul szabályozza a nagy teljesítményű, 150W-os COB LED-et és annak hűtését (hőmérséklet alapú PWM ventilátor szabályozás). 

A robotlámpát egy 20 V 230 W leadására képes kapcsoló üzemű laptop tápegység látja el egyenárammal. A 20 V lineáris feszültség szabályzó IC-k segítségével átalakításra kerül 12 V-ra (ventilátorok) és 5 V-ra (mikrokontrollerek, motorvezérlők).

## Szoftveres megvalósítás
A robotlámpában található három mikrokontroller I²C kommunikációs rendszer segítségével küld és fogad adatokat. A programkód Arduino IDE programozási felületen került megírásra (C++ környezet).

A MySpotV3 panelen található ESP32 mikrokontrolleren futó kód **két külön processzor magra** oszlik el:
* **Core 0:** A léptető motor vezérlők folyamatos irányítása (AccelStepper könyvtár).
* **Core 1:** I²C kommunikáció, felhasználói OLED kijelző és gombok kezelése, relé és ventilátor vezérlés, valamint a DMX csatornák adatainak feldolgozása.

A DMX512 adatfolyam olvasását és UART feldolgozását az ATmega328P végzi, amely továbbítja az utasításokat az ESP32 felé.

## A projektem célja
A robotlámpa, amit létrehoztam belső felépítését tekintve teljes mértékben különbözik a piacon megvásárolható hasonló eszközöktől. Egy professzionális, tömeggyártásra optimalizált eszköz nem úgy lett tervezve, hogy abból tanulni lehessen. 

Azért fontos, hogy az általam elkészített robotlámpa moduláris és átlátható, mert ezzel lehetővé teszi a tanulást. Elektronikai technikusnak tanuló diákként alkottam meg ezt azt eszközt, de úgy gondolom, hogy egyetemisták is sokat tanulhatnának az eszközöm elemzéséből vagy továbbfejlesztéséből. Az internet világában egy ilyen projektet úgy lehet a legjobban elérhetővé tenni mindenkinek, ha nyílt forráskódúvá (open-source) tesszük, ezért publikálom a teljes dokumentációt.
