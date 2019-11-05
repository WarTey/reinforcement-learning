#include <stdio.h>
#include <math.h>
#include "lib/GfxLib.h"
#include "lib/ESLib.h"

#define LargeurFenetre 1200
#define HauteurFenetre 720
#define NB_NOURRITURES 5

typedef enum {GoingUp = 0, GoingDown = 1, GoingRight = 2, GoingLeft = 3} State;
typedef enum {GoUp = 0, GoDown = 1, GoRight = 2, GoLeft = 3} Action;

typedef struct nourriture {
	float x, y;
	bool available;
} nourriture;

typedef struct individu {
	float xVitesse, yVitesse;
	float x, y;
	int panier;
} individu;

const State automaton[4][4] = {
	{GoingUp, GoingDown, GoingRight, GoingLeft},
	{GoingUp, GoingDown, GoingRight, GoingLeft},
	{GoingUp, GoingDown, GoingRight, GoingLeft},
	{GoingUp, GoingDown, GoingRight, GoingLeft}};

float Utility[4][4];
State currentState;
float LearningRate = 0.f;
float Factor = .5f;

void init(individu *humain) {
	currentState = GoingDown;
	humain->y -= humain->yVitesse;
	for (State y = GoingUp; y <= GoingLeft; y++)
		for (Action x = GoUp; x <= GoLeft; x++)
			Utility[y][x] = 1.f;
}

Action chooseAction() {
	const float randomValue = valeurAleatoire()*(Utility[currentState][GoUp]+Utility[currentState][GoDown]+Utility[currentState][GoRight]+Utility[currentState][GoLeft]);
	if (randomValue < Utility[currentState][GoUp])
		return GoUp;
	else if (randomValue < Utility[currentState][GoUp]+Utility[currentState][GoDown])
		return GoDown;
	else if (randomValue < Utility[currentState][GoUp]+Utility[currentState][GoDown]+Utility[currentState][GoRight])
		return GoRight;
	return GoLeft;
}

float reward(State nextState, individu humain, nourriture tabNourriture[NB_NOURRITURES]) {
	int index = 0;
	// On recherche le prochaine panier à chercher
	for (int i = 0; i < NB_NOURRITURES; i++) {
		if (tabNourriture[i].available) {
			index = i;
			break;
		}
	}

	float deltaDist = 0.f;
	switch (nextState) {
		case GoingUp:
			deltaDist = fabs(humain.y-tabNourriture[index].y)-fabs(humain.y-humain.yVitesse-tabNourriture[index].y);
			break;
		case GoingDown:
			deltaDist = fabs(humain.y-tabNourriture[index].y)-fabs(humain.y+humain.yVitesse-tabNourriture[index].y);
			break;
		case GoingRight:
			deltaDist = fabs(humain.x-tabNourriture[index].x)-fabs(humain.x-humain.xVitesse-tabNourriture[index].x);
			break;
		case GoingLeft:
			deltaDist = fabs(humain.x-tabNourriture[index].x)-fabs(humain.x+humain.xVitesse-tabNourriture[index].x);
			break;
	}

	if (deltaDist > 0.f)
		return .5f;
	else if (deltaDist < 0.f)
		return 2.f;
	return 1.f;
}

void updateUtility(State nextState, Action action, individu humain, nourriture tabNourriture[NB_NOURRITURES]) {
	const float maxUtility = fmaxf(Utility[nextState][GoUp], fmaxf(Utility[nextState][GoDown], fmaxf(Utility[nextState][GoRight], Utility[nextState][GoLeft])));
	Utility[currentState][action] = (1.f-LearningRate)*Utility[currentState][action]+LearningRate*(reward(nextState, humain, tabNourriture)+Factor*maxUtility);
}

void update(Action action, individu *humain, nourriture tabNourriture[NB_NOURRITURES]) {
	State newState = automaton[currentState][action];
	switch (newState) {
		case GoingUp:
			humain->y += humain->yVitesse;
			if (humain->y > hauteurFenetre()+30)
				humain->y = hauteurFenetre()/16-30;
			break;
		case GoingDown:
			humain->y -= humain->yVitesse;
			if (humain->y < hauteurFenetre()/16-30)
				humain->y = hauteurFenetre()+30;
			break;
		case GoingRight:
			humain->x += humain->xVitesse;
			if (humain->x > largeurFenetre()+30)
				humain->x = -30;
			break;
		case GoingLeft:
			humain->x -= humain->xVitesse;
			if (humain->x < -30)
				humain->x = largeurFenetre()+30;
			break;
	}
	updateUtility(newState, action, *humain, tabNourriture);
	currentState = newState;
}

