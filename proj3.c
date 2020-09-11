/*
 * Soubor:  proj.c
 * Datum:   2013/12/7
 * Autor:   Sara Skutova, xskuto00@stud.fit.vutbr.cz
 * Projekt: Hledani obrazcu, projekt c. 3 pro predmet IZP
 * Popis:   Program v danem monochromatickem obrazu hleda nejdelsi vodorovne a svisle
 *          usecky a nejdelsi ctverce. Obrazek je ulozeny jako bitmapa v texxtovem souboru
 *          ve forme obdelnikove matice nul a jednicek.
 *          Cilem je vypsat pocatecni a koncove souradnice nalezene usecy nebo ctverce
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <ctype.h>

/**Napoveda, vytiskne se pri --help*/
const char *HELPMSG =
  "Program: Hledani obrazcu\n"
  "Autor: Sara Skutova (c) 2013\n"
  "Program dle uzivatelovy specifikace vypise pocatecni a koncove \nsouradnice nalezene"
  " vodorovne nebo svisle usecky nebo nejvetsiho ctverce\n"
  "Pouziti: proj3 --help\n"
  "         proj3 --test soubor.txt\n"
  "         proj3 --hline obrazek.txt\n"
  "         proj3 --vline obrazek.txt\n"
  "         proj3 --square obrazek.txt\n"
  "Popis parametru:\n\n"
  "--help: vytiskne napovedu\n\n"
  "--test: zkontroluje, ze dany soubor obsahuje radnou definici bitmapoveho\n\tobrazku a vypise:"
  " valid - pokud obsahuje nebo invalid - pokud ne\n\n"
  "--hline: v danem obrazku nalezne a vytiskne pocatecni a koncovou souradnici\n\t prvni nejdelsi horizontalni usecky\n\n"
  "--vline: v danem obrazku nalezne a vytiskne pocatecnu a koncovou souradnici \n\t prvni nejdelsí vertikalni usecky\n\n"
  "--square: v danem obrazku nalezne a vytiskne pocatecni a koncovou souradnici \n\t  prvniho nejvetsiho ctverce\n";

/** Kody chyb programu */
enum tecodes
{
  EOK = 0,     /**< Bez chyby */
  ECLWRONG,    /**< Chybny prikazový radek*/
  EFILE,       /**< Chyba pri otevirani nebo uzavirani souboru*/
  EVALID,      /**< Soubor obsahuje rednou definici bitmapoveho obrazku*/
  EINVALID,    /**< Soubor neobsahuje radnou definici bitmapoveho obrazku*/
  EALLOC,      /**< Chybna alokace pole*/
  EUNKNOWN,    /**< Neznama chyba */
};

/** Kody stavu programu*/
enum testes
{
  SHELP,         /**< Napoveda */
  STEST,         /**< Test validace obrazku*/
  SHLINE,        /**< Horizontalni usecka*/
  SVLINE,        /**< Vertikalni usecka*/
  SSQUARE,       /**< Nejvetsi ctverec*/

};

/** Chybova hlaseni odpovidajici chybovym kodum. */
const char *ECODEMSG[] =
{
  [EOK] = "Vse v poradku\n",
  [ECLWRONG] = "Chybne parametry prikazoveho radku!\nPro napovedu zadejte parametr --help\n",
  [EFILE] = "Chyba pri otevirani nebo uzavirani souboru\n",
  [EVALID] = "Valid\n",
  [EINVALID] = "Invalid\n",
  [EALLOC] = "Chyba pri alokaci pameti\n",
  [EUNKNOWN] = "Neznama chyba\n",
};

 /**
 * Struktura obsahujici hodnoty parametru prikazove radky.
 */
typedef struct params
{
  int ecode;       /**< Chybový kod programu, odpovida vyctu tecodes. */
  int state;       /**< Stavový kod programu, odpovida vvctu tstates. */
  char *file;      /**< Jmeno souboru. */
} PARAMS;

 /**
 * Struktura obsahujici parametry matice/obrazku.
 */
 typedef struct {
  int rows;        /**< Pocet radku v matici.*/
  int cols;        /**< Pocet sloupcu v matici*/
  char *cells;     /**< Ukazatel na pole radku matice*/
} Bitmap;

/**
 * Vytiskn e hlaseni odpovidajici chybovemu kodu.
 * @param ecode kod chyby programu
 */
void printECode(int ecode)
{
  if (ecode < EOK || ecode > EUNKNOWN)
  { ecode = EUNKNOWN; }

  if (ecode == EVALID || ecode == EINVALID)
    fprintf(stdout, "%s", ECODEMSG[ecode]);
  else
    fprintf(stderr, "%s", ECODEMSG[ecode]);
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
        if (strcmp("--test", argv[1]) == 0)
        {
            result.state = STEST;
            result.file = argv[2];
        }
        else if (strcmp("--hline", argv[1]) == 0)
        {
            result.state = SHLINE;
            result.file = argv[2];
        }
        else if (strcmp("--vline", argv[1]) == 0)
        {
            result.state = SVLINE;
            result.file = argv[2];
        }
        else if (strcmp("--square", argv[1]) == 0)
        {
            result.state = SSQUARE;
            result.file = argv[2];
        }
        else
            result.ecode = ECLWRONG;
    }
    else
        result.ecode = ECLWRONG;

    return result;
}
/**
 * Funkce otestuje zpravnost bitmapoveho obrazku.
 * @param params Ukazatel na strukturu PARAMS.
 * @param bitmap Ukazatel na strukturu Bitmap.
 * @return 1 pokud je neco spatne, 0 pokud je vse v poradku
 */
