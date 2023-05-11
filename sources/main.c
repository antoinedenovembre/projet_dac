#include <dirent.h>
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

            // Ouvrir le dossier contenant les images source
            DIR *dir;
            struct dirent *ent;
            if ((dir = opendir(sourceDir)) != NULL) {
                /* parcourir tous les fichiers et répertoires dans le dossier */
                while ((ent = readdir(dir)) != NULL) {
                    if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
                        continue;
                    }
                    // Générer le chemin complet de l'image source
                    char filePath[200];
                    strcpy(filePath, sourceDir);
                    strcat(filePath, ent->d_name);
                    printf("Traitement de l'image %s ...\n", filePath);

                    // Charger l'image source
                    IMAGE img = {0, 0, NULL, NULL};
                    img = lectureImage(filePath);

                    // TODO : Traiter l'image

                    // Libérer la mémoire
                    liberationImage(&img);
				}
			}
		case 2:
			printf("\nQuelle image souhaitez-vous traiter ? \nEntrez le nom de l'image (exemple : In_1.pgm)\t: ");
            fileName = lectureString(10);

            strcat(sourceDir, fileName);

            // Charger l'image source
            IMAGE img = {0, 0, NULL, NULL};
            img = lectureImage(sourceDir);

            // TODO : Traiter l'image

            // Libérer la mémoire
            liberationImage(&img);
		default:
			printf("\nErreur de saisie.\n");
	}

	return EXIT_SUCCESS;
}
