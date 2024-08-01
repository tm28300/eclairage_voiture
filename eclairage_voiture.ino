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
    9/ Deux véhicules avec lumières de nuit fixes et gyrophare.
    10/ Deux véhicules avec lumières de nuit fixes, appel de phares du premier, clignotants à droite et feux de stop du second et gyrophare.

  Connexions :
    * Arduino Nano ou Uno.
    * Feux avant premier véhicule : 3.
    * Feux arrière premier véhicule: 5.
    * Clignotants gauche : 2.
    * Clignotants droite : 4.
    * Configuration : 7, 8, 9 et 10.
    * Feux avant second véhicule : 6.
    * Feux arrière second véhicule : 11.
    * Gyrophare second véhicule : 12.

  Fils à brancher pour les différentes configurations :
    1/ Aucun.
    2/ GND, 7.
    3/ GND, 8.
    4/ GND, 7 et 8.
    5/ GND, 9.
    6/ GND, 7 et 9.
    7/ GND, 8 et 9.
    8/ GND, 7, 8 et 9.
    9/ GND, 10.
    10/ GND, 7 et 10.
*/

// Connexions
#ifdef __AVR_ATtiny85__
  #define feu_avant 0 // PWM Timer 0
  #define feu_arriere 1 // PWM Timer 0
  #define clignotants_gauche 2
  #define clignotants_droite 3
  #define configuration_avec_potentiometre
  #define configuration_potentiometre_init 4
  #define configuration_potentiometre_read 2
#else
  #define avec_serial
  #define feu_avant 3 // PWM
  #define feu_arriere 5 // PWM
  #define clignotants_gauche 2
  #define clignotants_droite 4
  #define gyrophare 12
  #define feu_avant_second 6 // PWM
  #define feu_arriere_second 11 // PWM
  #define configuration_1 7
  #define configuration_2 8
  #define configuration_3 9
  #define configuration_4 10
#endif

// Paramètres des feux
#define frequence_clignotant 90
#define feu_arriere_position 63
#define feu_arriere_stop 255
#define feu_avant_croisement 127
#define feu_avant_route 255
#define feux_detresse 255

// Pour test (décommenter pour la mise au point)
//#define diviseur_blanc(valeur_blanc) ((valeur_blanc) / 50)
//#define diviseur_pause(valeur_pause) ((valeur_pause) / 4)
// DEBUG_SERIE permet d'ajouter des affichages sur la sortie série
//#define DEBUG_SERIE
// Mode normal
#ifndef diviseur_blanc
  #define diviseur_blanc(valeur_blanc) ((valeur_blanc))
#endif
#ifndef diviseur_pause
  #define diviseur_pause(valeur_pause) ((valeur_pause))
#endif

volatile bool clignotant_gauche_etat = LOW;
volatile bool clignotant_droite_etat = LOW;
volatile bool clignotant_gauche_active = false;
volatile bool clignotant_droite_active = false;
volatile bool feux_detresse_active = false;
#ifdef gyrophare
volatile bool gyrophare_etat = LOW;
volatile bool gyrophare_active = false;
#endif

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

// Répète un feu clignotant pendant une durée ajustable par potentiomètre
void repeter_clignotant (uint8_t pin_clignotant, uint16_t duree_repeter) {
  volatile bool& flag_actif = (pin_clignotant == clignotants_gauche) ? clignotant_gauche_active :
                              ((pin_clignotant == clignotants_droite) ? clignotant_droite_active :
                              feux_detresse_active);

  flag_actif = true;
  delay(duree_repeter);
  flag_actif = false;
}

void init_timer1 () {
// Configuration du Timer1 pour une interruption suivant la fréquence de clignotement paramétrée
  cli(); // Désactiver les interruptions
#ifdef __AVR_ATtiny85__
  // Configuration du Timer1 pour ATtiny85
  TCCR1 = 0; // Mode de fonctionnement normal
  TCNT1 = 0; // Initialiser le compteur
  OCR1C = (F_CPU / 16384) * 30 / 2 / frequence_clignotant; // Ignore une interruption sur deux
  TCCR1 |= (1 << CTC1) // Mode CTC
           | (1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10); // Prescaler 16384
  TIMSK |= (1 << OCIE1A); // Activer l'interruption de comparaison A
#else
  // Configuration du Timer1 pour ATmega328P
  TCCR1A = 0; // Mode de fonctionnement normal
  TCCR1B = 0;
  TCNT1 = 0; // Initialiser le compteur
  OCR1A = (F_CPU / 256) * 30 / frequence_clignotant;;
  TCCR1B |= (1 << WGM12) // Mode CTC
            | (1 << CS12); // Prescaler 256
  TIMSK1 |= (1 << OCIE1A); // Activer l'interruption de comparaison A
#endif
  sei(); // Activer les interruptions
// ATENTION : L'utilisation du timer 1 pour la gestion du clignotement ne permet plus d'utiliser les bornes 9 et 10 en PWM
// Seules les bornes 3 et 11 peuvent être utilisées en PWM grace au timer 2
}

