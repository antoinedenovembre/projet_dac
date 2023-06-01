// libCIPSI1.c
// Hubert Konik - filire IPSI

#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../include/libCIPSI1.h"

/* static -> non extern */

float scoreIOU(IMAGE im1, IMAGE im2)
{
	int i, j;
	int nbPixelsCommuns = 0;
	int nbPixelsUnion = 0;

	for (i = 0; i<im1.Nblig; i++)
		for (j = 0; j<im1.Nbcol; j++) {
			if (im1.pixel[i][j] == 255 && im2.pixel[i][j] == 255)
				nbPixelsCommuns++;
			if (im1.pixel[i][j] == 255 || im2.pixel[i][j] == 255)
				nbPixelsUnion++;
		}

	return((float)nbPixelsCommuns / (float)nbPixelsUnion);
}

// Vinet too many functions, see later

REGION* findRegions(IMAGE im, int* nbRegions)
{
    // Allocate memory for the maximum possible number of regions
    int maxNbRegions = (im.Nblig * im.Nbcol) / 4;
    REGION* regions = malloc(maxNbRegions * sizeof(REGION));

    // Find all regions in the image
    *nbRegions = 0;
    int i, j;
    for (i = 0; i < im.Nblig; i++)
    {
        for (j = 0; j < im.Nbcol; j++)
        {
            if (im.pixel[i][j] == 255)
            {
                // Expand the current region until it covers all adjacent pixels
                REGION region = {j, i, j, i};
                expandRegion(im, &region);
                regions[*nbRegions] = region;
                (*nbRegions)++;
            }
        }
    }

    // Trim the excess memory used for the regions array
    regions = realloc(regions, (*nbRegions) * sizeof(REGION));

    return regions;
}

void expandRegion(IMAGE im, REGION* region)
{
    int x, y;
    for (x = region->x1; x <= region->x2; x++)
    {
        for (y = region->y1; y <= region->y2; y++)
        {
            if (im.pixel[y][x] == 255)
            {
                // Expand the region horizontally
                if (x < region->x1) region->x1 = x;
                if (x > region->x2) region->x2 = x;
                // Expand the region vertically
                if (y < region->y1) region->y1 = y;
                if (y > region->y2) region->y2 = y;
                // Clear the pixel to avoid revisiting it
                im.pixel[y][x] = 0;
                // Recursively expand the adjacent pixels
                expandRegion(im, region);
            }
        }
    }
}

float computeOverlap(REGION r1, REGION r2)
{
    int x1 = MAX(r1.x1, r2.x1);
    int y1 = MAX(r1.y1, r2.y1);
    int x2 = MIN(r1.x2, r2.x2);
    int y2 = MIN(r1.y2, r2.y2);

    if (x1 > x2 || y1 > y2)
    {
        // The regions do not overlap
        return 0.0f;
    }
    else
    {
        // Compute the overlap score
        int nbPixelsIntersection = (x2 - x1 + 1) * (y2 - y1 + 1);
        int nbPixelsUnion = (r1.x2 - r1.x1 + 1) * (r1.y2 - r1.y1 + 1) +
                            (r2.x2 - r2.x1 + 1) * (r2.y2 - r2.y1 + 1) -
                            nbPixelsIntersection;
        return (float)nbPixelsIntersection / (float)nbPixelsUnion;
    }
}


float scoreVinet(IMAGE im1, IMAGE im2)
{
    int i, j, k, l;
    int nbPairsCommuns = 0;
    int nbPairsInRef = 0;
    int nbPairsInTest = 0;

    // Find all pairs of regions in the ground truth image
    int nbRegionsRef = 0;
    REGION *regionsRef = findRegions(im1, &nbRegionsRef);

    // Find all pairs of regions in the processed image
    int nbRegionsTest = 0;
    REGION *regionsTest = findRegions(im2, &nbRegionsTest);

    // Compute the overlap score for each pair of regions
    for (i = 0; i < nbRegionsRef; i++)
    {
        for (j = 0; j < nbRegionsTest; j++)
        {
            float overlap = computeOverlap(regionsRef[i], regionsTest[j]);
            if (overlap > 0)
            {
                nbPairsCommuns++;
            }
        }
    }

    // Compute the number of pairs in the ground truth image and in the processed image
    nbPairsInRef = nbRegionsRef * nbRegionsRef;
    nbPairsInTest = nbRegionsTest * nbRegionsTest;

    // Compute the Vinet's measure
    float precision = (float)nbPairsCommuns / (float)nbPairsInTest;
    float recall = (float)nbPairsCommuns / (float)nbPairsInRef;

    free(regionsRef);
    free(regionsTest);

    return (2 * precision * recall) / (precision + recall);
}

typedef struct
{
	int Nblig;
	int Nbcol;
	int *data;
	int **pixel;
} intIMAGE;

static intIMAGE allocationIntImage(int Nblig, int Nbcol)
{
	intIMAGE mat = { 0,0,NULL,NULL };
	int i;

	mat.Nblig = Nblig;
	mat.Nbcol = Nbcol;
	mat.data = (int*)calloc(Nblig*Nbcol, sizeof(int)); /* mise � 0 par d�faut, gestion des bords lors de l'agrandissement op�r� dans l'�tiquetage */
	if (mat.data == NULL)
		return(mat);
	mat.pixel = (int**)malloc(Nblig*sizeof(int*));
	if (mat.pixel == NULL) {
		free(mat.data);
		mat.data = NULL;
		return(mat);
	}
	for (i = 0; i<Nblig; i++)
		mat.pixel[i] = &mat.data[i*Nbcol];

	return(mat);
}

static void liberationIntImage(intIMAGE *img)
{
	if (img->data != NULL) {
		free(img->data);
		img->data = NULL;
	}
	if (img->pixel != NULL) {
		free(img->pixel);
		img->pixel = NULL;
	}
}

/* fonctionnalit�s -> exportables */

IMAGE allocationImage(int Nblig, int Nbcol)
{
	IMAGE mat = { 0,0,NULL,NULL };
	int i;

	mat.Nblig = Nblig;
	mat.Nbcol = Nbcol;
	mat.data = (unsigned char*)calloc(Nblig*Nbcol,sizeof(unsigned char));
	if (mat.data == NULL)
		return(mat);
	mat.pixel = (unsigned char**)malloc(Nblig*sizeof(unsigned char*));
	if (mat.pixel == NULL) {
		free(mat.data);
		mat.data = NULL;
		return(mat);
	}
	for (i = 0; i<Nblig; i++)
		mat.pixel[i] = &mat.data[i*Nbcol];

	return(mat);
}

