/*
  Scène d'éclairage d'une voiture avec différentes configurations. L'éclairage peut être constitué de :
    * Des feux avant et arrière (obligatoire). Les feux avant peuvent être éclairés en feu de croisement ou de route. Les feux arrière peuvent être éclairés en feux de position ou feux de stop.
    * De clignotants gauche et droite (facultatif - fréquence : 90 par minute). Obligatoires à partir du 1er janvier 1969
    * Les clignotants peuvent être utilisés comme feux de détresse. Obligatoires à partir du 1er octobre 1980.
    * Plusieurs configurations d'éclairages sont possibles. On choisit la configuration d'éclairage utilisé avec une combinaison de connexion, ou non, de plusieurs entrées de l'Arduino.
    * Un potentiomètre permet de faire varier la durée des différentes étapes de l'enchaînement d'éclairage.

  Configurations d'éclairage :
    1/ Lumières de nuit fixes.
    2/ Lumières de nuit avec freinage.
    3/ Lumières de nuit avec freinage et appel de phares.
    4/ Stationnement à droite la nuit (lumières de nuit, clignotant à droite, freinage, arrêt tout éteint, lumières de nuit, clignotant à gauche, lumières de nuit).
    5/ Lumières de nuit et feux de détresse permanents.
    6/ Stationnement à droite la nuit avec feux de détresse quand les lumières sont éteintes.
    7/ Tourner à gauche la nuit (lumières de nuit, clignotant à gauche, freinage, clignotant à gauche, lumières de nuit).
    8/ Stationnement à droite le jour (comme 4, sans les lumières de nuit).

  Connexions :
    * Arduino Nano ou Uno.
    * Feux avant : 3.
    * Feux arrière : 5.
    * Clignotants gauche : 2.
    * Clignotants droite : 4.
    * Configuration : 6, 7 et 8.
    * Potentiomètre pour la durée : A0.

  Fils à brancher pour les différentes configurations :
    1/ Aucun.
    2/ 6.
    3/ 7.
    4/ 6 et 7.
    5/ 8.
    6/ 6 et 8.
    7/ 6 et 7.
    8/ 6, 7 et 8.
*/

// Connexions
const uint8_t feu_avant = 3;
const uint8_t feu_arriere = 5;
const uint8_t clignotants_gauche = 2;
const uint8_t clignotants_droite = 4;
const uint8_t configuration_1 = 6;
const uint8_t configuration_2 = 7;
const uint8_t configuration_3 = 8;
const uint8_t potentiometre_duree = A0;

// Paramètres des feux
const uint8_t frequence_clignotant = 90;
const uint8_t feu_arriere_position = 63;
const uint8_t feu_arriere_stop = 255;
const uint8_t feu_avant_croisement = 127;
const uint8_t feu_avant_route = 255;

// Pour test
const uint8_t diviseur_blanc = 20;
const uint8_t diviseur_pause = 4;

// Heure précédent clignotement
uint32_t heure_clignotant;

// Allume les feux avant et arrière de nuit
void lumieres_nuit () {
  analogWrite (feu_avant, feu_avant_croisement / diviseur_blanc);
  analogWrite (feu_arriere, feu_arriere_position);
}

// Eteint les feux avant et arrière
void lumieres_eteintes () {
  analogWrite (feu_avant, 0);
  analogWrite (feu_arriere, 0);
}

// Calcule la pause en fonction du potentiomètre d'ajustement
uint16_t calculer_pause (uint16_t duree_minimum) {
  return map (analogRead (potentiometre_duree), 0, 1023, duree_minimum / diviseur_pause, duree_minimum * 2 / diviseur_pause);
}

// Pause d'une durée nominale, variable selon le potentiomètre
void pause_variable (uint16_t duree_minimum) {
  delay (calculer_pause (duree_minimum));
}

// Mémorise l'heure de début de clignotement
void debut_clignotant () {
  heure_clignotant = millis ();
}

// Pause entre deux clignotements
void pause_clignotant () {
  delay ((heure_clignotant + 60000 / frequence_clignotant / 2) - millis ());
  debut_clignotant ();
}

// Actionne les feux de détresse allumés ou éteints
void actionne_feux_detresse (uint8_t etat) {
  digitalWrite (clignotants_gauche, etat);
  digitalWrite (clignotants_droite, etat);
  pause_clignotant ();
}

// Répète un feu clignotant pendant une durée ajustable par potentiomètre
void repeter_clignotant (uint8_t pin_clignotant, uint16_t duree_repeter) {
  debut_clignotant ();
  uint32_t fin_repeter = millis () + calculer_pause (duree_repeter);
  do {
    digitalWrite (pin_clignotant, HIGH);
    pause_clignotant ();
    digitalWrite (pin_clignotant, LOW);
    pause_clignotant ();
  }
  while (millis () < fin_repeter);
}

