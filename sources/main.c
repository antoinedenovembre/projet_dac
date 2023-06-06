#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../lib_img/include/libCIPSI1.h"

void menu(void)
{
	printf("Quelle option choisissez-vous ?\n\n");
	printf("\t1 - Traiter l'ensemble des images\n");
	printf("\t2 - Traiter une image en particulier\n");
	printf("\t3 - Quitter\n");
	printf("\nChoix : ");
}

void imageProcessing(char * name, int * iou, int * vinet)
{
	printf("Traitement de l'image %s\n", name);

	char src_path[200];
	strcpy(src_path, "images/Source Images/");
	strcat(src_path, name);

	char gt_path[200];
	strcpy(gt_path, "images/Ground Truth/");
	strcat(gt_path, name);

	IMAGE src = lectureImage(src_path);
	IMAGE src_filtered = filtreMed(src, 4);

	IMAGE gt = lectureImage(gt_path);
	IMAGE gt_binarized = seuillageOtsu(gt);

	STRUCTURE_ELEMENT se = strelDisk(15);
	IMAGE src_tophat;
	if (strncmp(name, "In", 2) == 0) {
		src_tophat = blackTopHat(src_filtered, se);
	}
	else if (strncmp(name, "Sc", 2) == 0) {
		src_tophat = whiteTopHat(src_filtered, se);
	}
	else {
		printf("Nom d'image invalide : %s\n", name);
		return;
	}

	// Améliorer le contraste et binariser l'image top-hat en utilisant la méthode d'Otsu
	IMAGE src_tophat_contrast = expansionImage(src_tophat, 0, 255);
	IMAGE src_tophat_binarized = seuillageOtsu(src_tophat_contrast);

	float score_IOU = scoreIOU(src_tophat_binarized, gt_binarized);
	*iou = (int) (score_IOU * 100);

	// Calculer le score de Vinet
	float score_Vinet = scoreVinet(src_tophat_binarized, gt_binarized);
	*vinet = (int) (score_Vinet * 100);

	// Libérer la mémoire
	liberationImage(&src);
	liberationImage(&src_filtered);
	liberationImage(&gt);
	liberationImage(&gt_binarized);
	liberationImage(&src_tophat);
	liberationImage(&src_tophat_contrast);
	liberationImage(&src_tophat_binarized);
}

int main(void)
{
	int choix = 0;
	
	while (choix != 3)
	{
		menu();
		scanf("%d", &choix);

		switch (choix)
		{
			case 1:
				printf("\nTraitement de l'ensemble des images\n");

				int sum_iou = 0;
				int sum_vinet = 0;

				for (int i = 1; i <= 300; i++) {
					int iou = 0, vinet = 0;
					
					char in[11] = "In_";
					char sc[11] = "Sc_";
					char num[10];
					sprintf(num, "%d", i);
					strcat(in, num);
					strcat(sc, num);
					strcat(in, ".pgm");
					strcat(sc, ".pgm");

					imageProcessing(sc, &iou, &vinet);

					sum_iou += iou;
					sum_vinet += vinet;

					imageProcessing(in, &iou, &vinet);

					sum_iou += iou;
					sum_vinet += vinet;
				}

				printf("Score IOU moyen : %d%%\n", sum_iou / 600);
				printf("Score Vinet moyen : %d%%\n\n[Retour au menu]\n", sum_vinet / 600);
				break;
			case 2:
				printf("\nQuelle image souhaitez-vous traiter ? \nEntrez le nom de l'image (exemple : In_1.pgm)\t: ");
				
				char * fileName = lectureString(10);

				int iou, vinet;

				imageProcessing(fileName, &iou, &vinet);

				printf("Score IOU : %d%%\n", iou);
				printf("Score Vinet : %d%%\n\n[Retour au menu]\n", vinet);
				break;
			case 3:
				break;
			default:
				printf("\nChoix invalide\n");
				break;
		}
	}
	printf("\nArret du programme\n");

	return EXIT_SUCCESS;
}
