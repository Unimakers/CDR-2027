# Portage DevKitC-1 → XIAO ESP32-S3 Plus (carte body v1)

Code repris de `Firmware_Robot_Principale` (non modifié), brochage relu sur le schéma
**et** le PCB de `ECAD-2026` : `body v1`, `leg v1` (cartes driver), `head v1`.

## Ce qui change

| Fonction | Avant (DevKitC-1) | Maintenant (XIAO S3 Plus) |
|---|---|---|
| STEP/DIR | GPIO directs | GPIO directs (autres numéros, voir `board_pins.h`) |
| ~EN drivers | GPIO1, commun aux 4 | **PCF8574 body (0x20)** : P3 (carte NP1), P4 (carte NP2) |
| MS1/MS2 | jumpers à souder | **PCF8574 body** : réglés par le firmware depuis `robot_config.h` |
| I2C | 17 / 16 | 5 / 6 |
| PCA9685 | sur la carte, ~OE câblé | modules externes sur IC1..IC4, ~OE non câblé |
| Switchs | PCF carte principale P3..P7 | PCF de la tête (0x21), J1..J4 = P0..P3 |
| Tirette | GPIO21 | GPIO12 (via tête) + switch équipe GPIO11 |
| LD06 | UART1 8/9, PWM 18 | UART1 RX 43 / TX 44, PWM 8 (via tête, comme le code 2026) |
| XBee | — | UART2 RX 41 / TX 42 |
| LED RGB | — | 2 × WS2812B, GPIO7 |
| TFT, boutons | oui | supprimés |
| En 5VP | GPIO2 | GPIO13 |

Nouveaux fichiers : `pcf8574.h/.cpp` (driver PCF), `inputs.h/.cpp` (tirette, switch équipe,
switchs de la tête), `breadboard/test5_pcf_inputs.cpp`.

## Roues (repris du firmware 2026, même carte)

Roue gauche = slot 1 (U2 de la carte driver sur NP1, STEP 40 / DIR 10, inversée),
roue droite = slot 3 (U2 de la carte driver sur NP2, STEP 38 / DIR 39).
Les U1 (slots 0 et 2) sont pour les actionneurs (ascenseurs en 2026).
Vitesse 4494 Hz, accélération 7490 Hz/s, 1/16 (= 300 mm/s, 500 mm/s² avec des roues de 68 mm).

## Points matériels à vérifier

1. **STEP/DIR inversés sur U1 des cartes driver** (`leg v1`) : le net « STEP 1 » arrive sur la
   broche DIR du TMC et « DIR 1 » sur STEP (U2 est correct). C'est corrigé dans
   `board_pins.h` (slot 0 : STEP = GPIO4, slot 2 : STEP = GPIO2). À corriger sur la prochaine
   version du PCB, et ne pas « re-corriger » le code.
2. **Les deux PCF8574T ont la même adresse par défaut (0x20)** : **JP1 soudé sur la tête** (fait)
   → 0x21. Sinon le bus I2C ne marche pas. Le test 1 (scan I2C) le confirme.
3. Si le scan trouve 0x38+ au lieu de 0x20+, ce sont des PCF8574**A** : changer
   `PCF_DRIVERS_ADDR` / `PCF_HEAD_ADDR`.
4. **~EN via PCF = pull-up faible (~100 µA)** à l'état haut. Le test 5 bascule ~EN : mesurer
   la broche EN du TMC à l'état « disabled ». Si elle est loin de 3,3 V, le module tire EN vers
   le bas → ajouter une résistance de pull-up.
5. MS1/MS2 sont communs aux 2 drivers d'une carte : slots 0+1 et 2+3 doivent avoir le même
   `microsteps` (le firmware le vérifie au démarrage). Chaque roue est sur une carte
   différente : un actionneur sur U1 doit donc rester en 1/16.
6. Le code 2026 utilisait STEP = GPIO3 / GPIO1 pour les ascenseurs (U1), alors que le PCB
   indique l'inverse (point 1). Si un ascenseur ne bouge pas, inverser STEP/DIR du slot.

## Ordre des tests

`test1_I2C` → `test5_pcf_inputs` → `test6_wheels_debug` (dans `src/main.cpp`) / `test3_stepper`
→ `test2_pca`.
