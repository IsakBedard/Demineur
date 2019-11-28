 /**
 * @file main.c  
 * @author Isak B�dard
 * @date   28 novembre 2019
 * @brief  Jeu : D�mineur.
  * 
  * 
 *
 * @version 1.0
 * Environnement:
 *     D�veloppement: MPLAB X IDE (version 5.25)
 *     Compilateur: XC8 (version 2.10)
 *     Mat�riel: Carte d�mo du Pickit3, PIC 18F45K20, �cran LCD 4x20, power pack
  * 5V/3V3, joystick XY avec bouton poussoir
  */
/****************** Liste des INCLUDES ****************************************/
#include <xc.h>
#include <stdbool.h>  // pour l'utilisation du type bool
#include <conio.h>
#include "Lcd4Lignes.h" // pour utiliser le fichier header lcd4Lignes.h
#include "serie.h" //pour utiliser le fichier header serie.h
#include <stdlib.h>
/********************** CONSTANTES *******************************************/
#define _XTAL_FREQ 1000000 //Constante utilis�e par __delay_ms(x). Doit = fr�q interne du uC
#define NB_LIGNE 4  //afficheur LCD 4x20
#define NB_COL 20
#define AXE_X 7  //canal analogique de l'axe x de la manette
#define AXE_Y 6
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define TUILE 1 //caract�re cgram d'une tuile
#define MINE 2 //caract�re cgram d'une mine
/********************** PROTOTYPES *******************************************/
void initialisation(void);

/****************** VARIABLES GLOBALES ****************************************/
 char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau des caract�res affich�s au LCD
 char m_tabMines[NB_LIGNE][NB_COL+1];//Tableau contenant les mines, les espaces et les chiffres
/******************** PROGRAMME PRINCPAL **************************************/
void main(void)
{
    initialisation();
}

/**
 * @brief Fait l'initialisation des diff�rents registres et variables.
 * @param Aucun
 * @return Aucun
 */
void initialisation(void)
{
    TRISD = 0; //Tout le port D en sortie
    ANSELH = 0;  // RB0 � RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
    ANSEL = 0;  // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie
   
    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
   //Configuration du port analogique
    ANSELbits.ANS7 = 1;  //A7 en mode analogique
    ADCON0bits.ADON = 1; //Convertisseur AN � on
	ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
    ADCON2bits.ADFM = 0; //Alignement � gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB � gauche dans ADRESL)
    ADCON2bits.ACQT = 0;//7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0;//6; //Fosc/64 (Fr�quence pour la conversion la plus longue possible)
}