// libCIPSI1.c
// Hubert Konik - filiére IPSI

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "../include/libCIPSI1.h"

#define PI acos(-1)

/* static -> non extern */

int min(int a, int b)
{
	return (a < b) ? a : b;
}

int max(int a, int b)
{
	return (a > b) ? a : b;
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
	mat.data = (unsigned char*)malloc(Nblig*Nbcol*sizeof(unsigned char));
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

int* histogrammeImage(IMAGE img, int choix, int nbNiveaux)
{
    //n : puissance (pas de quantification) 2^n
    // Avec 8 nous avons touts les niveau de gris 
    int n;

    if (nbNiveaux == 0) {
        n = 256;
    }
    n = log(nbNiveaux) / log(2);

    int* h = NULL;

    if (img.data)
    {
        int i;
        int bins = 256 >> (8 - n);

        h = (int*)calloc(bins, sizeof(int));

        for (i = 0; i < img.Nbcol * img.Nblig; i++)
            h[((int)img.data[i]) >> (8 - n)] += 1;

        if (choix)
        {
            /* sauvegarde dans histo.csv */
            FILE* F;

            F = fopen("histo.csv", "w");
            int i;
            for (i = 0; i < bins; i++)
                fprintf(F, "%d ; %d\n", i, h[i]);
            fclose(F);
        }
    }
    return h;
}

int* histogrammeImageRGB(IMAGERGB img, int choix, int nbNiveaux)
{
	int* h = NULL; /* RGB pas possible car UC, donc 3xint avec R G B successivement pour chaque niveau */

	int n;

	if (nbNiveaux == 0) {
        n = 256;
    }
    n = log(nbNiveaux) / log(2);

	if (img.data)
	{
		int i;
		int bins = 256 >> (8 - n);

		h = (int*)calloc(3*bins, sizeof(int));

		for (i = 0; i < img.Nbcol*img.Nblig; i++)
		{
			h[3*(((int)img.data[i].R) >> (8 - n))] += 1; /* R */
			h[3*(((int)img.data[i].G) >> (8 - n))+1] += 1; /* G */
			h[3*(((int)img.data[i].B) >> (8 - n))+2] += 1; /* B */
		}

		if (choix)
		{
			/* sauvegarde dans histo.csv */
			FILE *F;

			F = fopen("histoRGB.csv", "w");
			int i;
			for (i = 0; i < bins; i++)
				fprintf(F, "%d ; %d; %d; %d\n", i, h[3*i],h[3*i+1],h[3*i+2]);
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

IMAGERGB colorisationImage(IMAGE img, char *table)
{
	char ligne[255]; 
    int index , r=0 , g=0, b=0;
	char path[255] = "./lib_img/data/";
	strcat(path, table);
    FILE* lut = fopen(path, "r");
    int i = 0;
    RGB* LUT = (RGB*)calloc(256, sizeof(RGB));

    while (fgets(ligne, 255, lut) != NULL)
    {
        sscanf(ligne, "%d %d %d %d", &index , &r , &g, &b);
        LUT[index].R = r;
        LUT[index].G = g;
        LUT[index].B = b;

    }
    fclose(lut);

    IMAGERGB imgRGB = { 0 , 0 , NULL , NULL };
    imgRGB = allocationImageRGB(img.Nblig, img.Nbcol);
    int valeur;
    for (i = 0; i < (img.Nblig * img.Nbcol); i++) {
        valeur = img.data[i];
        imgRGB.data[i].B = LUT[valeur].B;
        imgRGB.data[i].G = LUT[valeur].G;
        imgRGB.data[i].R = LUT[valeur].R;
    }
	free(LUT);

    return imgRGB;
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

IMAGE bruitAleatoireImage(IMAGE img, int amplitude)
{
	IMAGE out = { 0,0,NULL,NULL };
	int tmp;

	out = allocationImage(img.Nblig, img.Nbcol);

	for (int i = 0; i < img.Nblig; i++)
		for (int j = 0; j < img.Nbcol; j++)
		{
			// Using an int to avoid overflow
			tmp = img.pixel[i][j] + rand() % (2 * amplitude) - amplitude;
			if (tmp < 0)
				tmp = 0;
			if (tmp > 255)
				tmp = 255;
			out.pixel[i][j] = (unsigned char)tmp;
		}

	return out;
}

IMAGERGB bruitAleatoireImageRGB(IMAGERGB img, int amplitude)
{
	IMAGERGB out = { 0,0,NULL,NULL };
	int tmp;

	out = allocationImageRGB(img.Nblig, img.Nbcol);

	for (int i = 0; i < img.Nblig; i++)
		for (int j = 0; j < img.Nbcol; j++)
		{
			tmp = img.pixel[i][j].R + rand() % (2 * amplitude) - amplitude;
			if (tmp < 0)
				tmp = 0;
			if (tmp > 255)
				tmp = 255;
			out.pixel[i][j].R = (unsigned char)tmp;

			tmp = img.pixel[i][j].G + rand() % (2 * amplitude) - amplitude;
			if (tmp < 0)
				tmp = 0;
			if (tmp > 255)
				tmp = 255;
			out.pixel[i][j].G = (unsigned char)tmp;

			tmp = img.pixel[i][j].B + rand() % (2 * amplitude) - amplitude;
			if (tmp < 0)
				tmp = 0;
			if (tmp > 255)
				tmp = 255;
			out.pixel[i][j].B = (unsigned char)tmp;
		}

	return out;
}

IMAGERGB masqueImage(IMAGE img, IMAGERGB masque)
{
	IMAGERGB out = { 0,0,NULL,NULL };
	int tmp;

	out = allocationImageRGB(img.Nblig, img.Nbcol);

	for (int i = 0; i < img.Nblig; i++)
		for (int j = 0; j < img.Nbcol; j++)
		{
			if (masque.pixel[i][j].R == 0 && masque.pixel[i][j].G == 0 && masque.pixel[i][j].B == 0)
			{
				out.pixel[i][j].R = img.pixel[i][j];
				out.pixel[i][j].G = img.pixel[i][j];
				out.pixel[i][j].B = img.pixel[i][j];
			}
			else
			{
				out.pixel[i][j].R = masque.pixel[i][j].R;
				out.pixel[i][j].G = masque.pixel[i][j].G;
				out.pixel[i][j].B = masque.pixel[i][j].B;
			}
		}

	return out;	 
}

void lsqe(const float *x, const float *y, int n, float *a, float *b)
{
	float somme_x,somme_y,somme_x2,somme_xy;
	int i;

	somme_x = 0.;
	somme_y = 0.;
	somme_x2 = 0.;
	somme_xy = 0.;

	for (i=0;i<n;i++) {
		somme_x += x[i];
		somme_y += y[i];
		somme_x2 += x[i]*x[i];
		somme_xy += (x[i]*y[i]);
	}

	*a = (somme_xy - ((somme_x*somme_y)/(float)n))/(somme_x2-((somme_x*somme_x)/(float)n));
	*b = (somme_y - ((*a)*somme_x))/(float)n;
}

void regression(POINT *tab, int n, float *a, float *b)
{
	float *x = (float*)malloc(n * sizeof(float));
	float *y = (float*)malloc(n * sizeof(float));

	for (int i = 0; i < n; i++)
	{
		x[i] = tab[i].x;
		y[i] = tab[i].y;
	}

	lsqe(x, y, n, a, b);

	free(x);
	free(y);
}

POINT* imageVersPoints(IMAGE img, int *n, char axe)
{
	POINT *out = (POINT*)malloc(img.Nblig * img.Nbcol * sizeof(POINT));
	int i, j, k = 0;

	if (axe == 'i')
	{
		for (i = 0; i < img.Nblig; i++)
			for (j = 0; j < img.Nbcol; j++)
			{
				out[k].x = j;
				out[k].y = img.pixel[i][j];
				k++;
			}
	}
	else if (axe == 'j')
	{
		for (i = 0; i < img.Nblig; i++)
			for (j = 0; j < img.Nbcol; j++)
			{
				out[k].x = img.pixel[i][j];
				out[k].y = j;
				k++;
			}
	}

	*n = k;

	out = (POINT*)realloc(out, *n * sizeof(POINT));

	return out;
}

SIGNATURE_COMPOSANTE_CONNEXE* signaturesImage(IMAGE label, int nbComp)
{
	SIGNATURE_COMPOSANTE_CONNEXE *sig = NULL;
	int n = label.Nbcol*label.Nblig;
	POINT* pts = (POINT*) calloc (n, sizeof(POINT));
	pts = imageVersPoints(label, &n, 'i');
	IMAGE contours = { 0,0,NULL,NULL };
	POINT tmp = { 0,0 };
	int i, j;

	sig = (SIGNATURE_COMPOSANTE_CONNEXE*)calloc(nbComp + 1, sizeof(SIGNATURE_COMPOSANTE_CONNEXE)); /* 0 -> fond */

	for (i = 0; i < label.Nbcol*label.Nblig; i++)
		sig[label.data[i]].surface += 1;

	contours = erosionImage(label, 4);

	for (i = 0; i < label.Nbcol*label.Nblig; i++)
		contours.data[i] = label.data[i] - contours.data[i];

	for (i = 0; i < label.Nblig; i++)
		for (j = 0; j < label.Nbcol; j++)
			if (contours.pixel[i][j] != 0)
			{
				tmp.x = i;
				tmp.y = j;
				break;
			}

	// Compute perimeter of each component by going from a point on the contour to the next one
	for (i = 0; i < label.Nblig; i++)
		for (j = 0; j < label.Nbcol; j++)
			if (contours.pixel[i][j] != 0)
			{
				if (tmp.x == i || tmp.y == j)
					sig[contours.pixel[i][j]].perimetre += 1;
				else
					sig[contours.pixel[i][j]].perimetre += sqrt(2);
				tmp.x = i;
				tmp.y = j;
			}

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

	for (i = 1; i <= nbComp; i++)
		regression(pts, n, &sig[i].regA, &sig[i].regB);

	for (i = 1; i <= nbComp; i++)
	{
		sig[i].CG.x /= sig[i].surface;
		sig[i].CG.y /= sig[i].surface;
	}

	liberationImage(&contours);

	return sig;
}

void sauvegardeSignaturesImage(SIGNATURE_COMPOSANTE_CONNEXE* sig, int nbComp, const char* fic)
{
	char path[255] = "./lib_img/data/";
	strcat(path, fic);
	FILE *f = fopen(path, "w");

	fprintf(f, "Surface;Perimetre;Compacite;Bord;Centre de gravite;Rayon;Coefficient A;Coefficient B\n");
	for (int i = 1; i <= nbComp; i++)
		fprintf(f, "%d;%.2f;%.2f;%d;(%d|%d);%.2f;%.2f;%.2f\n",
				sig[i].surface, sig[i].perimetre, sig[i].compacite, sig[i].bord,
			sig[i].CG.x, sig[i].CG.y, sig[i].rayon, sig[i].regA, sig[i].regB);

	fclose(f);
}

IMAGE erosionImage(IMAGE img, int voisinage)
{
	IMAGE out = { 0,0,NULL,NULL };
	int i, j;

	out = allocationImage(img.Nblig, img.Nbcol);

	if (voisinage == 4)
	{
		/* premiËre ligne */
		out.pixel[0][0] = min(img.pixel[0][0], min(img.pixel[0][1], img.pixel[1][0]));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[0][j] = min(min(img.pixel[0][j], img.pixel[1][j]), min(img.pixel[0][j - 1], img.pixel[0][j + 1]));
		out.pixel[0][img.Nbcol - 1] = min(img.pixel[0][img.Nbcol - 2], min(img.pixel[0][img.Nbcol - 1], img.pixel[1][img.Nbcol - 1]));

		/* lignes centrales */
		for (i = 1; i < img.Nblig - 1; i++)
		{
			/* premiËre colonne */
			out.pixel[i][0] = min(min(img.pixel[i][0], img.pixel[i][1]), min(img.pixel[i - 1][0], img.pixel[i + 1][0]));
			/* coeur */
			for (j = 1; j < img.Nbcol - 1; j++)
				out.pixel[i][j] = min(img.pixel[i][j], min(min(img.pixel[i][j - 1], img.pixel[i][j + 1]), min(img.pixel[i - 1][j], img.pixel[i + 1][j])));
			/* derniËre colonne */
			out.pixel[i][img.Nbcol - 1] = min(min(img.pixel[i][img.Nbcol - 2], img.pixel[i][img.Nbcol - 1]), min(img.pixel[i - 1][img.Nbcol - 1], img.pixel[i + 1][img.Nbcol - 1]));
		}

		/* derniËre ligne */
		out.pixel[img.Nblig - 1][0] = min(img.pixel[img.Nblig - 1][0], min(img.pixel[img.Nblig - 2][0], img.pixel[img.Nblig - 1][1]));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[img.Nblig - 1][j] = min(min(img.pixel[img.Nblig - 1][j], img.pixel[img.Nblig - 2][j]), min(img.pixel[img.Nblig - 1][j - 1], img.pixel[img.Nblig - 1][j + 1]));
		out.pixel[img.Nblig - 1][img.Nbcol - 1] = min(img.pixel[img.Nblig - 1][img.Nbcol - 2], min(img.pixel[img.Nblig - 1][img.Nbcol - 1], img.pixel[img.Nblig - 2][img.Nbcol - 1]));
	}
	else
	{
		/* premiËre ligne */
		out.pixel[0][0] = min(img.pixel[1][1], min(img.pixel[0][0], min(img.pixel[0][1], img.pixel[1][0])));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[0][j] = min(min(min(img.pixel[0][j], img.pixel[1][j]), min(img.pixel[0][j - 1], img.pixel[0][j + 1])), min(img.pixel[1][j - 1], img.pixel[1][j + 1]));
		out.pixel[0][img.Nbcol - 1] = min(img.pixel[1][img.Nbcol - 2], min(img.pixel[0][img.Nbcol - 2], min(img.pixel[0][img.Nbcol - 1], img.pixel[1][img.Nbcol - 1])));

		/* lignes centrales */
		for (i = 1; i < img.Nblig - 1; i++)
		{
			/* premiËre colonne */
			out.pixel[i][0] = min(min(img.pixel[i - 1][1], img.pixel[i + 1][1]), min(min(img.pixel[i][0], img.pixel[i][1]), min(img.pixel[i - 1][0], img.pixel[i + 1][0])));
			/* coeur */
			for (j = 1; j < img.Nbcol - 1; j++)
				out.pixel[i][j] = min(min(min(img.pixel[i - 1][j - 1], img.pixel[i + 1][j + 1]), min(img.pixel[i - 1][j + 1], img.pixel[i + 1][j - 1])), min(img.pixel[i][j], min(min(img.pixel[i][j - 1], img.pixel[i][j + 1]), min(img.pixel[i - 1][j], img.pixel[i + 1][j]))));
			/* derniËre colonne */
			out.pixel[i][img.Nbcol - 1] = min(min(img.pixel[i - 1][img.Nbcol - 2], img.pixel[i + 1][img.Nbcol - 2]), min(min(img.pixel[i][img.Nbcol - 2], img.pixel[i][img.Nbcol - 1]), min(img.pixel[i - 1][img.Nbcol - 1], img.pixel[i + 1][img.Nbcol - 1])));
		}

		/* derniËre ligne */
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
		/* premiËre ligne */
		out.pixel[0][0] = max(img.pixel[0][0], max(img.pixel[0][1], img.pixel[1][0]));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[0][j] = max(max(img.pixel[0][j], img.pixel[1][j]), max(img.pixel[0][j - 1], img.pixel[0][j + 1]));
		out.pixel[0][img.Nbcol - 1] = max(img.pixel[0][img.Nbcol - 2], max(img.pixel[0][img.Nbcol - 1], img.pixel[1][img.Nbcol - 1]));

		/* lignes centrales */
		for (i = 1; i < img.Nblig - 1; i++)
		{
			/* premiËre colonne */
			out.pixel[i][0] = max(max(img.pixel[i][0], img.pixel[i][1]), max(img.pixel[i - 1][0], img.pixel[i + 1][0]));
			/* coeur */
			for (j = 1; j < img.Nbcol - 1; j++)
				out.pixel[i][j] = max(img.pixel[i][j], max(max(img.pixel[i][j - 1], img.pixel[i][j + 1]), max(img.pixel[i - 1][j], img.pixel[i + 1][j])));
			/* derniËre colonne */
			out.pixel[i][img.Nbcol - 1] = max(max(img.pixel[i][img.Nbcol - 2], img.pixel[i][img.Nbcol - 1]), max(img.pixel[i - 1][img.Nbcol - 1], img.pixel[i + 1][img.Nbcol - 1]));
		}

		/* derniËre ligne */
		out.pixel[img.Nblig - 1][0] = max(img.pixel[img.Nblig - 1][0], max(img.pixel[img.Nblig - 2][0], img.pixel[img.Nblig - 1][1]));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[img.Nblig - 1][j] = max(max(img.pixel[img.Nblig - 1][j], img.pixel[img.Nblig - 2][j]), max(img.pixel[img.Nblig - 1][j - 1], img.pixel[img.Nblig - 1][j + 1]));
		out.pixel[img.Nblig - 1][img.Nbcol - 1] = max(img.pixel[img.Nblig - 1][img.Nbcol - 2], max(img.pixel[img.Nblig - 1][img.Nbcol - 1], img.pixel[img.Nblig - 2][img.Nbcol - 1]));
	}
	else
	{
		/* premiËre ligne */
		out.pixel[0][0] = max(img.pixel[1][1], max(img.pixel[0][0], max(img.pixel[0][1], img.pixel[1][0])));
		for (j = 1; j < img.Nbcol - 1; j++)
			out.pixel[0][j] = max(max(max(img.pixel[0][j], img.pixel[1][j]), max(img.pixel[0][j - 1], img.pixel[0][j + 1])), max(img.pixel[1][j - 1], img.pixel[1][j + 1]));
		out.pixel[0][img.Nbcol - 1] = max(img.pixel[1][img.Nbcol - 2], max(img.pixel[0][img.Nbcol - 2], max(img.pixel[0][img.Nbcol - 1], img.pixel[1][img.Nbcol - 1])));

		/* lignes centrales */
		for (i = 1; i < img.Nblig - 1; i++)
		{
			/* premiËre colonne */
			out.pixel[i][0] = max(max(img.pixel[i - 1][1], img.pixel[i + 1][1]), max(max(img.pixel[i][0], img.pixel[i][1]), max(img.pixel[i - 1][0], img.pixel[i + 1][0])));
			/* coeur */
			for (j = 1; j < img.Nbcol - 1; j++)
				out.pixel[i][j] = max(max(max(img.pixel[i - 1][j - 1], img.pixel[i + 1][j + 1]), max(img.pixel[i - 1][j + 1], img.pixel[i + 1][j - 1])), max(img.pixel[i][j], max(max(img.pixel[i][j - 1], img.pixel[i][j + 1]), max(img.pixel[i - 1][j], img.pixel[i + 1][j]))));
			/* derniËre colonne */
			out.pixel[i][img.Nbcol - 1] = max(max(img.pixel[i - 1][img.Nbcol - 2], img.pixel[i + 1][img.Nbcol - 2]), max(max(img.pixel[i][img.Nbcol - 2], img.pixel[i][img.Nbcol - 1]), max(img.pixel[i - 1][img.Nbcol - 1], img.pixel[i + 1][img.Nbcol - 1])));
		}

		/* derniËre ligne */
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

int distanceEuclidienne(POINT p1, POINT p2)
{
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}
