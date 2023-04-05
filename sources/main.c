#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib_img/include/libCIPSI1.h"


int main(void)
{
	int choix;
	IMAGE img = { 0,0,NULL,NULL };
	char source[200] = "../images/Source Images/";
	char* nom = "abc";
	printf("Quelle option choisissez-vous ?\n\n");
	printf("\t1 - Vous souhaitez traiter l'ensemble des images.\n");
	printf("\t2 - Vous souhaitez traiter une image en particulier.\n");
	printf("\nChoix : ");
	scanf("%d", &choix);

	switch (choix)
	{
	case 1:
		printf("\nNous allons traiter l'ensemble des images\n");
		break;
	case 2:
		printf("\nQuelle image souhaitez-vous traiter ? \nEntrez le nom de l'image (exemple : In_1.pgm)\t: ");
		nom = EcritureChaineCaracteres(10);
		strcat(source, nom);
		LectureChaineCaracteres(source);
		img = allocationImage(200, 200);
		img = lectureImage(source);
		break;
	default:
		printf("\nErreur de saisie.\n");
	}


}