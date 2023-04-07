#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib_img/include/libCIPSI1.h"

void menu(void)
{
	printf("Quelle option choisissez-vous ?\n\n");
	printf("\t1 - Vous souhaitez traiter l'ensemble des images.\n");
	printf("\t2 - Vous souhaitez traiter une image en particulier.\n");
	printf("\nChoix : ");
}

int main(void)
{
	int choix;
	char sourceDir[200] = "../images/Source Images/";
	char gtDir[200] = "../images/Ground Truth/";
	char* fileName = "";
	
	menu();
	scanf("%d", &choix);

	switch (choix)
	{
		case 1:
			printf("\nNous allons traiter l'ensemble des images\n");
			
			// TODO : Traiter l'ensemble des images
			
			break;
		case 2:
			printf("\nQuelle image souhaitez-vous traiter ? \nEntrez le nom de l'image (exemple : In_1.pgm)\t: ");
			fileName = lectureString(10);
			
			strcat(sourceDir, fileName);
			
			IMAGE img = { 0,0,NULL,NULL };
			img = allocationImage(200, 200);
			img = lectureImage(sourceDir);

			// TODO : Traiter l'image
			
			break;
		default:
			printf("\nErreur de saisie.\n");
	}


}