void initialisationAleatoireImage(IMAGE img, int ngMin, int ngMax)
{
	int i, j;
	printf("Dans initialisation : img -> %p img.data -> %p img.pixel -> %p\n", &img, img.data, img.pixel);

	for (i = 0; i<img.Nblig; i++)
		for (j = 0; j<img.Nbcol; j++)
			img.pixel[i][j] = (unsigned char)(ngMin + rand() % (ngMax + 1 - ngMin));

	/* solution avec un seul indice
	for (i=0;i<img.Nblig*img.Nbcol;i++)
	img.data[i] = (unsigned char)(ngMin + rand() % (ngMax + 1 - ngMin));
	*/
}

void liberationImage(IMAGE *img)
{
	if (img->data != NULL) {
		free(img->data);
		img->data = NULL;
	}
	if (img->pixel != NULL) {
		free(img->pixel);
		img->pixel = NULL;
	}
}

IMAGE lectureImage(const char *in)
{
	FILE *F = NULL;
	IMAGE img = { 0,0,NULL };
	int dynamique = 0;

	if ((F = fopen(in, "r")) == NULL)
	{
		printf("Pb image inexistante");
	}
	else
	{
		char type[3];

		fgets(type, 3, F);
		fclose(F);
		/* selon le type ouverture binaire ou texte */
		if (strcmp(type, "P2") == 0)  /* cas ASCII niveaux de gris */
		{
			char buf;

			F = fopen(in, "r");

			/* lecture caract�re apr�s caract�re compte-tenu de la diversit� des ent�tes possibles */
			fscanf(F, "%c", &type[0]);
			fscanf(F, "%c", &type[1]);
			fscanf(F, "%c", &buf); /* caract�re espacement */

			fscanf(F, "%c", &buf);
			if (buf == '#') {
				/* on ignore tout jusqu'� trouver '\n' */
				while (buf != '\n')
					fscanf(F, "%c", &buf);
				fscanf(F, "%c", &buf);
			}
			while (((buf - '0') >= 0) && ((buf - '0') <= 9)) { /* possibilit� d'utiliser �galement isdigit de la librairie standard <ctype.h> */
				img.Nbcol = img.Nbcol * 10 + (buf - '0');
				fscanf(F, "%c", &buf);
			}

			fscanf(F, "%c", &buf);
			if (buf == '#') {
				/* on ignore tout jusqu'� trouver '\n' */
				while (buf != '\n')
					fscanf(F, "%c", &buf);
				fscanf(F, "%c", &buf);
			}
			while (((buf - '0') >= 0) && ((buf - '0') <= 9)) {
				img.Nblig = img.Nblig * 10 + (buf - '0');
				fscanf(F, "%c", &buf);
			}

			fscanf(F, "%c", &buf);
			if (buf == '#') {
				/* on ignore tout jusqu'� trouver '\n' */
				while (buf != '\n')
					fscanf(F, "%c", &buf);
				fscanf(F, "%c", &buf);
			}
			while (((buf - '0') >= 0) && ((buf - '0') <= 9)) {
				dynamique = dynamique * 10 + (buf - '0');
				fscanf(F, "%c", &buf);
			}

			/* d�but des data */

			printf("Lecture image NG type %s avec %d lignes et %d colonnes...\n", type, img.Nblig, img.Nbcol);

			/* taille connue, allocation dynamique possible */
			img = allocationImage(img.Nblig, img.Nbcol);

			/* lecture pixel par pixel */
			{
				int i, j;
				int tmp;

				for (i = 0; i<img.Nblig; i++)
					for (j = 0; j<img.Nbcol; j++)
					{
						fscanf(F, "%d", &tmp);
						img.pixel[i][j] = (unsigned char)tmp;
					}
			}
		}
		else
			if (strcmp(type, "P5") == 0)  /* cas brut niveaux de gris */
			{
				char buf;

				F = fopen(in, "rb");

				/* lecture caract�re apr�s caract�re compte-tenu de la diversit� des ent�tes possibles */
				type[0] = fgetc(F);
				type[1] = fgetc(F);
				buf = fgetc(F); /* caract�re espacement */

				buf = fgetc(F);
				if (buf == '#') {
					/* on ignore tout jusqu'� trouver '\n' */
					while (buf != '\n')
						buf = fgetc(F);
					buf = fgetc(F);
				}
				while (((buf - '0') >= 0) && ((buf - '0') <= 9)) { /* possibilit� d'utiliser �galement isdigit de la librairie standard <ctype.h> */
					img.Nbcol = img.Nbcol * 10 + (buf - '0');
					buf = fgetc(F);
				}

				buf = fgetc(F);
				if (buf == '#') {
					/* on ignore tout jusqu'� trouver '\n' */
					while (buf != '\n')
						buf = fgetc(F);
					buf = fgetc(F);
				}
				while (((buf - '0') >= 0) && ((buf - '0') <= 9)) {
					img.Nblig = img.Nblig * 10 + (buf - '0');
					buf = fgetc(F);
				}

				buf = fgetc(F);
				if (buf == '#') {
					/* on ignore tout jusqu'� trouver '\n' */
					while (buf != '\n')
						buf = fgetc(F);
					buf = fgetc(F);
				}
				while (((buf - '0') >= 0) && ((buf - '0') <= 9)) {
					dynamique = dynamique * 10 + (buf - '0');
					buf = fgetc(F);
				}

				/* d�but des data */

				printf("Lecture image NG type %s avec %d lignes et %d colonnes...\n", type, img.Nblig, img.Nbcol);

				/* taille connue, allocation dynamique possible */
				img = allocationImage(img.Nblig, img.Nbcol);

				/* lecture d'un bloc */
				fread(img.data, sizeof(unsigned char), img.Nbcol*img.Nblig, F);
			}
			else
				printf("Format non supporte pour l'instant...\n");
		fclose(F);
	}
	return img;
}

void sauvegardeImage(IMAGE img, const char *type, const char *out)
{
	FILE *F = NULL;

	if (img.data) /* image non vide */
	{
		printf("Sauvegarde image au type %s avec %d lignes et %d colonnes...\n", type, img.Nblig, img.Nbcol);
		/* selon le type ouverture binaire ou texte */
		if (strcmp(type, "P2") == 0)  /* cas ASCII niveaux de gris */
		{
			F = fopen(out, "w");

			fprintf(F, "%s\n", type);
			fprintf(F, "# Created by TSE FISA IPSI\n");
			fprintf(F, "%d %d %d\n", img.Nbcol, img.Nblig, 255);
			/* sauvegarde pixel par pixel */
			{
				int i, j;

				for (i = 0; i<img.Nblig; i++)
					for (j = 0; j<img.Nbcol; j++)
						fprintf(F, "%d ", img.pixel[i][j]);
			}
			fclose(F);
		}
		else
			if (strcmp(type, "P5") == 0)  /* cas brut niveaux de gris */
			{
				char tmp[255];

				F = fopen(out, "wb");

				fputs("P5\n", F);
				fputs("# Created by TSE FISA IPSI\n", F);
				sprintf(tmp, "%d %d %d\n", img.Nbcol, img.Nblig, 255);
				fputs(tmp, F);
				/* sauvegarde par bloc */
				fwrite(img.data, sizeof(unsigned char), img.Nbcol*img.Nblig, F);
				fclose(F);
			}
	}
	else
		printf("Image vide\n");
}

