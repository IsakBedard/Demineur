 /**
 * @file main.c  
 * @author Isak B�dard
 * @date   28 novembre 2019
 * @brief  Jeu : D�mineur.
  * 
  * Le joueur doit trouver l'emplacement de mines.
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
void initTabVue(void);
void rempliMines(int nb);
void metToucheCombien(void);
char calculToucheCombien(int ligne, int colonne);
void deplace(char* x, char* y);
bool demine(char x, char y);
void enleveTuilesAutour(char x, char y);
bool gagne(int* pMines);
/****************** VARIABLES GLOBALES ****************************************/
 char m_tabVue[NB_LIGNE][NB_COL+1]; //Tableau des caract�res affich�s au LCD
 char m_tabMines[NB_LIGNE][NB_COL+1];//Tableau contenant les mines, les espaces et les chiffres
/******************** PROGRAMME PRINCPAL **************************************/
void main(void)
{
    initialisation();
    lcd_init();
    lcd_putMessage("LAB6 Isak Bedard");
    initTabVue();
    while(1)
    {
        __delay_ms(100);
    }
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

/*
 * @brief Rempli le tableau m_tabVue avec le caract�re sp�cial (d�finie en CGRAM
 *  du LCD) TUILE. Met un '\0' � la fin de chaque ligne pour faciliter affichage
 *  avec lcd_putMessage().
 * @param rien
 * @return rien
 */
void initTabVue(void)
{
    for (char i = 0; i < NB_LIGNE; i++)
    {
        for(char j=0;j<(NB_COL);j++)
        {
            m_tabVue[i][j]=TUILE;
        }
        m_tabVue[i][NB_COL]=0;
    }
}
 
/*
 * @brief Rempli le tableau m_tabMines d'un nombre (nb) de mines au hasard.
 *  Les cases vides contiendront le code ascii d'un espace et les cases avec
 *  mine contiendront le caract�re MINE d�fini en CGRAM.
 * @param int nb, le nombre de mines � mettre dans le tableau 
 * @return rien
 */
void rempliMines(int nb)
{
    
}
/*
 * @brief Rempli le tableau m_tabMines avec le nombre de mines que touche la case.
 * Si une case touche � 3 mines, alors la m�thode place le code ascii de 3 dans
 * le tableau. Si la case ne touche � aucune mine, la m�thode met le code
 * ascii d'un espace.
 * Cette m�thode utilise calculToucheCombien(). 
 * @param rien
 * @return rien
 */
void metToucheCombien(void)
{
    
}
/*
 * @brief Calcul � combien de mines touche la case. Cette m�thode est appel�e par metToucheCombien()
 * @param int ligne, int colonne La position dans le tableau m_tabMines a v�rifier
 * @return char nombre. Le nombre de mines touch�es par la case
 */
char calculToucheCombien(int ligne, int colonne)
{
    
}
/**
 * @brief Si la manette est vers la droite ou la gauche, on d�place le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* x, char* y)
{
    
}
/*
 * @brief D�voile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derri�re les tuiles (m_tabMines).
 * Utilise enleveTuileAutour().
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y)
{
    
}
/*
 * @brief D�voile les cases non min�es autour de la tuile re�ue en param�tre.
 * Cette m�thode est appel�e par demine().
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y)
{
    
}
/*
 * @brief V�rifie si gagn�. On a gagn� quand le nombre de tuiles non d�voil�es
 * est �gal au nombre de mines. On augmente de 1 le nombre de mines si on a 
 * gagn�.
 * @param int* pMines. Le nombre de mine.
 * @return vrai si gagn�, faux sinon
 */
bool gagne(int* pMines)
{
    
}