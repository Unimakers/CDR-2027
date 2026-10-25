---
layout: default
title: "Carte PAMI"
parent: "Électronique"
grand_parent: "Conception"
nav_order: 2
---

# Carte PAMI

## Rôle

Cette carte permet de contrôler un PAMI dans son ensemble. Une seule carte permet de piloter un PAMI avec 2 roue et deux actionneurs (servomoteurs).

## Schéma

{% include kicad.html src="../../assets/kicad/PAMI/carte pamis.kicad_sch" controles="full" %}

## PCB

{% include kicad.html src="../../assets/kicad/PAMI/carte pamis.kicad_pcb" controles="full" %}

## Nomenclature

| Référence | Composant | Valeur / réf. exacte | Quantité | Source |
|---|---|---|---|---|
| U1 | Microcontrôleur | ESP32-S3-DevKitC-1 | 1 | … |
| U2-U3 | Controleur steppers | TMC2209 | 2 | … |