// Interruption du Timer1 pour gérer le clignotement
ISR(TIMER1_COMPA_vect) {
  #ifdef __AVR_ATtiny85__
  // Pour le AtTiny 85, comme le timer 1 a un compteur ainsi qu'un prescaler trop petits, on ignore une interruption sur deux
    static volatile uint8_t interruption_counter = 0;
    interruption_counter++;
    if (interruption_counter < 2) {
      return; // Ignorer une interruption sur deux
    }
    interruption_counter = 0;
  #endif
  if (clignotant_gauche_active || feux_detresse_active) {
    clignotant_gauche_etat = !clignotant_gauche_etat;
    digitalWrite(clignotants_gauche, clignotant_gauche_etat);
  } else if (clignotant_gauche_etat) {
    clignotant_gauche_etat = LOW;
    digitalWrite(clignotants_gauche, clignotant_gauche_etat);
  }
  if (clignotant_droite_active || feux_detresse_active) {
    clignotant_droite_etat = !clignotant_droite_etat;
    digitalWrite(clignotants_droite, clignotant_droite_etat);
  } else if (clignotant_droite_etat) {
    clignotant_droite_etat = LOW;
    digitalWrite(clignotants_droite, clignotant_droite_etat);
  }
  #ifdef gyrophare
    if (gyrophare_active) {
      gyrophare_etat = !gyrophare_etat;
      digitalWrite(gyrophare, gyrophare_etat);
    } else if (gyrophare_etat) {
      gyrophare_etat = LOW;
      digitalWrite(gyrophare, gyrophare_etat);
    }
  #endif
}

void setup() {
  #ifdef avec_serial
    Serial.begin (115200);
    Serial.println ("Scéne d'éclairage d'une voiture, 10 configurations - 01/08/2024");
  #endif
  pinMode (feu_avant, OUTPUT);
  pinMode (feu_arriere, OUTPUT);
  #ifdef feu_avant_second
    pinMode (feu_avant_second, OUTPUT);
  #endif
  #ifdef feu_arriere_second
    pinMode (feu_arriere_second, OUTPUT);
  #endif
  pinMode (clignotants_gauche, OUTPUT);
  pinMode (clignotants_droite, OUTPUT);
  #ifdef gyrophare
    pinMode (gyrophare, OUTPUT);
  #endif
  #ifdef configuration_avec_potentiometre
    pinMode (configuration_potentiometre_init, INPUT);
  #else
    pinMode (configuration_1, INPUT_PULLUP);
    pinMode (configuration_2, INPUT_PULLUP);
    pinMode (configuration_3, INPUT_PULLUP);
    pinMode (configuration_4, INPUT_PULLUP);
  #endif
  #ifdef avec_potentiometre_duree
    pinMode (potentiometre_duree_init, INPUT);
  #endif
  analogWrite (feu_avant, 0);
  analogWrite (feu_arriere, 0);
  #ifdef feu_avant_second
    analogWrite (feu_avant_second, 0);
  #endif
  #ifdef feu_arriere_second
    analogWrite (feu_arriere_second, 0);
  #endif
  digitalWrite (clignotants_gauche, LOW);
  digitalWrite (clignotants_droite, LOW);
  #ifdef gyrophare
    digitalWrite (gyrophare, LOW);
  #endif
  init_timer1 ();
}