int testMatrix(PARAMS *params, Bitmap *bitmap)
{
    FILE *fr;
    int velikost = 0;
    int c;
    int number = 0;

    if ((fr = fopen(params->file, "r")) == NULL) //otevrit a zkontrolovat otevreni
    {
        params->ecode = EFILE;
        return EXIT_FAILURE;
    }

    if (fscanf(fr, "%d %d", &bitmap->rows, &bitmap->cols) != 2)
    {
        if (fclose(fr) == EOF)
        {
            params->ecode = EFILE;
            return EXIT_FAILURE;
        }

        params->ecode = EINVALID;
        return EXIT_FAILURE;
    }

    if (bitmap->rows == 0 || bitmap->cols == 0)
    {
        if (fclose(fr) == EOF)
        {
            params->ecode = EFILE;
            return EXIT_FAILURE;
        }

        params->ecode = EINVALID;
        return EXIT_FAILURE;
    }

    velikost = bitmap->rows * bitmap->cols;

    while ((c = getc(fr)) != EOF)
    {
        if (isspace(c) || (isdigit(c) && (c == '0' || c == '1')))
        {
            if(isdigit(c))
                number++;
        }
        else
        {
            if (fclose(fr) == EOF)
            {
                params->ecode = EFILE;
                return EXIT_FAILURE;
            }

            params->ecode = EINVALID;
            return EXIT_FAILURE;
        }
    }

    if (number > velikost || number < velikost)
    {
        if (fclose(fr) == EOF)
        {
            params->ecode = EFILE;
            return EXIT_FAILURE;
        }

        params->ecode = EINVALID;
        return EXIT_FAILURE;
    }

    if (fclose(fr) == EOF)
    {
        params->ecode = EFILE;
        return EXIT_FAILURE;
    }


    params->ecode = EVALID;
    return EXIT_SUCCESS;
}

/**
 * Funkce precte soubor a hodnoty matice ulozi do pole
 * Hodnoty v poli jsou ulozeny jedna za druhou, mezery jsou ignorovany
 * @param params Ukazatel na strukturu PARAMS.
 * @param bitmap Ukazatel na strukturu Bitmap.
 * @return 1 pokud je neco spatne, 0 pokud je vse v poradku
 */
