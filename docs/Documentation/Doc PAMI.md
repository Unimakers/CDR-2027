---
layout: default
title: "Les PAMI"
parent: Documentation
nav_order: 1
---

# Les PAMI

Les Petits Actionneurs Mobiles Indépendants (PAMI), sont des petits robots actifs à partir de la 85ème seconde d'une partie. Ils doivent effectuer diverse actions indépendament du robot principal. La réalisation de ces actions octroit un certain nombre de point et peuvent faire une grande différence à l'issue de la partie.

Bien que généralement moins impressionnant que le robot principal, les PAMI sont une part importante dans les actions d'une partie et représentent un défi de miniaturisation et de respect d'un cahier des charges aussi (voir plus) exigent que le robot principal.

## Définition et Contraintes :

{: .warning}
> Les contraintes présentes dans ce document sont non-négociables !
> Le non-respect d'une de ces contraintes peux mener à l'impossibilité d'homologuer les PAMI
> et donc les empêcherais de jouer en partie réelle !

Cette section est un résumé des contraintes PAMI du Règlement général de la CDR qu vous pouvez visionner et télécharger en cliquant sur <a href="https://www.eurobot.org/wp-content/uploads/2026/09/Eurobot_General_Rules_SR_FR.pdf" download>ce lien</a> (à partir de la page 16 pour la section PAMI)

| Type de Contrainte | Point concernée | Contrainte |
|--------------------|-----------------|------------|
| Physique | Hauteur | 150mm maximum |
| Physique | Périmètre (non-déployé) | 400mm < Périmètre <= 600 mm |
| Physique | Périmètre (déployé) | 400mm < Périmètre <= 700mm |
| Physique | Poids | < 1.5kg |
| Homologation | Chassis | Le chassis des PAMI doivent posséder un espace de 30mmX30mm afin d'accueillir l'autocollant d'homologation |
| Jeu | Départ | Doivent tenir entièrement dans la zone de départ |
| Jeu | Placement | Les PAMI ne doivent pas être empilé sur la zone de départ (tous au sol) |
| Déplacement | Hauteur max | < 350mm d'altitude |
| Stratégie | Timer | Les PAMI ne doivent pas s'activer avant la 85ème seconde du match |

{: .info}
> Le nombre de PAMI n'est pas limité (du moment qu'ils tiennent dans la/les zone(s) de départ).
> Cependant, d'après la taille des zones de cette année, il devrait pouvoir en rentrer six au maximum et quatre de manière raisonable.

## Thème et Actions :

L'édition 2027 de la Coupe De Robotique (CDR) est centré sur le thème de **la légende de Camelot**, les PAMI seront donc les preux chevaliers chargé de **défendre** le chateau du roi Arthur et d'**attaquer** le chateaux ennemi.

Cette année, les PAMI sont à l'honneur avec un total de **3** actions possibles à réaliser :

| Action | Principe | Points octroyés |
|--------|----------|-----------------|
| Assiéger | Les PAMI devront se rendre dans les douves adverses (voir tapis de jeu en annexe) | X pts / douve prise |
| Duel | Les PAMI devront "attaquer" un PAMI adverse à l'aide d'un actionneur fait spécifiquement pour cette action | Y pts / PAMI attaqué |
| Catapultage | Les PAMI devront lancer un boulet (voir annexe) dans la cour du chateau adverse (1 boulet par PAMI) | Z pts / boulet dans la cour |

{: .warning }
> Tout les PAMI ne sont pas obligés de faire les trois actions.
> Mieux vaut un PAMI monotâche fiable qu'un PAMI multitâche disfonctionnel !

## Matériel à disposition :

Les outils du Makerspace offrent un large panel de solutions différentes pour la création d'un PAMI. La création du chassis, ainsi que les techniques et stratégies mises en place pour la conception des PAMI seront relativement libre. Laissez votre créativité s'exprimer !

Néanmoins, nous avons établis une liste de matériel standard afin de garantir une uniformité technique dans les solutions proposées. Le but :
* Augmenter la réparabilité des PAMI en ayant un seul type de composant pour tout les PAMI
* Permettre à tout le monde de comprendre le fonctionnement général d'un PAMI

En effet, bien que vous créerez des PAMI différents, vous devrez être à même de comprendre le fonctionnement et l'utilisation des autres PAMI, que ce soit en terme de composant mais aussi en terme de montage démontage.

{: .warning}
> L'uniformité des composant passe aussi par la **visserie** !  
> Avant de commencer les prototypages, vérifiez quels vis et outils vous aurez besoin, et si vous avez un quantité suffisante (cela inclut
> un surplus en cas de pannes/casse). De manière générale, préférez le même type de tête pour n'utiliser qu'un seul tournevis.

### Les kits :

Les kits contiennent les composants dont vous aurez besoin pour la conception des PAMI.

Contenu des Kits PAMI :

| Composant | Quantité | Remarque |
|-----------|----------|----------|
| Step-motors| 2       |          |
| Servomoteur| 1       | Changement pour sevomoteurs pour des plus gros<br>mieux adaptés aux nouvelles contraintes de taille |
| Bouton d'arrêt d'urgence (BAU) | 1 | |
|Télémètre à ultrason | 1 | Ont déjà les PIN intégrés |
|"Tirette" aimantée | 1 | Vérifier leur état de fonctionnement (certaines sont abimées)|
| Support pour "Tirette" | 1 | À remodéliser selon les besoins |
| Carte électronique de PAMI 2026 | 1 | Pré-assemblée |
| Module ESP-32 | 1 | Fixable directement sur la carte PAMI 2026 |
| Modules TMC-2209 | 2 | Fixables directement sur la carte PAMI 2026 |

![Composant kits I1](<../assets/images/img_documentation/composants kit PAMI legende.png>)

Vous remarquerez que les cartes électroniques fournis sont celles de l'année précédente. En effet, les contraintes mécanique des PAMI ont peu/pas changé et les anciennes cartes sont toujours adaptées aux demandes de cette année. Cela vous permettra de vous concentrer sur l'aspect prototypage et conception mécanique.

### Présentation d'un PAMI monté (version 2026) :

{: .warning}
> L'image est a titre d'**illustation** et n'est pas un exemple à suivre. Les contraintes physiques ont changés entre temps et vous devrez répondre à de nouveaux besoins. Le but est de montrer la composition **minimale** d'un PAMI.

![PAMI 2026 légendé](<../assets/images/img_documentation/PAMI 2026 legende.png>)

### La carte électronique :

La carte électronique est avant tout un système de connexions (routes) qui relient les composants les uns aux autres. Celles que vous avez dans les kits sont déjà assemblé avec les Pins. Les photos ci-dessous sont légendé afin de montrer les branchements des composants.

![Carte PAMI légendée (Verso)](<../assets/images/img_documentation/Carte PAM legende-verso.png>)
![Carte PAMI légendée (Recto)](<../assets/images/img_documentation/Carte PAM legende-recto.png>)

<!--
{: .a_modifier}
> Expliquer chaque composant (ESP-32, TMC...). 
> Expliquer simplement le fonctionnement globale de la carte.
-->
### Les moteurs :
<!--
{: .a_modifier}
> Bien faire la différence entre servo et step moteurs.  
> Expliquer le fonctionement des Pins 
> S'appuyer sur les documentations du Makerspace.
-->
[Apprendre à se programmer un servomoteur](https://doc.makerspace-amiens.fr/docs/tutorials/electronics/servomotor/)