IMAGERGB allocationImageRGB(int Nblig, int Nbcol)
{
	IMAGERGB mat = { 0,0,NULL,NULL };
	int i;

	mat.Nblig = Nblig;
	mat.Nbcol = Nbcol;
	mat.data = (RGB*)malloc(Nblig*Nbcol*sizeof(RGB));
	if (mat.data == NULL)
		return(mat);
	mat.pixel = (RGB**)malloc(Nblig*sizeof(RGB*));
	if (mat.pixel == NULL) {
		free(mat.data);
		mat.data = NULL;
		return(mat);
	}
	for (i = 0; i<Nblig; i++)
		mat.pixel[i] = &(mat.data[i*Nbcol]);

	return(mat);
}

void liberationImageRGB(IMAGERGB *m)
{
	if (m->data != NULL) {
		free(m->data);
		m->data = NULL;
	}
	if (m->pixel != NULL) {
		free(m->pixel);
		m->pixel = NULL;
	}
}

IMAGERGB lectureImageRGB(const char *in)
{
	FILE *F = NULL;
	IMAGERGB img = { 0,0,NULL };
	int dynamique = 0;

	if ((F = fopen(in, "r")) == NULL)
	{
		printf("Pb image inexistante");
	}
	else
	{
		char type[3];

		fgets(type, 3, F);
		fclose(F);
		/* selon le type ouverture binaire ou texte */
		if (strcmp(type, "P3") == 0)  /* cas ASCII couleur */
		{
			char tmp[255] = "";
			char buf;

			F = fopen(in, "r");

			/* lecture caract�re apr�s caract�re compte-tenu de la diversit� des ent�tes possibles */
			fscanf(F, "%c", &type[0]);
			fscanf(F, "%c", &type[1]);
			fscanf(F, "%c", &buf); /* caract�re espacement */

			fscanf(F, "%c", &buf);
			if (buf == '#') {
				/* on ignore tout jusqu'� trouver '\n' */
				while (buf != '\n')
					fscanf(F, "%c", &buf);
				fscanf(F, "%c", &buf);
			}
			while (((buf - '0') >= 0) && ((buf - '0') <= 9)) { /* possibilit� d'utiliser �galement isdigit de la librairie standard <ctype.h> */
				img.Nbcol = img.Nbcol * 10 + (buf - '0');
				fscanf(F, "%c", &buf);
			}

			fscanf(F, "%c", &buf);
			if (buf == '#') {
				/* on ignore tout jusqu'� trouver '\n' */
				while (buf != '\n')
					fscanf(F, "%c", &buf);
				fscanf(F, "%c", &buf);
			}
			while (((buf - '0') >= 0) && ((buf - '0') <= 9)) {
				img.Nblig = img.Nblig * 10 + (buf - '0');
				fscanf(F, "%c", &buf);
			}

			fscanf(F, "%c", &buf);
			if (buf == '#') {
				/* on ignore tout jusqu'� trouver '\n' */
				while (buf != '\n')
					fscanf(F, "%c", &buf);
				fscanf(F, "%c", &buf);
			}
			while (((buf - '0') >= 0) && ((buf - '0') <= 9)) {
				dynamique = dynamique * 10 + (buf - '0');
				fscanf(F, "%c", &buf);
			}

			/* d�but des data */

			printf("Lecture image RGB type %s avec %d lignes et %d colonnes...\n", type, img.Nblig, img.Nbcol);

			/* taille connue, allocation dynamique possible */
			img = allocationImageRGB(img.Nblig, img.Nbcol);

			/* lecture pixel par pixel */
			{
				int i, j;
				int tmp;

				for (i = 0; i<img.Nblig; i++)
					for (j = 0; j<img.Nbcol; j++)
					{
						fscanf(F, "%d", &tmp);
						img.pixel[i][j].R = (unsigned char)tmp;
						fscanf(F, "%d", &tmp);
						img.pixel[i][j].G = (unsigned char)tmp;
						fscanf(F, "%d", &tmp);
						img.pixel[i][j].B = (unsigned char)tmp;
					}
			}
		}
		else
			if (strcmp(type, "P6") == 0)  /* cas brut couleur */
			{
				char buf;

				F = fopen(in, "rb");

				/* lecture caract�re apr�s caract�re compte-tenu de la diversit� des ent�tes possibles */
				type[0] = fgetc(F);
				type[1] = fgetc(F);
				buf = fgetc(F); /* caract�re espacement */

				buf = fgetc(F);
				if (buf == '#') {
					/* on ignore tout jusqu'� trouver '\n' */
					while (buf != '\n')
						buf = fgetc(F);
					buf = fgetc(F);
				}
				while (((buf - '0') >= 0) && ((buf - '0') <= 9)) { /* possibilit� d'utiliser �galement isdigit de la librairie standard <ctype.h> */
					img.Nbcol = img.Nbcol * 10 + (buf - '0');
					buf = fgetc(F);
				}

				buf = fgetc(F);
				if (buf == '#') {
					/* on ignore tout jusqu'� trouver '\n' */
					while (buf != '\n')
						buf = fgetc(F);
					buf = fgetc(F);
				}
				while (((buf - '0') >= 0) && ((buf - '0') <= 9)) {
					img.Nblig = img.Nblig * 10 + (buf - '0');
					buf = fgetc(F);
				}

				buf = fgetc(F);
				if (buf == '#') {
					/* on ignore tout jusqu'� trouver '\n' */
					while (buf != '\n')
						buf = fgetc(F);
					buf = fgetc(F);
				}
				while (((buf - '0') >= 0) && ((buf - '0') <= 9)) {
					dynamique = dynamique * 10 + (buf - '0');
					buf = fgetc(F);
				}

				/* d�but des data */

				printf("Lecture image RGB type %s avec %d lignes et %d colonnes...\n", type, img.Nblig, img.Nbcol);

				/* taille connue, allocation dynamique possible */
				img = allocationImageRGB(img.Nblig, img.Nbcol);

				/* lecture d'un bloc */
				fread(img.data, sizeof(RGB), img.Nbcol*img.Nblig, F);
			}
			else
				printf("Format non supporte pour l'instant...\n");
		fclose(F);
	}
	return img;
}

