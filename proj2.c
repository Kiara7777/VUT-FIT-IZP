/*
 * Soubor:  proj2.c
 * Datum:   2013/11/19
 * Autor:   Sara Skutova, xskuto00@stud.fit.vutbr.cz
 * Projekt: Iteracni vypocty, projekt c. 2 pro predmet IZP
 * Popis:   Program dle uzivatelovy specifikace vypocita a vypise druhou odmocninu, arcus sinus
 *          nebo uhly obecneho trouhelnika
 */
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>

 #define EPS 1.0E-12
 #define SOURADNICE 6

  double my_sqrt(double x);

  const double PI_2 = 1.57079632679489661923;


 /**Napoveda, vytiskne se pri --help*/
const char *HELPMSG =
  "Program: Iteracni vypocty\n"
  "Autor: Sara Skutova (c) 2013\n"
  "Program dle uzivatelovy specifikace vypocita a vypise druhou odmocninu,\narcus sinus"
  " nebo uhly obecneho trouhelnika\n"
  "Pouziti: proj2 --help\n"
  "         proj2 --sqrt X\n"
  "         proj2 --asin X\n"
  "         proj2 --triangle AX AY BX BY CX CY\n"
  "Popis parametru:\n"
  "--help: vytiskne napovedu\n"
  "--sqrt X: vypocita a vypise druhou odmocninu z cisla X\n"
  "--asin X: vypocita a vypise arcus sinus z cisla X\n"
  "--triangle AX AY BX BY CX CY: vypocita a vypise uhly trouhelniku, ktery je dan\n"
  "                              vrcholy A = AX,AY; B = BX,BY; C = CX,CY\n";

/** Kody chyb programu */
enum tecodes
{
  EOK = 0,     /**< Bez chyby */
  ECLWRONG,    /**< Chybny prikazový radek*/
  ESQR_A_EASIN,/**< Cislo je mimo moznosti odmocniny a arcus sinus*/
  ETRIANGLE,   /**< Zadane souradnice nevytvori trouhelnik*/
  EUNKNOWN,    /**< Neznama chyba */
};

/** Kody stavu programu*/
enum testes
{
  SHELP,         /**< Napoveda */
  SSQRT,         /**< Odmocnina*/
  SASIN,         /**< Arkus sinus*/
  STRIANGLE,     /**< Triangle*/
};

/** Chybova hlaseni odpovidajici chybovym kodum. */
const char *ECODEMSG[] =
{
  [EOK] = "Vse v poradku\n",
  [ECLWRONG] = "Chybne parametry prikazoveho radku!\nPro napovedu zadejte parametr --help\n",
  [ESQR_A_EASIN] = "nan\n",
  [ETRIANGLE] = "Zadane souradnice nevytvori trouhelnik\n",
  [EUNKNOWN] = "Nastala neprevidana chyba\n",
};

typedef struct soustava
{
   double ax, ay;
   double bx, by;
   double cx, cy;

}TSOUSAVA;

 /**
 * Struktura obsahujici hodnoty parametru prikazove radky.
 */