void cercle(float centreX, float centreY, float rayon) {
	const int Pas = 180;
	const double PasAngulaire = 2.*M_PI/Pas;
	int index;
	for (index = 0; index < Pas; ++index) {
		const double angle = 2.*M_PI*index/Pas;
		triangle(centreX, centreY, centreX+rayon*cos(angle), centreY+rayon*sin(angle), centreX+rayon*cos(angle+PasAngulaire), centreY+rayon*sin(angle+PasAngulaire));
	}
}

void drawUtility() {
	const int drawStep = 20;
	float somme = 0.f;
	State y;
	for (y = GoingUp; y <= GoingLeft; ++y)
		for (Action x = GoUp; x <= GoLeft; ++x)
			somme += Utility[y][x];
	couleurCourante(0, 255, 0);
	for (y = GoingUp; y <= GoingLeft; ++y)
	{
		Action x;
		for (x = GoUp; x <= GoLeft; ++x)
		{
			cercle(2*drawStep*(1+x), hauteurFenetre()-2*drawStep*(1+y), drawStep/2*Utility[y][x]*5/somme);
		}
	}
}

void generationIndividu(individu *humain) {
	// Génération de notre individu
	for (int i = 0; i < 10; i++) {
		humain->panier = 0;
		// Démarrage au centre de la maison
		humain->x = largeurFenetre()/2;
		humain->y = hauteurFenetre()/2+hauteurFenetre()/16;
		// Vitesse en abscisse et ordonnée
		humain->xVitesse = 5;
		humain->yVitesse = 5;
	}
}

void checkNourriture(nourriture tabNourriture[NB_NOURRITURES]) {
	// Vérifie que les paniers ne soient pas trop proches
	bool indice = false;
	for (int i = 0; i < NB_NOURRITURES; i++) {
		for (int j = 0; j < NB_NOURRITURES; j++) {
			if (i != j) {
				// Regarde la position de la nourriture par rapport aux autres et la repositionne si trop proche
				while (tabNourriture[i].x <= 5 || tabNourriture[i].y <= hauteurFenetre()/16+5 || (tabNourriture[i].x >= largeurFenetre()/2-150 && tabNourriture[i].x <= largeurFenetre()/2+150 && tabNourriture[i].y >= hauteurFenetre()/2+hauteurFenetre()/16-150 && tabNourriture[i].y <= hauteurFenetre()/2+hauteurFenetre()/16+150) || (tabNourriture[i].x >= tabNourriture[j].x-100 && tabNourriture[i].x <= tabNourriture[j].x+100 && tabNourriture[i].y >= tabNourriture[j].y-100 && tabNourriture[i].y <= tabNourriture[j].y+100)) {
					indice = true;
					tabNourriture[i].x = valeurAleatoire()*largeurFenetre();
					tabNourriture[i].y = valeurAleatoire()*hauteurFenetre();
				}
				if (indice) {
					indice = false;
					i = 0;
					j = 0;
				}
			}
		}
	}
}

void generationNourriture(nourriture tabNourriture[NB_NOURRITURES]) {
	// Génération de la nourriture sur le terrain
	for (int i = 0; i < NB_NOURRITURES; i++) {
		tabNourriture[i].available = (i == 0) ? true : false;
		tabNourriture[i].x = valeurAleatoire()*largeurFenetre();
		tabNourriture[i].y = valeurAleatoire()*hauteurFenetre();
		// Regarde si la nourriture n'est pas trop proche des bords de l'écran
		while (tabNourriture[i].x <= 5 || tabNourriture[i].y <= hauteurFenetre()/16+5 || (tabNourriture[i].x >= largeurFenetre()/2-150 && tabNourriture[i].x <= largeurFenetre()/2+150 && tabNourriture[i].y >= hauteurFenetre()/2+hauteurFenetre()/16-150 && tabNourriture[i].y <= hauteurFenetre()/2+hauteurFenetre()/16+150)) {
			tabNourriture[i].x = valeurAleatoire()*largeurFenetre();
			tabNourriture[i].y = valeurAleatoire()*hauteurFenetre();
		}
	}
	checkNourriture(tabNourriture);
}

void afficheIndividu(individu humain) {
	// Affiche notre individu avec son score
	char score[100];
	couleurCourante(0, 0, 215);
	cercle(humain.x, humain.y, 15);
	couleurCourante(255, 255, 255);
	epaisseurDeTrait(2);
	sprintf(score, "%d", humain.panier);
	afficheChaine(score, 10, humain.x-tailleChaine(score, 10), humain.y-5);
}