void sauvegardeImageRGB(IMAGERGB img, const char *type, const char *out)
{
	FILE *F = NULL;

	if (img.data) /* image non vide */
	{
		printf("Sauvegarde image au type %s avec %d lignes et %d colonnes...\n", type, img.Nblig, img.Nbcol);
		/* selon le type ouverture binaire ou texte */
		if (strcmp(type, "P3") == 0)  /* cas ASCII couleur */
		{
			F = fopen(out, "w");

			fprintf(F, "%s\n", type);
			fprintf(F, "# Created by TSE FISA IPSI\n");
			fprintf(F, "%d %d %d\n", img.Nbcol, img.Nblig, 255);
			/* sauvegarde pixel par pixel */
			{
				int i, j;

				for (i = 0; i<img.Nblig; i++)
					for (j = 0; j<img.Nbcol; j++) {
						fprintf(F, "%d ", img.pixel[i][j].R);
						fprintf(F, "%d ", img.pixel[i][j].G);
						fprintf(F, "%d ", img.pixel[i][j].B);
					}
				fclose(F);
			}
		}
		else
			if (strcmp(type, "P6") == 0)  /* cas brut couleur */
			{
				char tmp[255];

				F = fopen(out, "wb");

				fputs("P6\n", F);
				fputs("# Created by TSE FISA IPSI\n", F);
				sprintf(tmp, "%d %d %d\n", img.Nbcol, img.Nblig, 255);
				fputs(tmp, F);

				/* sauvegarde par bloc */
				fwrite(img.data, sizeof(RGB), img.Nbcol*img.Nblig, F);
				fclose(F);
			}
	}
	else
		printf("Image vide\n");
}

IMAGE inverseImage(IMAGE img)
{
	IMAGE out = { 0,0,NULL,NULL };
	int* lut = NULL;
	int i;

	lut = (int*)calloc(256, sizeof(int)); /* LUT pour optimiser */
	for (i = 0; i<256; i++)
		lut[i] = 255 - i;

	out = allocationImage(img.Nblig, img.Nbcol);
	for (i = 0; i<img.Nbcol*img.Nblig; i++)
		out.data[i] = lut[img.data[i]];

	free(lut);

	return out;
}

IMAGE seuillageImage(IMAGE img, int sInf, int sSup)
{
	IMAGE out = { 0,0,NULL,NULL };
	int* lut = NULL;
	int i;

	lut = (int*)calloc(256, sizeof(int));
	for (i = sInf; i <= sSup; i++)
		lut[i] = 255;

	out = allocationImage(img.Nblig, img.Nbcol);
	for (i = 0; i<img.Nbcol*img.Nblig; i++)
		out.data[i] = lut[img.data[i]];

	free(lut);

	return out;
}


IMAGE planImage(IMAGERGB img, int plan)
{
	IMAGE out = { 0,0,NULL,NULL };
	int i;

	out = allocationImage(img.Nblig, img.Nbcol);
	switch (plan)
	{
	case 0: /* plan rouge */
		for (i = 0; i<out.Nbcol*out.Nblig; i++)
			out.data[i] = img.data[i].R;
		break;
	case 1: /* plan vert */
		for (i = 0; i<out.Nbcol*out.Nblig; i++)
			out.data[i] = img.data[i].G;
		break;
	case 2: /* plan bleu */
		for (i = 0; i<out.Nbcol*out.Nblig; i++)
			out.data[i] = img.data[i].B;
		break;
	}
	return(out);
}

IMAGE luminanceImage(IMAGERGB img, float r, float g, float b)
{
	IMAGE out = { 0,0,NULL,NULL };
	int i;

	out = allocationImage(img.Nblig, img.Nbcol);

	for (i = 0; i<out.Nbcol*out.Nblig; i++)
		out.data[i] = (unsigned char)((float)img.data[i].R*r + (float)img.data[i].G*g + (float)img.data[i].B*b);
	return(out);
}

int* histogrammeImage(IMAGE img, int choix, int n)
// n nombre de niveaux de gris, mise � jour dans les outils en cons�quence
{
	int* h = NULL;

	if (img.data)
	{
		int i;
		int bins = n;
		int diviseur = 256 / n;

		h = (int*)calloc(bins, sizeof(int));

		for (i = 0; i<img.Nbcol*img.Nblig; i++)
			h[(int)img.data[i] / diviseur] += 1;

		if (choix)
		{
			/* sauvegarde dans histo.csv */
			FILE *F;

			F = fopen("res/histo.csv", "w");
			int i;
			for (i = 0; i < bins; i++)
				fprintf(F, "%d ; %d\n", i, h[i]);
			fclose(F);
		}
	}

	return h;
}

int* histogrammeImageRGB(IMAGERGB img, int choix, int n)
{
	int* h = NULL; /* RGB pas possible car UC, donc 3xint avec R G B successivement pour chaque niveau */

	if (img.data)
	{
		int i;
		int bins = n;
		int diviseur = 256 / n;

		h = (int*)calloc(3 * bins, sizeof(int));

		for (i = 0; i < img.Nbcol*img.Nblig; i++)
		{
			h[3 * ((int)img.data[i].R / diviseur)] += 1; /* R */
			h[3 * ((int)img.data[i].G / diviseur) + 1] += 1; /* G */
			h[3 * ((int)img.data[i].B / diviseur) + 2] += 1; /* B */
		}

		if (choix)
		{
			/* sauvegarde dans histo.csv */
			FILE *F;

			F = fopen("res/histo.csv", "w");
			int i;
			for (i = 0; i < bins; i++)
				fprintf(F, "%d ; %d; %d; %d\n", i, h[3 * i], h[3 * i + 1], h[3 * i + 2]);
			fclose(F);
		}
	}

	return h;
}


IMAGE expansionImage(IMAGE img, int outMin, int outMax)
{
	IMAGE out = { 0,0,NULL,NULL };
	int* lut = NULL;
	int i;
	float a, b;
	int mini = 255, maxi = 0;

	for (i = 0; i<img.Nblig*img.Nbcol; i++)
	{
		mini = (img.data[i] < mini) ? img.data[i] : mini;
		maxi = (img.data[i] > maxi) ? img.data[i] : maxi;
	}

	a = (float)(outMax - outMin) / ((float)(maxi - mini));
	b = (float)outMin - a * mini;

	lut = (int*)calloc(256, sizeof(int));
	for (i = mini; i <= maxi; i++)
		lut[i] = (int)(a*i + b);

	out = allocationImage(img.Nblig, img.Nbcol);
	for (i = 0; i<img.Nbcol*img.Nblig; i++)
		out.data[i] = lut[img.data[i]];

	free(lut);

	return out;
}

