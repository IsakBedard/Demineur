/**
 * @file main.c  
 * @author Isak B�dard
 * @date   28 novembre 2019
 * @brief  Jeu : D�mineur.
 * 
 * L'�cran LCD 4x20 devient un champ de mines. Les mines, plac�es al�atoirement,
 * sont cach�es par des tuiles. Il est possible de d�couvrir ce qui a en dessous des 
 * tuiles en s'y d�pla�ant avec le joystick XY et en appuyant sur le bouton inclut
 * avec le joystick. 
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
#define AXE_Y 6 //canal analogique de l'axe y de la manette
#define PORT_SW PORTBbits.RB1 //sw de la manette
#define SW0 PORTBbits.RB0 //bouton sur la carte noire
#define TUILE 1 //caract�re cgram d'une tuile
#define MINE 2 //caract�re cgram d'une mine
#define DRAPEAU 3 //caract�re cgram d'un drapeau
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
char getAnalog(char canal);
void afficheTabVue(void);
void afficheTabMines(void);
void metOuEnleveDrapeau(char x, char y);
/****************** VARIABLES GLOBALES ****************************************/
char m_tabVue[NB_LIGNE][NB_COL + 1]; //Tableau des caract�res affich�s au LCD
char m_tabMines[NB_LIGNE][NB_COL + 1]; //Tableau contenant les mines, les espaces et les chiffres
/******************** PROGRAMME PRINCPAL **************************************/
void main(void) 
{
    char* posX = 10; //pointeur pour la position en X du curseur sur le LCD. Initialis� environ au centre.
    char* posY = 2; //pointeur pour la position en Y du curseur sur le LCD. Initialis� environ au centre.
    int* nbMine = 9; //nombre de mines dans le champ de mines. Augmente de 1 lorsqu'on gagne

    initialisation(); //initialisations diverses
    lcd_init(); //permet la fonctionnalit� du LCD
    initTabVue(); //initialise la matrice m_tabVue avec des tuiles (caract�re cgram)
    rempliMines(nbMine); //initialise partiellement m_tabMines pour placer les mines
    metToucheCombien(); //place les nombres dans m_tabMines selon l'emplacement des mines
    afficheTabVue(); //affiche la matrice m_tabVue
    
    
    while (1) //boucle infinie
    {  
        deplace(&posX, &posY); //on d�place le curseur
        if(PORT_SW==0)//si le bouton du joystick est enfonc�
            if(demine(posX,posY)==false || gagne(&nbMine)) //si on a gagn� ou perdu (trouv� toutes les mines ou touch� une mine)
            {
                afficheTabMines();
                while(PORT_SW==1); //on affiche m_tabMines jusqu'� ce que le bouton du joystick soit r�enfonc�
                initTabVue(); 
                rempliMines(nbMine);
                metToucheCombien();
                afficheTabVue(); //on r�initialise les deux matrices.
            }
        if(SW0==0) //si le bouton sur la carte noire est enfonc�
            metOuEnleveDrapeau(posX,posY); //appel de la fonction qui g�re les drapeaux
        __delay_ms(100); //d�lai de la boucle principale. d�termine la vitesse de d�placement
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
    ANSELH = 0; // RB0 � RB4 en mode digital. Sur 18F45K20 AN et PortB sont sur les memes broches
    TRISB = 0xFF; //tout le port B en entree
    ANSEL = 0; // PORTA en mode digital. Sur 18F45K20 AN et PortA sont sur les memes broches
    TRISA = 0; //tout le port A en sortie

    //Pour du vrai hasard, on doit rajouter ces lignes. 
    //Ne fonctionne pas en mode simulateur.
    T1CONbits.TMR1ON = 1;
    srand(TMR1);
    //Configuration du port analogique
    ANSELbits.ANS7 = 1; //A7 en mode analogique
    ADCON0bits.ADON = 1; //Convertisseur AN � on
    ADCON1 = 0; //Vref+ = VDD et Vref- = VSS
    ADCON2bits.ADFM = 0; //Alignement � gauche des 10bits de la conversion (8 MSB dans ADRESH, 2 LSB � gauche dans ADRESL)
    ADCON2bits.ACQT = 0; //7; //20 TAD (on laisse le max de temps au Chold du convertisseur AN pour se charger)
    ADCON2bits.ADCS = 0; //6; //Fosc/64 (Fr�quence pour la conversion la plus longue possible)
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
    for (char i = 0; i < NB_LIGNE; i++) {
        for (char j = 0; j < NB_COL; j++) { 
            m_tabVue[i][j] = TUILE; //on met des tuiles
        }
        m_tabVue[i][NB_COL] = 0; //le dernier caract�re de la ligne est 0 ou '\0'
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
    char x, y; //les caract�res pour la position en XY de la mine

    for (char i = 0; i < NB_LIGNE; i++) {
        for (char j = 0; j < NB_COL; j++) {
            m_tabMines[i][j] = ' '; //on met tout le tableau vide (avec des espaces)
        }
    }
    while (nb > 0) { //tant que le nombre de mines voulu n'a pas �t� atteint
        x = rand() % 20;
        y = rand() % 4; //on assigne des valeurs XY al�atoires
        if (m_tabMines[y][x] != MINE) { //si la position al�atoire est diponible
            m_tabMines[y][x] = MINE; //on place une mine
            nb--; //il reste une mine de moins � placer
        }
    }
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
    for (char i = 0; i < NB_LIGNE; i++) {
        for (char j = 0; j < NB_COL; j++) {
            if (m_tabMines[i][j] != MINE)
                m_tabMines[i][j] = calculToucheCombien(i, j) + 48; //on met le caract�re ASCII du nombre de mines autour de la case dans la case
            if (m_tabMines[i][j] == '0') //s'il y a 0 mines autour (afficherait 0)
                m_tabMines[i][j] = ' '; //on remplace par un espace pour �tre plus fid�le au jeu original
        }
    }
}

/*
 * @brief Calcul � combien de mines touche la case. Cette m�thode est appel�e par metToucheCombien()
 * @param int ligne, int colonne La position dans le tableau m_tabMines a v�rifier
 * @return char nombre. Le nombre de mines touch�es par la case
 */
char calculToucheCombien(int ligne, int colonne) 
{
    int i = ligne - 1;
    int j = colonne - 1;//variables pour faire un 3x3 autour de la case voulue
    char nbMines = 0; //valeur de retour

    if (i < 0)
        i = 0;
    if (j < 0)
        j = 0;//si on d�passe le LCD, on remet � la limite

    for (i = i; (i <= (ligne + 1))&&(i < NB_LIGNE); i++) {//
        for (j = j = colonne - 1; (j <= (colonne + 1))&&(j < NB_COL); j++) { //on fait le 3x3 autour de la case voulue.
                                                                     //on ne le fait pas si on d�passe du LCD
            if (m_tabMines[i][j] == MINE)//Si c'est une mine                               
                nbMines++;//incr�mentation du nombre de mines
        }
    }
    return nbMines; //on retourne le nombre de mines autour
}

/**
 * @brief Si la manette est vers la droite ou la gauche, on d�place le curseur 
 * d'une position (gauche, droite, bas et haut)
 * @param char* x, char* y Les positions X et y  sur l'afficheur
 * @return rien
 */
void deplace(char* x, char* y) 
{
    unsigned char analogX = getAnalog(AXE_X);//valeur entre 0 et 255 qui repr�sente la position X du joystick
    unsigned char analogY = getAnalog(AXE_Y);//valeur entre 0 et 255 qui repr�sente la position Y du joystick

    if (0 <= analogX && analogX <= 80) //si le joystick est vers la gauche
    {
        *x = (*x) - 1; //d�cale la position de 1 vers la gauche
        if ((*x) <= 0) //si on d�passe de l'�cran
            *x = 20; //on revient de l'autre c�t�
    } else if (175 <= analogX && analogX <= 255) //si le joystick est vers la droite
    {
        *x = (*x) + 1; //d�cale la position de 1 vers la droite
        if ((*x) >= 21) //si on d�passe de l'�cran
            *x = 1; //on revient de l'autre c�t�
    }

    if (0 <= analogY && analogY <= 80) //si le joystick est vers le haut
    {
        *y = (*y) - 1; //d�cale la position de 1 vers le haut
        if ((*y) <= 0) //si on d�passe de l'�cran
            *y = 4; //on revient de l'autre c�t�
    } else if (175 <= analogY && analogY <= 255) //si le joystick est vers le bas
    {
        *y = (*y) + 1; //d�cale la position de 1 vers le bas
        if ((*y) >= 5) //si on d�passe de l'�cran
            *y = 1; //on revient de l'autre c�t�
    }
    lcd_gotoXY(*x, *y); //on met le curseur � la nouvelle position.
}

/*
 * @brief D�voile une tuile (case) de m_tabVue. 
 * S'il y a une mine, retourne Faux. Sinon remplace la case et les cases autour
 * par ce qu'il y a derri�re les tuiles (m_tabMines).
 * Utilise enleveTuileAutour(). Ne d�voile rien si la case s�lectionn�e est un drapeau.
 * @param char x, char y Les positions X et y sur l'afficheur LCD
 * @return faux s'il y avait une mine, vrai sinon
 */
bool demine(char x, char y) 
{
    while(PORT_SW==true); //pour laisser le temps de voir si on a gagn� ou perdu
    if (m_tabMines[y - 1][x - 1] == MINE)//si la case s�lectionn�e est une mine
        return false;//retourne faux (on a perdu)
    else 
    {
        if (m_tabMines[y-1][x-1]==' ')//si c'est un espace (donc pas une mine ou pas un chiffre)
            enleveTuilesAutour(x, y);//on enl�ve les tuiles autour
        else if (m_tabVue[y-1][x-1]!=DRAPEAU)//si ce n'est pas un drapeau (donc un chiffre)
        {
            m_tabVue[y-1][x-1]=m_tabMines[y-1][x-1];//on actualise seulement la case s�lectionn�e, pas celles autour
            afficheTabVue();//actualise le LCD pour afficher la nouvelle matrice
        }
        return true;//retourne vrai (on a pas perdu)
    }
}

/*
 * @brief D�voile les cases non min�es autour de la tuile re�ue en param�tre.
 * Cette m�thode est appel�e par demine(). Ne devoile pas non plus les cases avec
 * drapeaux.
 * @param char x, char y Les positions X et y sur l'afficheur LCD.
 * @return rien
 */
void enleveTuilesAutour(char x, char y) 
{
    signed char i = x - 2;
    signed char j = y - 2;//variables pour faire un 3x3 autour de la case s�lectionn�e
    char mem; //variable m�moire pour r�initialisation ult�rieure

    if (i < 0)
        i = 0;
    if (j < 0)
        j = 0;//si on d�passe du LCD, on revient � la limite du LCD
    mem=i;//m�moire de la variable i
    
    
    while(j <= y && j<NB_LIGNE)
    {
        i=mem;//on remet i � sa valeur initiale
        while(i<=x && i<NB_COL) //on fait le 3x3 autour de la case voulue
        { 
            if(m_tabMines[j][i]!=MINE && m_tabVue[j][i]!=DRAPEAU) //si la case v�rifi�e dans le 3x3 n'est ni une mine, ni un drapeau
                m_tabVue[j][i]=m_tabMines[j][i]; //on la d�voile
            i++;
        }
        j++;
    }
    afficheTabVue(); //on actualise le LCD avec la nouvelle matrice
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
    char nbTuileEtDrapeau=0; //valeur de comparaison. doit �tre �gale � la somme du nombre de drapeaux et du nombre de tuiles dans m_tabVue
    
    for (char i = 0; i < NB_LIGNE; i++) {
        for (char j = 0; j < NB_COL; j++) {//on parcourt le LCD au complet
            if(m_tabVue[i][j]==TUILE||m_tabVue[i][j]==DRAPEAU) //si c'est un drapeau ou une tuile
                nbTuileEtDrapeau++;//on incr�mente
        }
    }
    if (nbTuileEtDrapeau == *pMines)//si la valeur compt�e pr�c�demment correspond au nombre de mines
    {
        (*pMines)++; //on augmente le nombre de mines � placer pour la prochaine partie
        return true;//retourne vrai (on a gagn�)
    }
    else
        return false;//retourne faux (on a pas gagn�)
}

/*
 * @brief Lit le port analogique. 
 * @param Le no du port � lire
 * @return La valeur des 8 bits de poids forts du port analogique
 */
char getAnalog(char canal) 
{
    ADCON0bits.CHS = canal;
    __delay_us(1);
    ADCON0bits.GO_DONE = 1; //lance une conversion
    while (ADCON0bits.GO_DONE == 1); //attend fin de la conversion
    return ADRESH; //retourne seulement les 8 MSB. On laisse tomber les 2 LSB de ADRESL
}


/*
 * @brief Affiche le tableau m_tabVue.
 * @param rien
 * @return rien
 */
void afficheTabVue(void) 
{
    for (char i = 0; i < NB_LIGNE; i++) {//pour toutes les lignes � �crire
        lcd_gotoXY(1, i + 1);//on se d�place au d�but de la ligne � �crire
        lcd_putMessage(m_tabVue[i]);//on �crit la ligne
    }
}

/*
 * @brief Affiche le tableau m_tabMines.
 * @param rien
 * @return rien
 */
void afficheTabMines(void) 
{
    for (char i = 0; i < NB_LIGNE; i++) {//pour toutes les lignes � �crire
        lcd_gotoXY(1, i + 1);//on se d�place au d�but de la ligne � �crire
        lcd_putMessage(m_tabMines[i]);//on �crit la ligne
    }
}

/*
 * @brief Remplace la tuile s�lectionn�e avec un drapeau dans m_tabVue. Si la case s�lectionn�e
 * est un drapeau, on l'enl�ve. Les drapeaux ne sont pas enlev�s par enleveTuilesAutour.
 * On peut placer un drapeau seulement sur une tuile (pas une case vide ou chiffr�e).
 * @param char x, char y la position du curseur sur le LCD
 * @return rien
 */
void metOuEnleveDrapeau(char x, char y) 
{
    if (m_tabVue[y-1][x-1]==TUILE)//si la case s�lectionn�e est une tuile
        m_tabVue[y-1][x-1]=DRAPEAU;//on la remplace par un drapeau
    else if (m_tabVue[y-1][x-1]==DRAPEAU)//sinon, si c'est un drapeau
        m_tabVue[y-1][x-1]=TUILE;//on le remplace avec une tuile
    afficheTabVue();//on actualise le LCD pour affiche la nouvelle matrice
    while(SW0==0);//boucle antirebond qui attend que le bouton de la carte noire soit rel�ch�
}