// Répète les feux de détresse pendant une durée ajustable par potentiomètre
void repeter_feux_detresse (uint16_t duree_repeter) {
  debut_clignotant ();
  uint32_t fin_repeter = millis () + calculer_pause (duree_repeter);
  do {
    actionne_feux_detresse (HIGH);
    actionne_feux_detresse (LOW);
  }
  while (millis () < fin_repeter);
}

void setup() {
  Serial.begin (115200);
  Serial.println ("Scère d'éclairage d'une voiture, 8 configurations - 09/08/2024");
  pinMode (feu_avant, OUTPUT);
  pinMode (feu_arriere, OUTPUT);
  pinMode (clignotants_gauche, OUTPUT);
  pinMode (clignotants_droite, OUTPUT);
  pinMode (configuration_1, INPUT_PULLUP);
  pinMode (configuration_2, INPUT_PULLUP);
  pinMode (configuration_3, INPUT_PULLUP);
  pinMode (potentiometre_duree, INPUT);
  analogWrite (feu_avant, 0);
  analogWrite (feu_arriere, 0);
  digitalWrite (clignotants_gauche, LOW);
  digitalWrite (clignotants_droite, LOW);
  debut_clignotant ();
}

void loop() {
  // Calcule la configuration choisie
  uint8_t configuration_choisie = 0;
  if (digitalRead (configuration_1) == HIGH) {
    configuration_choisie |= 1;
  }
  if (digitalRead (configuration_2) == HIGH) {
    configuration_choisie |= 2;
  }
  if (digitalRead (configuration_3) == HIGH) {
    configuration_choisie |= 4;
  }

  // Enchainement selon la configuration choisie
  // (on regroupe les configurations qui ont suffisement de points communs)
  switch (configuration_choisie) {
    case 6:
    case 5:
    // Lumières de nuit avec freinage
      lumieres_nuit ();
      pause_variable (20000);
      analogWrite (feu_arriere, feu_arriere_stop);
      pause_variable (10000);
      if (configuration_choisie == 5) {
    // Lumières de nuit avec freinage et appel de phares
        analogWrite (feu_arriere, feu_arriere_position);
        pause_variable (20000);
        analogWrite (feu_avant, feu_avant_route / diviseur_blanc);
        delay (1000);
        analogWrite (feu_avant, feu_avant_croisement / diviseur_blanc);
        delay (1000);
        analogWrite (feu_avant, feu_avant_route / diviseur_blanc);
        delay (1000);
      }
      break;
    case 4:
    // Stationnement à droite la nuit (lumières de nuit, clignotant à droite, freinage, arrêt tout éteint, lumières de nuit, clignotant à gauche, lumières de nuit)
    case 2:
    // Stationnement à droite la nuit avec feux de détresse quand les lumières sont éteintes
    case 0:
    // Stationnement à droite le jour (comme 4, sans les lumières de nuit)
      if (configuration_choisie == 0) {
        lumieres_eteintes ();
      }
      else {
        lumieres_nuit ();
      }
      pause_variable (20000);
      repeter_clignotant (clignotants_droite, 5000);
      analogWrite (feu_arriere, feu_arriere_stop);
      repeter_clignotant (clignotants_droite, 10000);
      analogWrite (feu_arriere, (configuration_choisie == 0) ? 0 : feu_arriere_position);
      repeter_clignotant (clignotants_droite, 30000);
      lumieres_eteintes ();
      if (configuration_choisie == 2) {
        repeter_feux_detresse (60000);
      }
      else {
        pause_variable (60000);
      }
      if (configuration_choisie != 0) {
        lumieres_nuit ();
      }
      pause_variable (5000);
      repeter_clignotant (clignotants_gauche, 20000);
      break;
    case 3:
    // Lumières de nuit et feux de détresse permanents
      lumieres_nuit ();
      actionne_feux_detresse (HIGH);
      actionne_feux_detresse (LOW);
      break;
    case 1:
    // Tourner à gauche la nuit (lumières de nuit, clignotant à gauche, freinage, toujours clignotant à gauche, lumières de nuit)
      lumieres_nuit ();
      pause_variable (20000);
      repeter_clignotant (clignotants_gauche, 5000);
      analogWrite (feu_arriere, feu_arriere_stop);
      repeter_clignotant (clignotants_gauche, 10000);
      analogWrite (feu_arriere, feu_arriere_position);
      repeter_clignotant (clignotants_gauche, 30000);
      pause_variable (40000);
      break;
    default:
    // Lumières de nuit fixes
      lumieres_nuit ();
      delay (10000);
      break;
  }
}