void loop() {
  // Calcule la configuration choisie
  uint8_t configuration_choisie;
  #ifdef configuration_avec_potentiometre
    configuration_choisie = analogRead (configuration_potentiometre_read) / 128;
  #else
    configuration_choisie = 0;
    if (digitalRead (configuration_1) == LOW) {
      configuration_choisie |= 1;
    }
    if (digitalRead (configuration_2) == LOW) {
      configuration_choisie |= 2;
    }
    if (digitalRead (configuration_3) == LOW) {
      configuration_choisie |= 4;
    }
    if (digitalRead (configuration_4) == LOW) {
      configuration_choisie |= 8;
    }
  #endif

  #if defined(DEBUG_SERIE) && defined(avec_serial)
    Serial.print ("Configuration choisie ");
    Serial.println (configuration_choisie);
  #endif

  // Enchainement selon la configuration choisie
  // (on regroupe les configurations qui ont suffisement de points communs)
  switch (configuration_choisie) {
    case 1:
    // Lumières de nuit avec freinage
    case 2:
    // Lumières de nuit avec freinage et appel de phares
      lumieres_nuit ();
      delay (diviseur_pause (20000));
      analogWrite (feu_arriere, feu_arriere_stop);
      delay (diviseur_pause (10000));
      if (configuration_choisie == 2) {
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
    case 3:
    // Stationnement à droite la nuit (lumières de nuit, clignotant à droite, freinage, arrêt tout éteint, lumières de nuit, clignotant à gauche, lumières de nuit)
    case 5:
    // Stationnement à droite la nuit avec feux de détresse quand les lumières sont éteintes
    case 7:
    // Stationnement à droite le jour (comme 4, sans les lumières de nuit)
      if (configuration_choisie == 7) {
        lumieres_eteintes ();
      }
      else {
        lumieres_nuit ();
      }
      delay (diviseur_pause (20000));
      repeter_clignotant (clignotants_droite, diviseur_pause (5000));
      analogWrite (feu_arriere, feu_arriere_stop);
      repeter_clignotant (clignotants_droite, diviseur_pause (10000));
      analogWrite (feu_arriere, (configuration_choisie == 7) ? 0 : feu_arriere_position);
      repeter_clignotant (clignotants_droite, diviseur_pause (30000));
      lumieres_eteintes ();
      if (configuration_choisie == 5) {
      // Répète les feux de détresse pendant une durée
        repeter_clignotant (feux_detresse, diviseur_pause (60000));
      }
      else {
        delay (diviseur_pause (60000));
      }
      if (configuration_choisie != 7) {
        lumieres_nuit ();
      }
      delay (diviseur_pause (5000));
      repeter_clignotant (clignotants_gauche, diviseur_pause (20000));
      break;
    case 6:
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
#if defined(feu_avant_second) && defined(feu_arriere_second) && defined(gyrophare)
    case 8:
    // Deux véhicules avec lumières de nuit fixes et gyrophare.
    case 9:
    // Deux véhicules avec lumières de nuit fixes, appel de phares du premier, clignotants à droite du second et gyrophare.
      lumieres_nuit ();
      analogWrite (feu_avant_second, diviseur_blanc (feu_avant_croisement));
      analogWrite (feu_arriere_second, feu_arriere_position);
      gyrophare_active = true;
      delay (diviseur_pause (30000));
      if (configuration_choisie == 9) {
        // Appels de phares sur second véhicule
        analogWrite (feu_avant_second, diviseur_blanc (feu_avant_route)); // Appel
        delay (diviseur_pause (1000));
        analogWrite (feu_avant_second, diviseur_blanc (feu_avant_croisement)); // Normal
        delay (diviseur_pause (1000));
        analogWrite (feu_avant_second, diviseur_blanc (feu_avant_route)); // Appel
        delay (diviseur_pause (1000));
        analogWrite (feu_avant_second, diviseur_blanc (feu_avant_croisement)); //Normal
        delay (diviseur_pause (5000));
        // La première voiture se stationne
        repeter_clignotant (clignotants_droite, diviseur_pause (5000));
        analogWrite (feu_arriere, feu_arriere_stop);
        repeter_clignotant (clignotants_droite, diviseur_pause (10000));
        analogWrite (feu_arriere, feu_arriere_position);
        repeter_clignotant (clignotants_droite, diviseur_pause (30000));
        delay (diviseur_pause (60000));
      }
      gyrophare_active = false;
      analogWrite (feu_avant_second, 0);
      analogWrite (feu_arriere_second, 0);
      break;
#endif
    case 4:
    // Lumières de nuit et feux de détresse permanents
    default: // 0
    // Lumières de nuit fixes
      lumieres_nuit ();
      if (configuration_choisie == 4) {
        repeter_clignotant (feux_detresse, diviseur_pause (60000));
      }
      else
      {
        delay (diviseur_pause (10000));
      }
      break;
  }
}
