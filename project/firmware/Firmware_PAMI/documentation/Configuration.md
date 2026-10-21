# Configuration

Toute la configuration matérielle du robot est centralisée dans **`src/Config.h`**.
C'est le seul fichier à modifier pour adapter le firmware à votre câblage.

> Les valeurs marquées `// TODO` doivent impérativement être renseignées avant utilisation.

---

## Broches moteurs

```cpp
#define LEFT_STEP_PIN     // TODO : GPIO branché sur STEP du TMC2209 gauche
#define LEFT_DIR_PIN      // TODO : GPIO branché sur DIR du TMC2209 gauche
#define RIGHT_STEP_PIN    // TODO
#define RIGHT_DIR_PIN    // TODO
```

Si un moteur tourne dans le mauvais sens, passer son flag à `true` :

```cpp
#define LEFT_MOTOR_INVERTED   false
#define RIGHT_MOTOR_INVERTED  false
```

---

## Géométrie du robot

```cpp
#define WHEEL_DIAMETER_MM    60.0f   // Diamètre extérieur des roues en mm
#define WHEEL_BASE_MM        200.0f  // Distance entre les deux points de contact
#define MOTOR_STEPS_PER_REV  200     // Pas complets par tour moteur (moteur 1.8°)
#define MICROSTEPS           16      // Diviseur de microstepping du TMC2209
#define GEAR_RATIO           1.0f    // 1.0 = pas de réducteur
```

Ces valeurs permettent au firmware de convertir des distances en mm en nombre de pas moteur. Elles doivent correspondre à la mécanique réelle du robot.

**Vérification rapide :** demander au robot de faire `robot.go(500)` et mesurer la distance parcourue. Si elle ne correspond pas, ajuster `WHEEL_DIAMETER_MM`.

### Microstepping TMC2209

La valeur de `MICROSTEPS` doit correspondre aux niveaux MS1/MS2 envoyés au driver :

| MS1 | MS2 | Microstepping |
|-----|-----|---------------|
| 0   | 0   | 1/8 |
| 1   | 0   | 1/2 |
| 0   | 1   | 1/4 |
| 1   | 1   | **1/16** ← valeur par défaut |

```cpp
#define TMC2209_MS1_VALUE  true   // HIGH pour 1/16
#define TMC2209_MS2_VALUE  true   // HIGH pour 1/16
```
---

```cpp

// PIN des servomoteurs
#define SERVO1 4
#define SERVO2 5


// PIN activation élec
#define ENABLE_PIN 8

// PIN tirette
#define TIRETTE_PIN 10

#define SWITCH_PIN 9

// PIN du télémètre à ultrason
#define TRIGER_PIN 7
#define ECHO_PIN 6
#define S_ITERATIONS 5

// intervalles
#define SONAR_PING_INTERVAL 33
```

---

## Vitesses et accélérations

```cpp
// Déplacements droits
#define DEFAULT_SPEED_MM_S       300.0f   // Vitesse normale en mm/s
#define DEFAULT_ACCEL_MM_S2      500.0f   // Accélération en mm/s²

// Rotations
#define DEFAULT_TURN_SPEED_DEG_S   90.0f  // Vitesse de rotation en °/s
#define DEFAULT_TURN_ACCEL_DEG_S2 180.0f  // Accélération en °/s²

// Séquence datum (contact avec la bordure)
#define DATUM_SPEED_MM_S   80.0f
#define DATUM_ACCEL_MM_S2  100.0f
```

Augmenter la vitesse si le robot est trop lent, la diminuer si les moteurs perdent des pas (bruits de claquement, trajectoire imprécise).
---

## Boutons et tirette

```cpp
#define COLOR_SWITCH_PIN  -1  // TODO : sélecteur de couleur
#define INIT_BUTTON_PIN   -1  // TODO : bouton d'initialisation
#define TIRETTE_PIN       -1  // TODO : contact de départ

#define BUTTON_ACTIVE_LOW  true   // LOW = bouton enfoncé (avec pull-up)

#define COLOR_SWITCH_YELLOW_STATE  LOW   // État du GPIO pour l'équipe Jaune
#define COLOR_SWITCH_BLUE_STATE    HIGH  // État du GPIO pour l'équipe Bleue

#define TIRETTE_INSERTED_STATE  LOW   // Tirette insérée = contact fermé = LOW
#define TIRETTE_REMOVED_STATE   HIGH  // Tirette retirée = contact ouvert = HIGH
```

Tous les boutons utilisent `INPUT_PULLUP`. Adapter `COLOR_SWITCH_YELLOW_STATE` selon le câblage réel.

---

## Autres paramètres

```cpp
#define MATCH_DURATION_MS  100000UL  // Durée du match : 100 secondes

#define DEBUG_SERIAL  true  // false = désactive tous les logs Serial
```