typedef struct params
{
  int ecode;       /**< Chybový kod programu, odpovida vyctu tecodes. */
  double numberX;  /**< Cislo X pro odmocninu a arcus sinus*/
  int state;       /**< Stavový kod programu, odpovida vvctu tstates. */
  TSOUSAVA body;   /**< Struktura obsahujici souradnice bodu*/
  double pole[SOURADNICE];
  double strana_a;
  double strana_b;
  double strana_c;
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
* Vypocita vzdalenost mezi 2 body aby mohl urcit delku strany trouhelnika
* @param ax a ay souradnice prvniho bodu
* @param bx a by souradnice druheho bodu
* @return vzdalenost mezi nimi
*/
double get_length(double ax, double ay, double bx, double by)
{
    double vzdalenost;

    vzdalenost = (ax - bx) * (ax - bx) + (ay - by) * (ay - by);
    vzdalenost = my_sqrt(vzdalenost);

    return vzdalenost;
}

/**
* Prevede a zkontroluje cislo s parametru
* @param p_result ukazatel na strukturu result
* @param c pole textových retezcu s argumenty.
*/
void get_number(PARAMS *p_result, char *c[])
{
    char *ch;

    if (p_result->state == SSQRT || p_result->state == SASIN)
    {
        p_result->numberX = strtod(c[2], &ch);

        if (ch == c[2] || *ch != '\0')
            p_result->ecode = ECLWRONG;

        if (p_result->state == SSQRT && p_result->numberX < 0)
            {
                p_result->ecode = ESQR_A_EASIN;
                p_result->numberX = 0;
            }
        else if(p_result->state == SASIN && (p_result->numberX < -1.0 || p_result->numberX > 1.0))
            {
                p_result->ecode = ESQR_A_EASIN;
                p_result->numberX = 0;
            }
    }
    else
    {
        int i = 0;
        for (int n = 2; n <= 7; n++)
        {
            p_result->pole[i] = strtod(c[n], &ch);

            if (ch == c[n] || *ch != '\0')
                p_result->ecode = ECLWRONG;

            i++;
        }
    }
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
    .numberX = 0.0,
    .state = SHELP,
  };
    if (argc == 1)
    result.ecode = ECLWRONG;
    else if (argc == 2)
    {
        if (strcmp("--help", argv[1]) == 0)
            result.state = SHELP;
        else
            result.ecode = ECLWRONG;


    }
    else if (argc == 3)
    {
        if (strcmp("--sqrt", argv[1]) == 0)
        {
            result.state = SSQRT;
            get_number(&result, argv);
        }
         else if (strcmp("--asin", argv[1]) == 0)
        {

            result.state = SASIN;
            get_number(&result, argv);
        }
        else
            result.ecode = ECLWRONG;

    }

    else if (argc == 8)
    {
        if (strcmp("--triangle", argv[1]) == 0)
        {
            result.state = STRIANGLE;
            get_number(&result, argv);

            result.body.ax = result.pole[0];
            result.body.ay = result.pole[1];
            result.body.bx = result.pole[2];
            result.body.by = result.pole[3];
            result.body.cx = result.pole[4];
            result.body.cy = result.pole[5];

            result.strana_c = get_length(result.body.ax, result.body.ay, result.body.bx, result.body.by);
            result.strana_a = get_length(result.body.bx, result.body.by, result.body.cx, result.body.cy);
            result.strana_b = get_length(result.body.cx, result.body.cy, result.body.ax, result.body.ay);

            if ((result.strana_a + result.strana_b > result.strana_c) && (result.strana_a + result.strana_c > result.strana_b) &&
                (result.strana_c + result.strana_b > result.strana_a))
                result.ecode = EOK;
            else
                result.ecode = ETRIANGLE;
        }
        else
            result.ecode = ECLWRONG;
    }
    else
        result.ecode = ECLWRONG;
     return result;

 }

 /**
 * Funkce vypocte absolutni hodnotu ze sveho parametru
 * @param x cislo, ktere se prevede na absolutni hodnotu
 * @return cislo v absolutni hodnote
 **/

 double my_fabs(double x)
{
    return (x < 0) ? -x : x;
}

/**
* Vypocita druhou odmocninu z cisla zadaneho v parametru
* @param x cislo ze ktereho se vypocita druha odmocnina
* @return vypocitana odmocnina
*/

 double my_sqrt(double x)
 {
     double pred_hod = 1.0;

    double nova_hod = ((1.0/2.0) * ((x/pred_hod) + pred_hod));

     while (my_fabs(nova_hod - pred_hod) >=  EPS)
    {
        if (x == 0)
        {
            nova_hod = 0;
            break;
        }
        pred_hod = nova_hod;
        nova_hod = ((1.0/2.0) * ((x/pred_hod) + pred_hod));
    }
    return nova_hod;
 }

 /**
 * Vypocita arkus sinus z cisla zadaneho v parametru
 * Ta nejhnusnesi vec co me kdy napadla
 * @param x cislo z nehos dobry buh Ásin vypocita arkus sinus
 * @return sezvikana mnoha iteracemi hodnota arkus sinus
 */

double my_asin(double x)
{
    //temnoto povstan...
    double pred_vysl_b = x;
    double pred_vysl = x;
    double soucet_rady = pred_vysl;
    double pred_vysl_a = 1.0;
    double k = 0.0;
    double n = 3.0;
    double novy_vysld = 0.0;
    double novy_vysld_a = 0.0;
    double novy_vysld_b = 0.0;

    //...a vykonej sve dilo!

    while (my_fabs(pred_vysl) > EPS * (my_fabs(soucet_rady)))
    {
        novy_vysld_a = pred_vysl_a * ((1 + k) / (2 + k));
        pred_vysl_a = novy_vysld_a;

        pred_vysl_b *= x * x;
        novy_vysld_b = pred_vysl_b / n;

        novy_vysld = novy_vysld_a * novy_vysld_b;

        soucet_rady += novy_vysld;
        pred_vysl = novy_vysld;

        k += 2.0;
        n += 2.0;
    }
    return  soucet_rady;
}

 /**
 * Vypocita uhly v zadanem trouhelniku (alfa, beta, gama)
 * @param a, b, c strany trouhelnika
 * @return vypocitana velikost daneho uhlu
 */
double my_triangle(double a, double b, double c)
{
   double arccos = 0;

   arccos = (a * a + b * b - c * c) / (2 * a * b);
   arccos = my_asin(arccos);
   arccos = PI_2 - arccos;

   return arccos;
}



/** Hlavni program */
int main(int argc, char *argv[])
{
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

    if (params.state == SSQRT)
        printf("%.10e\n", my_sqrt(params.numberX));
    else if (params.state == SASIN)
         printf("%.10e\n", my_asin(params.numberX));
    else
    {
        printf("%.10e\n", my_triangle(params.strana_b, params.strana_c, params.strana_a));
        printf("%.10e\n", my_triangle(params.strana_c, params.strana_a, params.strana_b));
        printf("%.10e\n", my_triangle(params.strana_b, params.strana_a, params.strana_c));
    }
return EXIT_SUCCESS;
}
