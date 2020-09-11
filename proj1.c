/*
 * Soubor:  proj1.c
 * Datum:   2013/10/15
 * Autor:   Sara Skutova, xskuto00@stud.fit.vutbr.cz
 * Projekt: Pocitani slov, projekt c. 1 pro predmet IZP
 * Popis:   Program ve vstupnim textu spocita slova obsahujici uzivatelem specifikovany znak a jejich pocet vypise
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>

#define mez_znak(c) (((c) >= '0' && (c) <= '9')\
                     || ((c) >= 'A' && (c) <= 'Z')\
                     || ((c) >= 'a' && (c) <= 'z')\
                     || ((c) == '-')\
                     || ((c) == '_'))
#define MEZ 81

/** Kody chyb programu */
enum tecodes
{
  EOK = 0,     /**< Bez chyby */
  ECLWRONG,    /**< Chybny prikazový radek*/
  EUNKNOWN,    /**< Neznama chyba */
};

/** Kody stavu programu*/
enum testes
{
  SHELP,         /**< Napoveda */
  SPOC,         /**<Pocitani slov */
};

/** Chybova hlaseni odpovidajici chybovym kodum. */
const char *ECODEMSG[] =
{
  [EOK] = "Vse v poradku\n",
  [ECLWRONG] = "Chybne parametry prikazoveho radku!\nPro napovedu zadejte parametr --help\n",
  [EUNKNOWN] = "Nastala nepredvidaná chyba.\n",
};


/**Napoveda, vytiskne se pri --help*/
const char *HELPMSG =
  "Program: Pocitani slov\n"
  "Autor: Sara Skutova (c) 2013\n"
  "Program ve vstupnim textu spocita slova obsahujici uzivatelm specifikovany znak a jejich pocet vypise\n"
  "Pouziti: proj1 --help\n"
  "         proj1 X [N] [-d]\n"
  "Popis parametru:\n"
  "--help: vytiskne napovedu\n"
  "X: reprezentuje hledany znak v pocitanych slovech\n"
  " -specialni znak : navic symbolizuje libovolne cislo 0-9,\n"
  " -znak ^ symbolizuje libovolne velke pismeno A-Z\n"
  " -znak . symbolizuje libovolny znak\n"
  "N: Volitelny argument N predstavuje cislo vyjadrujici pozici hledaneho znaku v pocitanych slovech\n"
  "Cislo N je kladne - prvni znak ve slove ma pozici 1\n"
  "-d: ladici rezim, vypise kazde nactene slovo na novy radek\n";



 /**
 * Struktura obsahujici hodnoty parametru prikazove radky.
 */
typedef struct params
{
  int ecode;       /**< Chybový kod programu, odpovida vyctu tecodes. */
  int znak;        /**< Vyhledavany znak.*/
  int N;  /**< Hodnota N*/
  bool debg;       /**< Zda je zadan ladici rezim*/
  int state;       /**< Stavový kod programu, odpovida vvctu tstates. */
} PARAMS;

/**
 * Vytiskn e hlaseni odpovidajici chybovemu kodu.
 * @param ecode kod chyby programu
 */
void printECode(int ecode)
{
  if (ecode < EOK || ecode > EUNKNOWN)
  { ecode = EUNKNOWN; }

  fprintf(stderr, "%s", ECODEMSG[ecode]);
}

/**
 * Zpracuje parametr N a vrati ho ve strukture PARAMS. Implementace atoi.
 * Pokud je format N chybny, program se s chybou ukonci.
 * @param p_result ukazatel na strukturu.
 * @param c pole znaku s argumentem N.
 * @return N cislo
*/

 int getNumber(PARAMS *p_result, char c[])
 {
    int i, n = 0;

    for (i = 0; c[i] >= '0' && c[i] <= '9'; i++)
    {
        if (n < INT_MAX/10)
        {
             n = 10 * n + (c[i] - '0');
        }
        else if (n == INT_MAX/10 && (c[i] - '0') <= INT_MAX%10)
        {
            n = 10 * n + (c[i] - '0');
        }
        else
        {
           printf("CHYBA: Parametr N pretekl\n");
           n = 0;
           break;
        }
    }
    if (n > 0 && n < INT_MAX && c[i] == '\0')
        // parametr ulozime do struktury
        p_result->N = n;
    else
        p_result->ecode = ECLWRONG;

    return n;
 }

 /**
 * Zpracuje znak z parametru.
 * V pripade ze je zadano vice znaku (slovo), program vypise chybu.
 * Je-li znak oddelovac vypise se chyba.
 * @param p_result Ukazatel na strukturu
 * @param z pole znaku z parametrem X
 *
 */

 void getZnak(PARAMS *p_result, char z[])
 {
    if (mez_znak(z[0]) || z[0] == ':' || z[0] == '.' || z[0] == '^')
        p_result->znak = z[0];
    else
        p_result->ecode = ECLWRONG;

    if (z[1] != '\0')
        p_result->ecode = ECLWRONG;
 }