IMAGE egalisationImage(IMAGE img)
{
	IMAGE out = { 0,0,NULL,NULL };
	int* h = histogrammeImage(img, 0, 256);
	int* hC = NULL; 
	int* lut = NULL;
	int i;

	hC = (int*)calloc(256, sizeof(int));
	hC[0] = h[0];
	for (i = 1; i<256; i++)
		hC[i] = hC[i - 1] + h[i];
	free(h);
	h = NULL;

	lut = (int*)calloc(256, sizeof(int));
	for (i = 0; i<256; i++)
		lut[i] = (int)((255 / (float)(img.Nbcol*img.Nblig)) * hC[i]);
	free(hC);
	hC = NULL;

	out = allocationImage(img.Nblig, img.Nbcol);
	for (i = 0; i<img.Nbcol*img.Nblig; i++)
		out.data[i] = lut[img.data[i]];
	free(lut);

	return(out);
}

SIGNATURES statistiquesImage(IMAGE img)
{
	SIGNATURES sig;
	int i;
	double som = 0, som2 = 0;
	int somme, moitPop = (img.Nbcol*img.Nblig) / 2;
	int* h = histogrammeImage(img, 0, 256);

	for (i = 0; i<img.Nbcol*img.Nblig; i++) {
		som += (double)img.data[i];
		som2 += (double)img.data[i] * (double)img.data[i];
	}

	som /= (double)(img.Nblig*img.Nbcol);
	som2 = sqrt(som2 / (double)(img.Nblig*img.Nbcol) - som*som);

	sig.moyenne = som;
	sig.ecartType = som2;

	i = 0;
	while (h[i] == 0)
		i += 1;
	sig.min = i;

	i = 255;
	while (h[i] == 0)
		i -= 1;
	sig.max = i;

	i = 0;
	somme = h[i];
	while (somme < moitPop) {
		i += 1;
		if (i < 256)
			somme += h[i];
	}
	sig.mediane = i;

	free(h);
	h = NULL;

	return sig;
}

IMAGE seuillageOtsu(IMAGE img)
{
	IMAGE out = { 0, 0, NULL, NULL };
	int* h = histogrammeImage(img, 0, 256);
	int* hC = NULL;

	double *tab = NULL;
	int i, seuil;
	double M1, M2;
	double w1;
	int* lut = NULL;
	int min, max;

	/* recherche min et max */
	i = 0;
	while (h[i] == 0)
		i++;
	min = i;
	i = 255;
	while (h[i] == 0)
		i--;
	max = i;

	hC = (int*)calloc(256, sizeof(int));
	hC[0] = h[0];
	for (i = 1; i < 256; i++)
		hC[i] = hC[i - 1] + h[i];

	tab = (double*)calloc(256, sizeof(double));

	/* parcours entre min et max pour �viter divisions par 0 */
	/* initialisation */
	M1 = min;
	seuil = min;
	w1 = (double)hC[min] / (double)(img.Nbcol*img.Nblig);
	M2 = 0;
	for (int i = min + 1; i <= max; i++)
		M2 += (double)h[i] * i;
	M2 /= (double)(hC[max] - h[min]);
	tab[min] = w1*(1 - w1)*(M1 - M2)*(M1 - M2);

	// parcours
	for (int i = min + 1; i < max; i++) {
		M1 = ((double)hC[i - 1] * M1 + (double)h[i] * i) / hC[i];
		M2 = ((double)(hC[255] - hC[i - 1])*M2 - (double)(h[i] * i)) / (double)(hC[255] - hC[i]);
		w1 = (double)hC[i] / (double)(img.Nbcol*img.Nblig);
		tab[i] = w1*(1 - w1)*(M1 - M2)*(M1 - M2);
		if (tab[i] > tab[seuil])
			seuil = i;
	}

	free(h); h = NULL;
	free(hC); hC = NULL;

	free(tab);

	lut = (int*)calloc(256, sizeof(int));
	for (i = seuil; i <= max; i++)
		lut[i] = 255;

	out = allocationImage(img.Nblig, img.Nbcol);
	for (i = 0; i<img.Nbcol*img.Nblig; i++)
		out.data[i] = lut[img.data[i]];

	free(lut);
	return out;
}

/* �tiquetage V8 */
IMAGE labelImage(IMAGE img, int *nbComp)
{
	IMAGE out = { 0,0,NULL,NULL };

	int i, ii, j, jj;

	int k = 1, kk;
	int eps;
	int predNonNul[5];
	int nbPredNonNuls;

	int nbMaxEtiquettes = img.Nblig * img.Nbcol;
	intIMAGE withBords = { 0,0,NULL,NULL };

	withBords = allocationIntImage(img.Nblig + 2, img.Nbcol + 2);

	for (i = 0, ii = 1; i < img.Nblig; i++, ii++) // pure copie 
		for (j = 0, jj = 1; j < img.Nbcol; j++, jj++)
			withBords.pixel[ii][jj] = (int)img.pixel[i][j];

	int *tableEtiquette = NULL;
	tableEtiquette = (int*)calloc(nbMaxEtiquettes, sizeof(int));

	for (kk = 0; kk < nbMaxEtiquettes; kk++) {
		tableEtiquette[kk] = kk;
	}

	for (i = 1; i < withBords.Nblig - 1; i++)
		for (j = 1; j < withBords.Nbcol - 1; j++) {
			if (withBords.pixel[i][j] != 0) {
				// voisinage V8 
				int pred1 = withBords.pixel[i][j - 1];
				int pred2 = withBords.pixel[i - 1][j - 1];
				int pred3 = withBords.pixel[i - 1][j];
				int pred4 = withBords.pixel[i - 1][j + 1];

				nbPredNonNuls = 0;
				if (pred1) predNonNul[nbPredNonNuls++] = pred1;
				if (pred2) predNonNul[nbPredNonNuls++] = pred2;
				if (pred3) predNonNul[nbPredNonNuls++] = pred3;
				if (pred4) predNonNul[nbPredNonNuls++] = pred4;

				if (nbPredNonNuls == 0) {
					withBords.pixel[i][j] = k;
					k++;
				}
				else {
					eps = 0;
					int minPred = tableEtiquette[predNonNul[0]];
					int maxPred = tableEtiquette[predNonNul[0]];
					while (eps < nbPredNonNuls) {
						if (tableEtiquette[predNonNul[eps]] < minPred) minPred = tableEtiquette[predNonNul[eps]];
						if (tableEtiquette[predNonNul[eps]] > maxPred) maxPred = tableEtiquette[predNonNul[eps]];
						eps++;
					}
					if (minPred == maxPred)
						withBords.pixel[i][j] = minPred;
					else {
						withBords.pixel[i][j] = minPred;
						for (eps = 0; eps < nbPredNonNuls; eps++) {
							int beta = predNonNul[eps];
							while (tableEtiquette[beta] != minPred) {
								int m = tableEtiquette[beta];
								tableEtiquette[beta] = minPred;
								beta = m;
							}
						}
					}
				}
			}
		}


	// actualisation de la table d'�quivalence
	for (int kk = 1; kk < k; kk++) {
		int m = kk;
		while (tableEtiquette[m] != m)
			m = tableEtiquette[m];
		tableEtiquette[kk] = m;
	}

	int* etiquettes = (int*)calloc(k + 1, sizeof(int));

	// histo pour rep�rer les trous
	for (int kk = 1; kk<k; kk++)
		etiquettes[tableEtiquette[kk]]++;

	// on remet � jour les index (etiquetage d�finitif hors trou)
	etiquettes[0] = 0;
	int compt = 1;
	for (int kk = 1; kk<k; kk++) {
		if (etiquettes[kk])
			etiquettes[kk] = compt++;
	}

	for (i = 0; i<withBords.Nblig; i++)
		for (j = 0; j<withBords.Nbcol; j++) {
			withBords.pixel[i][j] = etiquettes[tableEtiquette[withBords.pixel[i][j]]];
		}

	free(etiquettes);
	free(tableEtiquette);

	out = allocationImage(img.Nblig, img.Nbcol);

	*nbComp = compt - 1;
	for (i = 0, ii = 1; i < img.Nblig; i++, ii++)
		for (j = 0, jj = 1; j < img.Nbcol; j++, jj++)
			out.pixel[i][j] = (unsigned char)withBords.pixel[ii][jj];

	liberationIntImage(&withBords);

	return out;
}

