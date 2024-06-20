/*
  Scène d'éclairage d'une voiture avec différentes configurations. L'éclairage peut être constitué de :
    * Des feux avant et arrière (obligatoire). Les feux avant peuvent être éclairés en feu de croisement ou de route. Les feux arrière peuvent être éclairés en feux de position ou feux de stop.
    * De clignotants gauche et droite (facultatif - fréquence : 90 par minute). Obligatoires à partir du 1er janvier 1969
    * Les clignotants peuvent être utilisés comme feux de détresse. Obligatoires à partir du 1er octobre 1980.
    * Plusieurs configurations d'éclairages sont possibles. On choisit la configuration d'éclairage utilisé avec une combinaison de connexion, ou non, de plusieurs entrées de l'Arduino.

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
    2/ GND, 6.
    3/ GND, 7.
    4/ GND, 6 et 7.
    5/ GND, 8.
    6/ GND, 6 et 8.
    7/ GND, 6 et 7.
    8/ GND, 6, 7 et 8.
*/

// Connexions
#ifdef __AVR_ATtiny85__
  #define feu_avant 0
  #define feu_arriere 1
  #define clignotants_gauche 2
  #define clignotants_droite 3
  #define configuration_avec_potentiometre
  #define configuration_potentiometre_init 4
  #define configuration_potentiometre_read 2
#else
  #define avec_serial
  #define feu_avant 3
  #define feu_arriere 5
  #define clignotants_gauche 2
  #define clignotants_droite 4
  #define configuration_1 6
  #define configuration_2 7
  #define configuration_3 8
#endif

// Paramètres des feux
#define frequence_clignotant 90
#define feu_arriere_position 63
#define feu_arriere_stop 255
#define feu_avant_croisement 127
#define feu_avant_route 255

// Pour test
//#define diviseur_blanc(valeur_blanc) ((valeur_blanc) / 50)
//#define diviseur_pause(valeur_pause) ((valeur_pause) / 4)
// Mode normal
#define diviseur_blanc(valeur_blanc) ((valeur_blanc))
#define diviseur_pause(valeur_pause) ((valeur_pause))

// Allume les feux avant et arrière de nuit
void lumieres_nuit () {
  analogWrite (feu_avant, diviseur_blanc (feu_avant_croisement));
  analogWrite (feu_arriere, feu_arriere_position);
}

// Eteint les feux avant et arrière
void lumieres_eteintes () {
  analogWrite (feu_avant, 0);
  analogWrite (feu_arriere, 0);
}

// Pause entre deux clignotements
void pause_clignotant () {
  delay (60000 / frequence_clignotant / 2);
}

// Actionne les feux de détresse allumés ou éteints
void actionne_feux_detresse (uint8_t etat) {
  digitalWrite (clignotants_gauche, etat);
  digitalWrite (clignotants_droite, etat);
  pause_clignotant ();
}

// Répète un feu clignotant pendant une durée ajustable par potentiomètre
void repeter_clignotant (uint8_t pin_clignotant, uint16_t duree_repeter) {
  uint16_t nombre_boucle = duree_repeter / (60000 / frequence_clignotant);
  uint16_t compteur_boucle;
  for (compteur_boucle = 0 ; compteur_boucle < nombre_boucle ; ++compteur_boucle) {
    digitalWrite (pin_clignotant, HIGH);
    pause_clignotant ();
    digitalWrite (pin_clignotant, LOW);
    pause_clignotant ();
  }
}