/**
 * Zpracuje argumenty prikazoveho radku a vrati je ve strukture PARAMS.
 * Pokud je formát argumentu chybny, ukonci program s chybovym kodem.
 * @param argc Pocet argumentu.
 * @param argv Pole textových retezcu s argumenty.
 * @return Vraci analyzovane argumenty prikazoveho radku.
 */
PARAMS getParams(int argc, char *argv[])
{
  PARAMS result =
  { // inicializace struktury
    .ecode = EOK,
    .N = 0,
    .debg = false,
    .state = SPOC,
  };

  if (argc == 1)
    result.ecode = ECLWRONG;
  else if (argc == 2)
    {
        if (strcmp("--help", argv[1]) == 0)
            result.state = SHELP;
        else
            getZnak(&result, argv[1]);
    }
  else if (argc == 3)
    {
        if (strcmp("-d", argv[2]) == 0)
        {
            result.debg = true;
            getZnak(&result, argv[1]);
        }
        else if (getNumber(&result, argv[2]))
            getZnak(&result, argv[1]);
        else
            result.ecode = ECLWRONG;
        }
    else if (argc == 4)
    {
        if (strcmp("-d", argv[3]) == 0)
        {
            getZnak(&result, argv[1]);
            result.debg = true;
            getNumber(&result, argv[2]);
        }
        else if (getNumber(&result, argv[3]))
        {
            if (strcmp("-d", argv[2]) == 0)
                result.debg = true;
            getZnak(&result, argv[1]);
        }
        else
            result.ecode = ECLWRONG;
    }
    else
        result.ecode = ECLWRONG;

    return result;

}
/**
* Nactene znaky porovnava z hodnotami paramatru
* @param znak      Nacteny znak
* @param poradi    Poradi znaku ve slove
* @param params    Struktura s parametry prikazoveho radku
* @return          0||1 (zda se nasel pozadovany znak)
*/

int hledejZnak(int znak, int poradi, PARAMS params)
{
   int nalezeno = 0;

   if (params.N >= 1)
   {
        if (params.znak == ':' && params.N == poradi && znak >= '0' && znak <= '9')
            nalezeno++;
        else if (params.znak == '^' && params.N == poradi && znak >= 'A' && znak <= 'Z')
            nalezeno++;
        else if (params.znak == '.' && params.N == poradi)
            nalezeno++;
        else if (params.znak == znak && params.N == poradi)
            nalezeno++;
        else
            nalezeno = 0;
    }
   else
   {
        if (params.znak == ':' && znak >= '0' && znak <= '9')
            nalezeno++;
        else if (params.znak == '^' && znak >= 'A' && znak <= 'Z')
            nalezeno++;
        else if (params.znak == '.')
            nalezeno++;
        else if (params.znak == znak)
            nalezeno++;
        else
            nalezeno = 0;
   }
   return nalezeno;
}
/**
* Ze vstupu nacita znaky, v pripade zapnuteho ladiciho
* rezimu je uklada do pole. Pokud slovo bude delsi nez 80
* znaku, bude slovo oriznuto.
* @param mez Makro MEZ
* @param ukazatel na pocet_slov
* @param struktura s parametry
* @return precteny znak
*/

int nactiSlovo(int mez, int *p_pocet, PARAMS params)
{
    int c, i = 0, poc_slov = 0, poc_znak_slov = 0, poz_znak = 0;
            //mez_znak(c) makro pro platnost znaku
    while ((c = getchar()) != EOF && mez_znak(c))
    {
        poz_znak++;
        poc_znak_slov += hledejZnak(c, poz_znak, params);

        if (params.debg && i < mez)
        {
            putchar(c);
            i++;
        }
    }

    if (i > 0)
        putchar('\n');

    if (poc_znak_slov >= 1)
        poc_slov++;

    *p_pocet = poc_slov;

    return c;
}
/** Hlavni program */
int main(int argc, char *argv[])
{
    int d, pocet_slov = 0, cel_pocet = 0;

    PARAMS params = getParams(argc, argv);

    if (params.ecode != EOK)
    {
        printECode(params.ecode);
        return EXIT_FAILURE;
    }

    if (params.state == SHELP)
    {
        printf("%s", HELPMSG);
        return EXIT_SUCCESS;
    }


    while ((d = nactiSlovo(MEZ, &pocet_slov, params)) != EOF)
    {
        cel_pocet += pocet_slov;
    }

    printf("%d\n", cel_pocet);

    return EXIT_SUCCESS;
}
