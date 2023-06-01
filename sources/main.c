#include 
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


    const char* source_dir = "images/Source Images/";
    const char* gt_dir = "images/Ground Truth/";

    // Récupérer la liste de tous les fichiers .pgm dans le répertoire source
    IMAGE* source_files = NULL;
    int num_source_files = 0;
    source_files = listFiles(source_dir, ".pgm", &num_source_files);

    // Calculer le score de Jaccard pour chaque image et afficher le score moyen
    float* scores = (float*)calloc(num_source_files, sizeof(float));
    for (int i = 0; i < num_source_files; i++)
    {
        const char* source_name = source_files[i].data;
        const char* source_path = concatenatePaths(source_dir, source_name);

        const char* gt_name = source_name;
        const char* gt_path = concatenatePaths(gt_dir, gt_name);

        // Lire l'image source et appliquer un filtre médian
        IMAGE source = lectureImage(source_path);
        IMAGE source_filtered = filtreMed(source, 4, 4);

        // Lire l'image ground truth et la binariser
        IMAGE gt = lectureImage(gt_path);
        IMAGE gt_binarized = seuillageOtsu(gt);

        // Effectuer une opération top-hat morphologique
        STRUCTURE_ELEMENT se = strelDisk(15);
        IMAGE source_tophat;
        const char* top_hat_operation;
        if (strncmp(source_name, "In", 2) == 0) {
            // Top-hat noir pour les images négatives
            top_hat_operation = "Top-Hat Noir";
            source_tophat = blackTopHat(source_filtered, se);
        }
        else if (strncmp(source_name, "Sc", 2) == 0) {
            // Top-hat blanc pour les images positives
            top_hat_operation = "Top-Hat Blanc";
            source_tophat = whiteTopHat(source_filtered, se);
        }
        else {
            printf("Nom d'image invalide : %s\n", source_name);
            continue;
        }


        // Améliorer le contraste et binariser l'image top-hat en utilisant la méthode d'Otsu
        IMAGE source_tophat_contrast = expansionImage(source_tophat, 0, 255);
        IMAGE source_tophat_binarized = seuillageOtsu(source_tophat_contrast);

        // Définir le seuil de surface
        int area_threshold = 200;

        // Supprimer les petites composantes connexes de l'image binaire
        IMAGE source_tophat_binarized_clean = delSmallCompImage(source_tophat_binarized, area_threshold);

        // Calculer le score de IoU et l'ajouter à la liste des scores

        float score_IOU = scoreIOU(source_tophat_binarized_clean, gt_binarized);
        printf("score IOU = %f\n", score_IOU);


        // Libérer la mémoire
        freeImage(source);
        freeImage(source_filtered);
        freeImage(gt);
        freeImage(gt_binarized);
        freeImage(source_tophat);
        freeImage(source_tophat_contrast);
        freeImage(source_tophat_binarized);
        freeImage(source_tophat_binarized_clean);
    }


}