void afficheNourriture(nourriture tabNourriture[NB_NOURRITURES]) {
	// Affiche la nourriture
	for (int i = 0; i < NB_NOURRITURES; i++) {
		if (tabNourriture[i].available)
			couleurCourante(205, 0, 0);
		else
			couleurCourante(0, 125, 0);
		rectangle(tabNourriture[i].x-5, tabNourriture[i].y-5, tabNourriture[i].x+5, tabNourriture[i].y+5);
	}
}

void ramasse(individu *humain, nourriture tabNourriture[NB_NOURRITURES]) {
	for (int j = 0; j < NB_NOURRITURES; j++) {
		// Lorqu'un individu rentre en collision avec de la nourriture il l'a ramasse (si elle est disponible)
		if (humain->x >= tabNourriture[j].x-20 && humain->x <= tabNourriture[j].x+20 && humain->y >= tabNourriture[j].y-20 && humain->y <= tabNourriture[j].y+20) {
			if (tabNourriture[j].available) {
				humain->panier += 1;
				tabNourriture[j].available = false;
				if (j < NB_NOURRITURES-1)
					tabNourriture[j+1].available = true;
				else
					tabNourriture[0].available = true;
			}
		}
	}
}

void acceleration(individu *humain, bool accelerate) {
	// Augmente ou réduit la vitesse des individus
	if (accelerate) {
		humain->xVitesse *= 2;
		humain->yVitesse *= 2;
	} else {
		humain->xVitesse /= 2;
		humain->yVitesse /= 2;
	}
}

int main(int argc, char **argv) {
	initialiseGfx(argc, argv);
	prepareFenetreGraphique("Eat Me", LargeurFenetre, HauteurFenetre);
	lanceBoucleEvenements();
	return 0;
}

void gestionEvenement(EvenementGfx evenement) {
	static bool accelerate = false;
	static individu humain;
	static nourriture tabNourriture[NB_NOURRITURES];
	
	switch (evenement) {
		case Initialisation:
			// Génération de la nourriture
			generationNourriture(tabNourriture);
			// Génération de notre individu
			generationIndividu(&humain);
			// ???
			init(&humain);
			demandeTemporisation(20);
			break;
		
		case Temporisation:
			// Ramassage de la nourriture
			ramasse(&humain, tabNourriture);
			update(chooseAction(), &humain, tabNourriture);
			rafraichisFenetre();
			break;
			
		case Affichage:
			// Départ écran blanc
			effaceFenetre (255, 255, 255);
			// Affichage de la maison
			couleurCourante(215, 0, 0);
			cercle(largeurFenetre()/2, hauteurFenetre()/2+hauteurFenetre()/16, 75);
			// Affichage de notre individu
			afficheIndividu(humain);
			// Affichage de la nourriture
			afficheNourriture(tabNourriture);
			// Affiche le schéma de 'Utility'
			drawUtility();
			// Traçage d'un rectangle blanc
			couleurCourante(255, 255, 255);
			rectangle(0, 0, largeurFenetre(), hauteurFenetre()/16);
			// Traçage d'une ligne en bas de l'écran
			couleurCourante(0, 0, 0);
			epaisseurDeTrait(2);
			ligne(0, hauteurFenetre()/16, largeurFenetre(), hauteurFenetre()/16);
			// Affiche d'une chaîne de caractères à l'écran
			couleurCourante(0, 0, 0);
			epaisseurDeTrait(2);
			afficheChaine("0...3: learning rate ; 4...6: factor", hauteurFenetre()/32, largeurFenetre()/128, hauteurFenetre()/64);
			break;
			
		case Clavier:
			switch (caractereClavier()) {
				case 'A':
				case 'a':
					// Double ou réduit la vitesse des individus
					accelerate = !accelerate;
					acceleration(&humain, accelerate);
					break;

				case 'Q':
				case 'q':
					termineBoucleEvenements();
					break;

				case 'C':
				case 'c':
					demandeTemporisation(20);
					break;

				case 'P':
				case 'p':
					demandeTemporisation(-1);
					break;
				
				case '0':
					LearningRate = 0.f;
					break;

				case '1':
					LearningRate = .1f;
					break;

				case '2':
					LearningRate = .2f;
					break;

				case '3':
					LearningRate = .3f;
					break;

				case '4':
					Factor = .05f;
					break;

				case '5':
					Factor = .5f;
					break;

				case '6':
					Factor = .95f;
					break;
			}
			break;

		case ClavierSpecial:
			break;
			
		case BoutonSouris:
			break;
		
		case Souris:
			break;
		
		case Inactivite:
			break;
		
		case Redimensionnement:
			break;
	}
}
