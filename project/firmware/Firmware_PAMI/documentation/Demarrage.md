# Démarrage

Ce guide vous explique comment installer l'environnement de développement, configurer le projet et lancer le robot pour la première fois.

---

## 1. Prérequis logiciels

### VS Code + PlatformIO

1. Télécharger et installer [Visual Studio Code](https://code.visualstudio.com/)
2. Ouvrir VS Code → onglet **Extensions** (icône puzzle à gauche)
3. Rechercher **PlatformIO IDE** et l'installer
4. Redémarrer VS Code

> PlatformIO gère automatiquement le téléchargement du compilateur, des bibliothèques et du flasheur. Pas besoin d'installer quoi que ce soit d'autre.

---

## 2. Ouvrir le projet

1. Cloner ou télécharger ce dépôt
2. Dans VS Code : **Fichier → Ouvrir le dossier** → sélectionner le dossier du projet
3. PlatformIO détecte automatiquement le fichier `platformio.ini` et installe les dépendances

---

## 3. Branchements matériels

### Alimentation

- Le XIAO ESP32-S3 s'alimente via USB (programmation) ou via sa broche 5V (en autonomie)
- Les TMC2209 ont leur propre alimentation moteur (24V recommandé)
- Le RPLidar s'alimente en 5V (vérifier la tension admissible)

### Connexions à configurer

Toutes les broches sont à définir dans `src/Config.h`. Les valeurs `-1` sont des **placeholders** — le robot ne fonctionnera pas tant qu'elles ne sont pas renseignées.

| Signal | Broche ESP32-S3 | Remarque |
|--------|-----------------|----------|
| Moteur gauche STEP | `LEFT_STEP_PIN` | Impulsion de pas |
| Moteur gauche DIR | `LEFT_DIR_PIN` | Direction |
| Moteur droit STEP | `RIGHT_STEP_PIN` | |
| Moteur droit DIR | `RIGHT_DIR_PIN` | |
| Sélecteur couleur | `COLOR_SWITCH_PIN` | Entrée avec pull-up interne |
| Tirette | `TIRETTE_PIN` | Entrée avec pull-up interne |

---

## 4. Compiler et flasher

Dans VS Code, la barre PlatformIO en bas de l'écran propose :

| Bouton | Action |
|--------|--------|
| ✓ Build | Compile le projet |
| → Upload | Compile et flashe la carte |
| 🔌 Monitor | Ouvre le moniteur série (115200 baud) |

Raccourci clavier : `Ctrl+Alt+U` pour uploader directement.

---

## 5. Vérifier le démarrage

Une fois flashé, ouvrir le moniteur série. Vous devriez voir :

```
[BOOT] Serial initialized
[MOTOR] Microstepping configured
[ROBOT] Robot ready
[BOOT] Boot complete
[COLOR] Selected team: Yellow
[INIT] Waiting for init button...
```

Si un message `ERROR` apparaît, vérifier les branchements et les adresses dans `Config.h`.

---

## 6. Séquence de démarrage d'un match

1. Allumer le robot
2. Sélectionner la couleur d'équipe avec l'interrupteur
3. Appuyer sur le bouton **Init** → le robot exécute `initYellow()` ou `initBlue()`
4. Insérer la tirette
5. Retirer la tirette → **le match démarre immédiatement**
6. Le robot exécute la stratégie pendant 100 secondes puis s'arrête définitivement

---

## Étape suivante

→ [Configurer le robot](Configuration.md)
