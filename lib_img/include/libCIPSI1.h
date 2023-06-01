#pragma once

/* librairie IPSI1 */

typedef struct image {
	int   Nblig;
	int   Nbcol;
	unsigned char*  data;
	unsigned char** pixel;
} IMAGE;

typedef struct structureElement {
	int   Nblig;
	int   Nbcol;
	int** data;
} STRUCTURE_ELEMENT;

typedef struct Region
{
    int x1;
    int y1;
    int x2;
    int y2;
} REGION;

typedef struct rgb {
	unsigned char R;
	unsigned char G;
	unsigned char B;
} RGB;

typedef struct imageRGB {
	int   Nblig;
	int   Nbcol;
	RGB*  data;
	RGB** pixel;
} IMAGERGB;

typedef struct signatures {
	int		min;
	int		max;
	double	moyenne;
	double	ecartType;
	int		mediane;
} SIGNATURES;

typedef struct signature_composantes_connexes {
	int surface;
	float perimetre;
	float compacite;
	int bord;
} SIGNATURE_COMPOSANTE_CONNEXE;

float scoreIOU(IMAGE im1, IMAGE im2);
REGION* findRegions(IMAGE im, int* nbRegions);
void expandRegion(IMAGE im, REGION* region);
float computeOverlap(REGION r1, REGION r2);
float scoreVinet(IMAGE im1, IMAGE im2);

IMAGE allocationImage(int Nblig, int Nbcol);
void initialisationAleatoireImage(IMAGE img, int ngMin, int ngMax);

void sauvegardeImage(IMAGE img, const char *type, const char *out);
void liberationImage(IMAGE *im);

IMAGE lectureImage(const char *nom);

IMAGERGB allocationImageRGB(int Nblig, int Nbcol);
IMAGERGB lectureImageRGB(const char *nom);

void sauvegardeImageRGB(IMAGERGB img, const char *type, const char *out);
void liberationImageRGB(IMAGERGB *im);

IMAGE planImage(IMAGERGB img, int plan);
IMAGE luminanceImage(IMAGERGB img, float r, float g, float b);
IMAGE inverseImage(IMAGE img);
IMAGE seuillageImage(IMAGE img, int sInf, int sSup);

IMAGE expansionImage(IMAGE img, int outMin, int outMax);
IMAGE egalisationImage(IMAGE img);
SIGNATURES statistiquesImage(IMAGE img);

IMAGE seuillageOtsu(IMAGE img);

IMAGE labelImage(IMAGE img, int *nbComp); /* image label + nb composantes connexes */

int* histogrammeImage(IMAGE img, int choix, int n);
int* histogrammeImageRGB(IMAGERGB img, int choix, int n);
IMAGERGB colorisationImage(IMAGE img, char *table);

SIGNATURE_COMPOSANTE_CONNEXE* signaturesImage(IMAGE img, int nbComp);
IMAGE dilatationImage(IMAGE img, int voisinage);
IMAGE erosionImage(IMAGE img, int voisinage);
IMAGE ouvertureImage(IMAGE img, int voisinage);
IMAGE fermetureImage(IMAGE img, int voisinage);

STRUCTURE_ELEMENT strelDisk(int dim);
STRUCTURE_ELEMENT strelSquare(int dim);
STRUCTURE_ELEMENT strelLine(int dim);
STRUCTURE_ELEMENT strel(char *nom, int dim1);
STRUCTURE_ELEMENT allocationStructureElement(int Nblig, int Nbcol);
void liberationStructureElement(STRUCTURE_ELEMENT *se);

IMAGE filtreMed(IMAGE img, int dim);
int dilatationPixel(IMAGE img, int i, int j, STRUCTURE_ELEMENT se);
IMAGE dilatationImageSE(IMAGE img, STRUCTURE_ELEMENT se);
int erosionPixel(IMAGE img, int i, int j, STRUCTURE_ELEMENT se);
IMAGE erosionImageSE(IMAGE img, STRUCTURE_ELEMENT se);
IMAGE ouvertureImageSE(IMAGE img, STRUCTURE_ELEMENT se);
IMAGE fermetureImageSE(IMAGE img, STRUCTURE_ELEMENT se);

char* lectureString(int taille);