IMAGERGB colorisationImage(IMAGE img, char *table)
{
	FILE *lut;
	IMAGERGB out = { 0,0,NULL,NULL };
	char ligne[255];

	int i;
	RGB *LUT = NULL;

	lut = fopen(table, "r");

	LUT = (RGB*)calloc(256, sizeof(RGB));

	while (fgets(ligne, 255, lut) != NULL)
	{
		int a, b, c, d;

		sscanf(ligne, "%d %d %d %d", &a, &b, &c, &d);
		LUT[a].R = b;
		LUT[a].G = c;
		LUT[a].B = d;
	}

	fclose(lut);

	out = allocationImageRGB(img.Nblig, img.Nbcol);

	for (i = 0; i < img.Nblig*img.Nbcol; i++)
	{
		out.data[i].R = LUT[img.data[i]].R;
		out.data[i].G = LUT[img.data[i]].G;
		out.data[i].B = LUT[img.data[i]].B;
	}
	free(LUT);

	return out;
}

SIGNATURE_COMPOSANTE_CONNEXE* signaturesImage(IMAGE label, int nbComp)
{
	SIGNATURE_COMPOSANTE_CONNEXE *sig = NULL;
	IMAGE contours = { 0,0,NULL,NULL };
	int i, j;

	sig = (SIGNATURE_COMPOSANTE_CONNEXE*)calloc(nbComp + 1, sizeof(SIGNATURE_COMPOSANTE_CONNEXE)); /* 0 -> fond */

	for (i = 0; i < label.Nbcol*label.Nblig; i++)
		sig[label.data[i]].surface += 1;

	contours = erosionImage(label, 4);

	for (i = 0; i < label.Nbcol*label.Nblig; i++)
		contours.data[i] = label.data[i] - contours.data[i];

	for (i = 0; i < label.Nbcol*label.Nblig; i++)
		sig[contours.data[i]].perimetre += 1;

	for (i = 1; i <= nbComp; i++)
		sig[i].compacite = (sig[i].perimetre*sig[i].perimetre) / (4 * (float)acos(-1) *sig[i].surface);

	for (i = 0; i < label.Nblig; i++)
	{
		sig[label.pixel[i][0]].bord = 1;
		sig[label.pixel[i][label.Nbcol - 1]].bord = 1;
	}
	for (j = 0; j < label.Nbcol; j++)
	{
		sig[label.pixel[0][j]].bord = 1;
		sig[label.pixel[label.Nblig - 1][j]].bord = 1;
	}

	liberationImage(&contours);

	return sig;
}

IMAGE erosionImage(IMAGE img, int voisinage)
{
	IMAGE out = { 0,0,NULL,NULL };
	int i, j;

	out = allocationImage(img.Nblig, img.Nbcol);

	if (voisinage == 4)
	{
		/* premi�re ligne */
		out.pixel[0][0] = min(img.pixel[0][0], min(img.pixel[0][1], img.pixel[1][0]));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[0][j] = min(min(img.pixel[0][j], img.pixel[1][j]), min(img.pixel[0][j - 1], img.pixel[0][j + 1]));
		out.pixel[0][img.Nbcol - 1] = min(img.pixel[0][img.Nbcol - 2], min(img.pixel[0][img.Nbcol - 1], img.pixel[1][img.Nbcol - 1]));

		/* lignes centrales */
		for (i = 1; i < img.Nblig - 1; i++)
		{
			/* premi�re colonne */
			out.pixel[i][0] = min(min(img.pixel[i][0], img.pixel[i][1]), min(img.pixel[i - 1][0], img.pixel[i + 1][0]));
			/* coeur */
			for (j = 1; j < img.Nbcol - 1; j++)
				out.pixel[i][j] = min(img.pixel[i][j], min(min(img.pixel[i][j - 1], img.pixel[i][j + 1]), min(img.pixel[i - 1][j], img.pixel[i + 1][j])));
			/* derni�re colonne */
			out.pixel[i][img.Nbcol - 1] = min(min(img.pixel[i][img.Nbcol - 2], img.pixel[i][img.Nbcol - 1]), min(img.pixel[i - 1][img.Nbcol - 1], img.pixel[i + 1][img.Nbcol - 1]));
		}

		/* derni�re ligne */
		out.pixel[img.Nblig - 1][0] = min(img.pixel[img.Nblig - 1][0], min(img.pixel[img.Nblig - 2][0], img.pixel[img.Nblig - 1][1]));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[img.Nblig - 1][j] = min(min(img.pixel[img.Nblig - 1][j], img.pixel[img.Nblig - 2][j]), min(img.pixel[img.Nblig - 1][j - 1], img.pixel[img.Nblig - 1][j + 1]));
		out.pixel[img.Nblig - 1][img.Nbcol - 1] = min(img.pixel[img.Nblig - 1][img.Nbcol - 2], min(img.pixel[img.Nblig - 1][img.Nbcol - 1], img.pixel[img.Nblig - 2][img.Nbcol - 1]));
	}
	else
	{
		/* premi�re ligne */
		out.pixel[0][0] = min(img.pixel[1][1], min(img.pixel[0][0], min(img.pixel[0][1], img.pixel[1][0])));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[0][j] = min(min(min(img.pixel[0][j], img.pixel[1][j]), min(img.pixel[0][j - 1], img.pixel[0][j + 1])), min(img.pixel[1][j - 1], img.pixel[1][j + 1]));
		out.pixel[0][img.Nbcol - 1] = min(img.pixel[1][img.Nbcol - 2], min(img.pixel[0][img.Nbcol - 2], min(img.pixel[0][img.Nbcol - 1], img.pixel[1][img.Nbcol - 1])));

		/* lignes centrales */
		for (i = 1; i < img.Nblig - 1; i++)
		{
			/* premi�re colonne */
			out.pixel[i][0] = min(min(img.pixel[i - 1][1], img.pixel[i + 1][1]), min(min(img.pixel[i][0], img.pixel[i][1]), min(img.pixel[i - 1][0], img.pixel[i + 1][0])));
			/* coeur */
			for (j = 1; j < img.Nbcol - 1; j++)
				out.pixel[i][j] = min(min(min(img.pixel[i - 1][j - 1], img.pixel[i + 1][j + 1]), min(img.pixel[i - 1][j + 1], img.pixel[i + 1][j - 1])), min(img.pixel[i][j], min(min(img.pixel[i][j - 1], img.pixel[i][j + 1]), min(img.pixel[i - 1][j], img.pixel[i + 1][j]))));
			/* derni�re colonne */
			out.pixel[i][img.Nbcol - 1] = min(min(img.pixel[i - 1][img.Nbcol - 2], img.pixel[i + 1][img.Nbcol - 2]), min(min(img.pixel[i][img.Nbcol - 2], img.pixel[i][img.Nbcol - 1]), min(img.pixel[i - 1][img.Nbcol - 1], img.pixel[i + 1][img.Nbcol - 1])));
		}

		/* derni�re ligne */
		out.pixel[img.Nblig - 1][0] = min(img.pixel[img.Nblig - 2][1], min(img.pixel[img.Nblig - 1][0], min(img.pixel[img.Nblig - 2][0], img.pixel[img.Nblig - 1][1])));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[img.Nblig - 1][j] = min(min(img.pixel[img.Nblig - 2][j - 1], img.pixel[img.Nblig - 2][j + 1]), min(min(img.pixel[img.Nblig - 1][j], img.pixel[img.Nblig - 2][j]), min(img.pixel[img.Nblig - 1][j - 1], img.pixel[img.Nblig - 1][j + 1])));
		out.pixel[img.Nblig - 1][img.Nbcol - 1] = min(img.pixel[img.Nblig - 2][img.Nbcol - 2], min(img.pixel[img.Nblig - 1][img.Nbcol - 2], min(img.pixel[img.Nblig - 1][img.Nbcol - 1], img.pixel[img.Nblig - 2][img.Nbcol - 1])));
	}
	return out;
}

