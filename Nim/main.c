#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <math.h>
#include "lib/ESLib.h"

#define NB_STATES 13
#define NB_ACTIONS 3

typedef int Action;
typedef int State;

State currentState = 12;
float learningRate = 0.1;
float factor = 0.9;
float utility[NB_STATES][NB_ACTIONS];
const State automaton[NB_STATES][NB_ACTIONS] = {
    {0, 0, 0},
    {0, 0, 0},
    {1, 0, 0},
    {2, 1, 0},
    {3, 2, 1},
    {4, 3, 2},
    {5, 4, 3},
    {6, 5, 4},
    {7, 6, 5},
    {8, 7, 6},
    {9, 8, 7},
    {10, 9, 8},
    {11, 10, 9}
};

void init() {
    for (State x = 0; x < NB_STATES; x++)
        for (Action y = 0; y < NB_ACTIONS; y++)
            utility[x][y] = 1;
}

int chooseAction() {
    float select = valeurIntervalleZeroUn()*(utility[currentState][0]+utility[currentState][1]+utility[currentState][2]);
    if (select < utility[currentState][0])
        return 0;
    else if (select < utility[currentState][0] + utility[currentState][1])
        return 1;
    else
        return 2;
}

float reward(State nextState) {
    for (int i = 1; i < currentState; i += 4)
        if (nextState == i)
            return 1;
    return -1;
}

void updateUtility(State nextState, Action action) {
    float maxUtility = fmax(utility[nextState][0], fmax(utility[nextState][1], utility[nextState][2]));
    utility[currentState][action] = (1-learningRate)*utility[currentState][action] + learningRate*(reward(nextState) + factor*maxUtility);
}

void update(Action action) {
    State nextState = automaton[currentState][action];
    updateUtility(nextState, action);
    currentState = nextState;
}

int main(void) {
    bool playerTurn = false;
    int choix = 0;
    int iter = 10000;
    init();
    while (iter > 0) {
        update(chooseAction());
        if (currentState == 0) {
            currentState = 12;
            iter -= 1;
        }
    }

    for (State x = 0; x < NB_STATES; x++)
        for (Action y = 0; y < NB_ACTIONS; y++)
            if (utility[x][y] < 0.9)
                utility[x][y] = 0;

    for (State x = 2; x < NB_STATES; x++) {
        for (Action y = 0; y < NB_ACTIONS; y++)
            printf("%.1f ", utility[x][y]);
        printf("\n");
    }

    printf("IA Ready!\n");

    while (true) {
        learningRate = 0;
        printf("%d\n", currentState);
        if (!playerTurn) {
            update(chooseAction());
            printf("IA played\n");
            playerTurn = true;
        } else {
            printf("Your turn (1, 2 or 3)\n");
            scanf("%d", &choix);
            currentState -= choix;
            playerTurn = false;
        }
        if (currentState == 0) {
            if (!playerTurn)
                printf("You Loose\n\n");
            else
                printf("You Win\n\n");
            currentState = 12;
        }
    }
}