int loadMatrix(PARAMS *params, Bitmap *bitmap)
{
    testMatrix(params, bitmap);

    if (params->ecode == EOK || params->ecode == EVALID)
        params->ecode = EOK;
    else
        return EXIT_FAILURE;

    FILE *fr;

    if ((fr = fopen(params->file, "r")) == NULL)
    {
        params->ecode = EFILE;
        return EXIT_FAILURE;
    }

    fscanf(fr, "%d %d", &bitmap->rows, &bitmap->cols);

    bitmap->cells =(char *) malloc(bitmap->rows * bitmap->cols * sizeof(char));
    if (bitmap->cells == NULL)
    {
        if (fclose(fr) == EOF)
        {
            params->ecode = EFILE;
            return EXIT_FAILURE;
        }
        params->ecode = EALLOC;
        return EXIT_FAILURE;
    }

    int c, i = 0;

    while((c = fgetc(fr)) != EOF)
    {
        if (isdigit(c))
        {
            bitmap->cells[i] = c;
            i++;
        }
        else
            continue;
    }

    if (fclose(fr) == EOF)
    {
        params->ecode = EFILE;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;


}

/**
 * Funcke diky pristupu do struktury pameti zjisti barvu baneho indexu
 * daneho parametrami x a y.
 * Pokud najde cernou barvu(1) vrati 1 pokud bilou(0) vrati 0
 * @param bitmap Ukazatel na strukturu Bitmap
 * @param x hodnota radku (cisluje se od 0)
 * @param y hodnota sloupce (cisluje se od 0)
 * @return 0 || 1 dle barvy (vis vyse)
 */
char getcolor(Bitmap *bitmap, int x, int y)
{
    if (bitmap->cells[(x * bitmap->cols) + y] == '1')
        return 1;
    else
        return 0;
}
/**
 * Funkce nalezna v zadane matici prvni nejvetsi horizontalni usecku
 * @param bitmap Ukazatel na strukturu
 * @param x1 y1 x2 y2 Ukazatele na pocatecni a koncove souradnice usecky
 * @return 1 pokud se utvar podarilo najit, 0 pokud nebzlo nic nalezeno
 */
int find_hline(Bitmap *bitmap, int *x1, int *y1, int *x2, int *y2)
{
    int barva, pocet = 0;
    int sx1, sy1, sx2, sy2, pred_pocet = 0;
    int nalezeno = 0;
    for (int i = 0; i < bitmap->rows; i++)
    {
        for (int j = 0; j < bitmap->cols; j++)
        {
            barva = getcolor(bitmap, i, j);

            if (pocet == 0 && barva == 1)
            {
                *x1 = i;
                *y1 = j;
            }

            if (barva)
                pocet++;

            if (j == (bitmap->cols - 1))
            {
                *x2 = i;
                *y2 = j;

                if (pred_pocet < pocet)
                {
                    pred_pocet = pocet;
                    sx1 = *x1;
                    sy1 = *y1;
                    sx2 = *x2;
                    sy2 = *y2;
                    nalezeno = 1;
                }
                pocet = 0;
            }
            else if (pocet != 0 && barva == 0)
            {
                *x2 = i;
                *y2 = j-1;
                if (pred_pocet < pocet)
                {
                    pred_pocet = pocet;
                    sx1 = *x1;
                    sy1 = *y1;
                    sx2 = *x2;
                    sy2 = *y2;
                    nalezeno = 1;
                }
                pocet = 0;

            }
        }
    }

    if (!nalezeno)
        return 0;
    else
    {
        *x1 = sx1;
        *y1 = sy1;
        *x2 = sx2;
        *y2 = sy2;
        return 1;
    }
}

/**
 * Funkce nalezna v zadane matici prvni nejvetsi vertikalni usecku
 * @param bitmap Ukazatel na strukturu
 * @param x1 y1 x2 y2 Ukazatele na pocatecni a koncove souradnice usecky
 * @return 1 pokud se utvar podarilo najit, 0 pokud nebzlo nic nalezeno
 */

int find_vline(Bitmap *bitmap, int *x1, int *y1, int *x2, int *y2)
{
    int barva, pocet = 0;
    int sx1, sy1, sx2, sy2, pred_pocet = 0;
    int nalezeno = 0;
    for (int i = 0; i < bitmap->cols; i++)
    {
        for (int j = 0; j < bitmap->rows; j++)
        {
            barva = getcolor(bitmap, j, i);

            if (pocet == 0 && barva == 1)
            {
                *x1 = j;
                *y1 = i;
            }

            if (barva)
                pocet++;

            if (j == (bitmap->rows - 1))
            {
                *x2 = j;
                *y2 = i;

                if (pred_pocet < pocet)
                {
                    pred_pocet = pocet;
                    sx1 = *x1;
                    sy1 = *y1;
                    sx2 = *x2;
                    sy2 = *y2;
                    nalezeno = 1;
                }
                pocet = 0;
            }
            else if (pocet != 0 && barva == 0)
            {
                *x2 = j-1;
                *y2 = i;
                if (pred_pocet < pocet)
                {
                    pred_pocet = pocet;
                    sx1 = *x1;
                    sy1 = *y1;
                    sx2 = *x2;
                    sy2 = *y2;
                    nalezeno = 1;
                }
                pocet = 0;

            }
        }
    }

    if (!nalezeno)
        return 0;
    else
    {
        *x1 = sx1;
        *y1 = sy1;
        *x2 = sx2;
        *y2 = sy2;
        return 1;
    }

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

    Bitmap bitmap =
    {
        .rows = 0,
        .cols = 0,
        .cells = NULL
    };

    params.ecode = EOK;

    if (params.state == STEST)
    {
        testMatrix(&params, &bitmap);
        if (params.ecode != EOK)
        {
            printECode(params.ecode);
            return EXIT_FAILURE;
        }
    }
    else if (params.state == SHLINE)
    {
        loadMatrix(&params, &bitmap);
        if (params.ecode != EOK)
        {
            printECode(params.ecode);
            return EXIT_FAILURE;
        }

        int x1, y1, x2, y2;
        int hline = find_hline(&bitmap, &x1, &y1, &x2, &y2);

        if (!hline || (x1 == x2 && y1 == y2))
            printf("nenasel se pozadovany utvar\n");
        else
            printf("%d %d %d %d\n", x1, y1, x2, y2);

        free(bitmap.cells);
    }
    else if (params.state == SVLINE)
    {
        loadMatrix(&params, &bitmap);
        if (params.ecode != EOK)
        {
            printECode(params.ecode);
            return EXIT_FAILURE;
        }

        int x1, y1, x2, y2;
        int vline = find_vline(&bitmap, &x1, &y1, &x2, &y2);

        if (!vline || (x1 == x2 && y1 == y2))
            printf("Nenasel se pozadovany utvar\n");
        else
            printf("%d %d %d %d\n", x1, y1, x2, y2);

        free(bitmap.cells);
    }
    else if (params.state == SSQUARE)
    {
        loadMatrix(&params, &bitmap);
        if (params.ecode != EOK)
        {
            printECode(params.ecode);
            return EXIT_FAILURE;
        }

        printf("NEFUNGUJE!\n");

        free(bitmap.cells);

    }
    return 0;
}