IMAGE dilatationImage(IMAGE img, int voisinage)
{
	IMAGE out = { 0,0,NULL,NULL };
	int i, j;

	out = allocationImage(img.Nblig, img.Nbcol);

	if (voisinage == 4)
	{
		/* premi�re ligne */
		out.pixel[0][0] = max(img.pixel[0][0], max(img.pixel[0][1], img.pixel[1][0]));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[0][j] = max(max(img.pixel[0][j], img.pixel[1][j]), max(img.pixel[0][j - 1], img.pixel[0][j + 1]));
		out.pixel[0][img.Nbcol - 1] = max(img.pixel[0][img.Nbcol - 2], max(img.pixel[0][img.Nbcol - 1], img.pixel[1][img.Nbcol - 1]));

		/* lignes centrales */
		for (i = 1; i < img.Nblig - 1; i++)
		{
			/* premi�re colonne */
			out.pixel[i][0] = max(max(img.pixel[i][0], img.pixel[i][1]), max(img.pixel[i - 1][0], img.pixel[i + 1][0]));
			/* coeur */
			for (j = 1; j < img.Nbcol - 1; j++)
				out.pixel[i][j] = max(img.pixel[i][j], max(max(img.pixel[i][j - 1], img.pixel[i][j + 1]), max(img.pixel[i - 1][j], img.pixel[i + 1][j])));
			/* derni�re colonne */
			out.pixel[i][img.Nbcol - 1] = max(max(img.pixel[i][img.Nbcol - 2], img.pixel[i][img.Nbcol - 1]), max(img.pixel[i - 1][img.Nbcol - 1], img.pixel[i + 1][img.Nbcol - 1]));
		}

		/* derni�re ligne */
		out.pixel[img.Nblig - 1][0] = max(img.pixel[img.Nblig - 1][0], max(img.pixel[img.Nblig - 2][0], img.pixel[img.Nblig - 1][1]));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[img.Nblig - 1][j] = max(max(img.pixel[img.Nblig - 1][j], img.pixel[img.Nblig - 2][j]), max(img.pixel[img.Nblig - 1][j - 1], img.pixel[img.Nblig - 1][j + 1]));
		out.pixel[img.Nblig - 1][img.Nbcol - 1] = max(img.pixel[img.Nblig - 1][img.Nbcol - 2], max(img.pixel[img.Nblig - 1][img.Nbcol - 1], img.pixel[img.Nblig - 2][img.Nbcol - 1]));
	}
	else
	{
		/* premi�re ligne */
		out.pixel[0][0] = max(img.pixel[1][1], max(img.pixel[0][0], max(img.pixel[0][1], img.pixel[1][0])));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[0][j] = max(max(max(img.pixel[0][j], img.pixel[1][j]), max(img.pixel[0][j - 1], img.pixel[0][j + 1])), max(img.pixel[1][j - 1], img.pixel[1][j + 1]));
		out.pixel[0][img.Nbcol - 1] = max(img.pixel[1][img.Nbcol - 2], max(img.pixel[0][img.Nbcol - 2], max(img.pixel[0][img.Nbcol - 1], img.pixel[1][img.Nbcol - 1])));

		/* lignes centrales */
		for (i = 1; i < img.Nblig - 1; i++)
		{
			/* premi�re colonne */
			out.pixel[i][0] = max(max(img.pixel[i - 1][1], img.pixel[i + 1][1]), max(max(img.pixel[i][0], img.pixel[i][1]), max(img.pixel[i - 1][0], img.pixel[i + 1][0])));
			/* coeur */
			for (j = 1; j < img.Nbcol - 1; j++)
				out.pixel[i][j] = max(max(max(img.pixel[i - 1][j - 1], img.pixel[i + 1][j + 1]), max(img.pixel[i - 1][j + 1], img.pixel[i + 1][j - 1])), max(img.pixel[i][j], max(max(img.pixel[i][j - 1], img.pixel[i][j + 1]), max(img.pixel[i - 1][j], img.pixel[i + 1][j]))));
			/* derni�re colonne */
			out.pixel[i][img.Nbcol - 1] = max(max(img.pixel[i - 1][img.Nbcol - 2], img.pixel[i + 1][img.Nbcol - 2]), max(max(img.pixel[i][img.Nbcol - 2], img.pixel[i][img.Nbcol - 1]), max(img.pixel[i - 1][img.Nbcol - 1], img.pixel[i + 1][img.Nbcol - 1])));
		}

		/* derni�re ligne */
		out.pixel[img.Nblig - 1][0] = max(img.pixel[img.Nblig - 2][1], max(img.pixel[img.Nblig - 1][0], max(img.pixel[img.Nblig - 2][0], img.pixel[img.Nblig - 1][1])));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[img.Nblig - 1][j] = max(max(img.pixel[img.Nblig - 2][j - 1], img.pixel[img.Nblig - 2][j + 1]), max(max(img.pixel[img.Nblig - 1][j], img.pixel[img.Nblig - 2][j]), max(img.pixel[img.Nblig - 1][j - 1], img.pixel[img.Nblig - 1][j + 1])));
		out.pixel[img.Nblig - 1][img.Nbcol - 1] = max(img.pixel[img.Nblig - 2][img.Nbcol - 2], max(img.pixel[img.Nblig - 1][img.Nbcol - 2], max(img.pixel[img.Nblig - 1][img.Nbcol - 1], img.pixel[img.Nblig - 2][img.Nbcol - 1])));
	}
	return out;
}

