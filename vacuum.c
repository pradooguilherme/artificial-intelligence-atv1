#include "vacuum.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
// #include <windows.h> função sleep no windows
#include <math.h>

/*------------------------------------------------------------------------------
    Funções básicas do ambiente
------------------------------------------------------------------------------*/

enviroment newEnviroment(int h, int w)
{
    enviroment E;
    E.h = h;
    E.w = w;
    E.grid = malloc(E.h * sizeof(place *));
    if (E.grid != NULL)
    {
        for (int i = 0; i < h; i++)
        {
            E.grid[i] = malloc(E.w * sizeof(place));
            if (E.grid[i] != NULL)
                for (int j = 0; j < w; j++)
                {
                    E.grid[i][j].dirt = false;
                    E.grid[i][j].row = i;
                    E.grid[i][j].col = j;
                }
        }
    }
    return E;
}

void delEnviroment(enviroment *E)
{
    if (E != NULL)
    {
        for (int i = 0; i < E->h; i++)
        {
            free(E->grid[i]);
        }
        free(E->grid);
    }
}

void initEnviromentDirt(enviroment E, int numDirt)
{
    /*
        Inicializa sujeiras no ambiente.
    */
    srand(time(NULL));
    while (numDirt > 0)
    {
        int i, j;
        i = rand() % E.h;
        j = rand() % E.w;
        if (!E.grid[i][j].dirt)
        {
            E.grid[i][j].dirt = true;
            numDirt--;
        }
    }
}

bool isNeighbor(place p, place q)
{
    /*
        Considerando vizinhança-4 (em cruz). No exemplo abaixo, as posições marcadas
        como v são vizinhas de o, enquanto aquelas marcadas como u não são.
        u v u
        v o v
        u v u
    */
    if (p.row == q.row || p.col == q.col)
    {
        if (p.row == q.row + 1 || p.row == q.row - 1)
            return true;
        if (p.col == q.col + 1 || p.col == q.col - 1)
            return true;
    }
    return false;
}

place *getPlace(enviroment E, int i, int j)
{
    /*
        Retorna ponteiro para place com indices i e j no grid.
    */
    return &E.grid[i][j];
}

/*------------------------------------------------------------------------------
    Funções básicas do agente (cleaner)
------------------------------------------------------------------------------*/

cleaner newCleaner(enviroment E)
{
    cleaner C;
    C.battery = MAX_BATTERY;
    C.whereCharger = &E.grid[0][0];
    C.whereCleaner = &E.grid[0][0];
    C.numActions = 0;
    return C;
}

bool move(cleaner *C, enviroment E, place *target)
{
    /*
        Movimenta o agente para a posição place, desde que esta seja uma vizinha de
        cleaner.whereCleaner e que a bateria de C não esteja vazia.
    */
    if (isNeighbor(*C->whereCleaner, *target) && C->battery > 0)
    {
        C->whereCleaner = target;
        C->battery--;
        C->numActions++;
        return true;
    }
    return false;
}

bool goTarget(cleaner *C, enviroment E, place *target)
{
    if (C->whereCleaner != target)
    {
        // Distâncias verticais e horizontais (distância de Hamming)
        int dist_i = abs(C->whereCleaner->row - target->row);
        int dist_j = abs(C->whereCleaner->col - target->col);
        // Checa se bateria permite deslocamento
        if (dist_i + dist_j < C->battery)
        {
            // Movimento pela altura
            while (C->whereCleaner->row != target->row)
            {
                // Checa se deve mover para cima ou para baixo
                if (C->whereCleaner->row > target->row)
                    move(
                        C, E, &E.grid[C->whereCleaner->row - 1][C->whereCleaner->col]);
                else
                    move(
                        C, E, &E.grid[C->whereCleaner->row + 1][C->whereCleaner->col]);
            }
            // Movimento pela largura
            while (C->whereCleaner->col != target->col)
            {
                // Checa se deve mover para esquerda ou para direita
                if (C->whereCleaner->col > target->col)
                    move(
                        C, E, &E.grid[C->whereCleaner->row][C->whereCleaner->col - 1]);
                else
                    move(
                        C, E, &E.grid[C->whereCleaner->row][C->whereCleaner->col + 1]);
            }
        }
        else
        {
            printf("Bateria insuficiente! \n");
            return false;
        }
    }
    return true;
}

bool charge(cleaner *C, enviroment E)
{
    // Se está na posição do carregador, recarrega
    if (goTarget(C, E, C->whereCharger))
    {
        printf("Recarregando bateria... \n");
        sleep(1);
        C->battery = MAX_BATTERY;
        return true;
    }
    return false;
}

void clean(cleaner *C)
{
    if (C->whereCleaner->dirt)
    {
        C->whereCleaner->dirt = false;
        C->battery--;
        C->numActions++;
    }
}

// Imprime o ambiente e o agente na tela
void printSimulation(cleaner C, enviroment E)
{
    // Posição do carregador é sempre a mesma
    // Itera sobre posições do grid
    for (int i = 0; i < E.h; i++)
    {
        for (int j = 0; j < E.w; j++)
        {
            // Posição do agente
            if (C.whereCleaner->row == i && C.whereCleaner->col == j)
            {
                printf("o");
            }
            else
            {
                if (C.whereCharger->row == i && C.whereCharger->col == j)
                {
                    printf("C");
                }
                else
                {
                    if (E.grid[i][j].dirt)
                        printf("x");
                    else
                        printf("-");
                }
            }
        }
        printf("\n");
    }
    printf("\n");
    sleep(1);
}

bool isDirty(enviroment E, int row, int col)
{
    if (E.grid[row][col].dirt)
    {
        return true;
    }

    return false;
}

bool isAPosition(cleaner C, enviroment E, int row, int col)
{
    if (row >= 0 && row <= E.h - 1 && col >= 0 && col <= E.w - 1)
    {
        if (C.whereCharger != getPlace(E, row, col))
        {
            return true;
        }
    }

    return false;
}

int getDistanceBetweenTwoPoints(place* a, place* b){

    return abs(a->col - b-> col) + abs(a->row - b->row);
}

place *vaccumSensor(cleaner C, enviroment E)
{
    // Essa função é passível de otimização;
    
    int row = C.whereCleaner->row;
    int col = C.whereCleaner->col;

    for(int mov = 1; mov <= E.h + E.w - 2; mov++){
            for(int j = 0; j <= mov; j++){
                int i = mov - j;
                if(isAPosition(C,E, row + i, col + j) && isDirty(E, row + i, col + j)){
                    return getPlace(E, row + i, col + j);
                }
                if(isAPosition(C,E,row - i, col - j) && isDirty(E, row - i, col - j)){
                    return getPlace(E, row - i, col - j);
                }
                if(isAPosition(C,E,row+i, col - j) && isDirty(E, row + i, col - j)){
                    return getPlace(E, row + i, col-j);
                }
                if(isAPosition(C,E,row -i, col + j) && isDirty(E, row-i, col +j)){
                    return getPlace(E,row-i,col+j);
                }
            }    
    }
    printf("Todos os quadrados estão limpos.\n");
    return NULL;
}