void setup() {
  #ifdef avec_serial
    Serial.begin (115200);
    Serial.println ("Scéne d'éclairage d'une voiture, 8 configurations - 20/05/2024");
  #endif
  pinMode (feu_avant, OUTPUT);
  pinMode (feu_arriere, OUTPUT);
  pinMode (clignotants_gauche, OUTPUT);
  pinMode (clignotants_droite, OUTPUT);
  #ifdef configuration_avec_potentiometre
    pinMode (configuration_potentiometre_init, INPUT);
  #else
    pinMode (configuration_1, INPUT_PULLUP);
    pinMode (configuration_2, INPUT_PULLUP);
    pinMode (configuration_3, INPUT_PULLUP);
  #endif
  #ifdef avec_potentiometre_duree
    pinMode (potentiometre_duree_init, INPUT);
  #endif
  analogWrite (feu_avant, 0);
  analogWrite (feu_arriere, 0);
  digitalWrite (clignotants_gauche, LOW);
  digitalWrite (clignotants_droite, LOW);
}

void loop() {
  // Calcule la configuration choisie
  uint8_t configuration_choisie;
  #ifdef configuration_avec_potentiometre
    configuration_choisie = analogRead (configuration_potentiometre_read) / 128;
  #else
    configuration_choisie = 0;
    if (digitalRead (configuration_1) == HIGH) {
      configuration_choisie |= 1;
    }
    if (digitalRead (configuration_2) == HIGH) {
      configuration_choisie |= 2;
    }
    if (digitalRead (configuration_3) == HIGH) {
      configuration_choisie |= 4;
    }
  #endif

  // Enchainement selon la configuration choisie
  // (on regroupe les configurations qui ont suffisement de points communs)
  switch (configuration_choisie) {
    case 6:
    case 5:
    // Lumières de nuit avec freinage
      lumieres_nuit ();
      delay (diviseur_pause (20000));
      analogWrite (feu_arriere, feu_arriere_stop);
      delay (diviseur_pause (10000));
      if (configuration_choisie == 5) {
    // Lumières de nuit avec freinage et appel de phares
        analogWrite (feu_arriere, feu_arriere_position);
        delay (diviseur_pause (20000));
        analogWrite (feu_avant, diviseur_blanc (feu_avant_route));
        delay (diviseur_pause (1000));
        analogWrite (feu_avant, diviseur_blanc (feu_avant_croisement));
        delay (diviseur_pause (1000));
        analogWrite (feu_avant, diviseur_blanc (feu_avant_route));
        delay (diviseur_pause (1000));
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
      delay (diviseur_pause (20000));
      repeter_clignotant (clignotants_droite, diviseur_pause (5000));
      analogWrite (feu_arriere, feu_arriere_stop);
      repeter_clignotant (clignotants_droite, diviseur_pause (10000));
      analogWrite (feu_arriere, (configuration_choisie == 0) ? 0 : feu_arriere_position);
      repeter_clignotant (clignotants_droite, diviseur_pause (30000));
      lumieres_eteintes ();
      if (configuration_choisie == 2) {
      // Répète les feux de détresse pendant une durée
        uint16_t nombre_boucle = diviseur_pause (60000) / (60000 / frequence_clignotant);
        uint16_t compteur_boucle;
        for (compteur_boucle = 0 ; compteur_boucle < nombre_boucle ; ++compteur_boucle) {
          actionne_feux_detresse (HIGH);
          actionne_feux_detresse (LOW);
        }
      }
      else {
        delay (diviseur_pause (60000));
      }
      if (configuration_choisie != 0) {
        lumieres_nuit ();
      }
      delay (diviseur_pause (5000));
      repeter_clignotant (clignotants_gauche, diviseur_pause (20000));
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
      delay (diviseur_pause (20000));
      repeter_clignotant (clignotants_gauche, diviseur_pause (5000));
      analogWrite (feu_arriere, feu_arriere_stop);
      repeter_clignotant (clignotants_gauche, diviseur_pause (10000));
      analogWrite (feu_arriere, feu_arriere_position);
      repeter_clignotant (clignotants_gauche, diviseur_pause (30000));
      delay (diviseur_pause (40000));
      break;
    default:
    // Lumières de nuit fixes
      lumieres_nuit ();
      delay (diviseur_pause (10000));
      break;
  }
}