IMAGE fermetureImage(IMAGE img, int voisinage)
{
	IMAGE out = { 0,0,NULL,NULL };
	IMAGE inter = { 0,0,NULL,NULL };

	inter = dilatationImage(img, voisinage);
	out = erosionImage(inter, voisinage);
	liberationImage(&inter);

	return out;
}

IMAGE ouvertureImage(IMAGE img, int voisinage)
{
	IMAGE out = { 0,0,NULL,NULL };
	IMAGE inter = { 0,0,NULL,NULL };

	inter = erosionImage(img, voisinage);
	out = dilatationImage(inter, voisinage);
	liberationImage(&inter);

	return out;
}

STRUCTURE_ELEMENT strelDisk(int dim)
{
	STRUCTURE_ELEMENT se = allocationStructureElement(dim, dim);
	int i, j;
	int rayon = dim / 2;
	int rayon2 = rayon * rayon;
	int x, y;
	for (i = 0; i < dim; i++)
	{
		for (j = 0; j < dim; j++)
		{
			x = i - rayon;
			y = j - rayon;
			if (x * x + y * y <= rayon2)
			{
				se.data[i][j] = 1;
			}
		}
	}
	return se;
}

STRUCTURE_ELEMENT strelSquare(int dim)
{
	STRUCTURE_ELEMENT se = allocationStructureElement(dim, dim);
	int i, j;
	for (i = 0; i < dim; i++)
	{
		for (j = 0; j < dim; j++)
		{
			se.data[i][j] = 1;
		}
	}
	return se;
}

STRUCTURE_ELEMENT strelLine(int dim)
{
	STRUCTURE_ELEMENT se = allocationStructureElement(dim, dim);
	int i, j;
	for (i = 0; i < dim; i++)
	{
		for (j = 0; j < dim; j++)
		{
			se.data[i][j] = 1;
		}
	}
	return se;
}

STRUCTURE_ELEMENT strel(char *nom, int dim1)
{
	if (strcmp(nom, "disk") == 0)
	{
		return strelDisk(dim1);
	}
	else if (strcmp(nom, "square") == 0)
	{
		return strelSquare(dim1);
	}
	else if (strcmp(nom, "line") == 0)
	{
		return strelLine(dim1);
	}
	else
	{
		printf("Erreur : strel() : nom de structure element inconnu\n");
		exit(1);
	}
}

STRUCTURE_ELEMENT allocationStructureElement(int Nblig, int Nbcol)
{
	STRUCTURE_ELEMENT se = { 0,0,NULL };
	int i;

	se.Nblig = Nblig;
	se.Nbcol = Nbcol;
	se.data = (int**)malloc(se.Nblig * sizeof(int*));
	for (i = 0; i < se.Nblig; i++)
	{
		se.data[i] = (int*)malloc(se.Nbcol * sizeof(int));
	}
	return se;
}

void liberationStructureElement(STRUCTURE_ELEMENT *se)
{
	int i;

	for (i = 0; i < se->Nblig; i++)
	{
		free(se->data[i]);
	}
	free(se->data);
	se->Nblig = 0;
	se->Nbcol = 0;	
}

int dilatationPixel(IMAGE img, int i, int j, STRUCTURE_ELEMENT se)
{
	int k, l, max = 0;

	for (k = 0; k < se.Nblig; k++)
	{
		for (l = 0; l < se.Nbcol; l++)
		{
			if (se.data[k][l] == 1)
			{
				if (img.pixel[i + k][j + l] > max)
				{
					max = img.pixel[i + k][j + l];
				}
			}
		}
	}
	return max;
}

IMAGE dilatationImageSE(IMAGE img, STRUCTURE_ELEMENT se)
{
	IMAGE out = { 0,0,NULL,NULL };
	int i, j;

	out = allocationImage(img.Nblig, img.Nbcol);

	for (i = 0; i < img.Nblig; i++)
	{
		for (j = 0; j < img.Nbcol; j++)
		{
			out.pixel[i][j] = dilatationPixel(img, i, j, se);
		}
	}
	return out;
}

int erosionPixel(IMAGE img, int i, int j, STRUCTURE_ELEMENT se)
{
	int k, l, min = 255;

	for (k = 0; k < se.Nblig; k++)
	{
		for (l = 0; l < se.Nbcol; l++)
		{
			if (se.data[k][l] == 1)
			{
				if (img.pixel[i + k][j + l] < min)
				{
					min = img.pixel[i + k][j + l];
				}
			}
		}
	}
	return min;
}

IMAGE erosionImageSE(IMAGE img, STRUCTURE_ELEMENT se)
{
	IMAGE out = { 0,0,NULL,NULL };
	int i, j;

	out = allocationImage(img.Nblig, img.Nbcol);

	for (i = 0; i < img.Nblig; i++)
	{
		for (j = 0; j < img.Nbcol; j++)
		{
			out.pixel[i][j] = erosionPixel(img, i, j, se);
		}
	}
	return out;
}

IMAGE ouvertureImageSE(IMAGE img, STRUCTURE_ELEMENT se)
{
	IMAGE out = { 0,0,NULL,NULL };
	IMAGE inter = { 0,0,NULL,NULL };

	inter = erosionImageSE(img, se);
	out = dilatationImageSE(inter, se);
	liberationImage(&inter);

	return out;
}

IMAGE fermetureImageSE(IMAGE img, STRUCTURE_ELEMENT se)
{
	IMAGE out = { 0,0,NULL,NULL };
	IMAGE inter = { 0,0,NULL,NULL };

	inter = dilatationImageSE(img, se);
	out = erosionImageSE(inter, se);
	liberationImage(&inter);

	return out;
}

char* lectureString(int taille)
{
	char* mot;
	mot = (char*)malloc(taille * sizeof(char));

	scanf("%s", mot);

	return mot;
}
