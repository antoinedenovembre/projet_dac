#ifndef LIBCIPSI1_H
#define LIBCIPSI1_H

/* librairie IPSI1 */

typedef struct image {
	int   Nblig;
	int   Nbcol;
	unsigned char*  data;
	unsigned char** pixel;
} IMAGE;

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

typedef struct point
{
	int x;
	int y;
} POINT;

typedef struct signature_composantes_connexes {
	int surface;
	float perimetre;
	float compacite;
	int bord;
	POINT CG ; /* pour le centre de gravité, cf libOutils */
	float rayon ; /* rayon moyen calculé via la surface */
	float regA;
	float regB;
} SIGNATURE_COMPOSANTE_CONNEXE;

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

int* histogrammeImage(IMAGE img, int choix, int n);
int* histogrammeImageRGB(IMAGERGB img, int choix, int n);
IMAGE expansionImage(IMAGE img, int outMin, int outMax);
IMAGE egalisationImage(IMAGE img);
SIGNATURES statistiquesImage(IMAGE img);
IMAGERGB colorisationImage(IMAGE img, char *table);

IMAGE seuillageOtsu(IMAGE img);

void lsqe(const float *x, const float *y, int n, float *a, float *b);
void regression(POINT *tab, int n, float *a, float *b);
POINT* imageVersPoints(IMAGE img, int *n, char axe);
SIGNATURE_COMPOSANTE_CONNEXE* signaturesImage(IMAGE img, int nbComp);
void sauvegardeSignaturesImage(SIGNATURE_COMPOSANTE_CONNEXE* sig, int nbComp, const char* fic);

IMAGE erosionImage(IMAGE img, int voisinage);
IMAGE dilatationImage(IMAGE img, int voisinage);
IMAGE ouvertureImage(IMAGE img, int voisinage);
IMAGE fermetureImage(IMAGE img, int voisinage);

int distanceEuclidienne(POINT p1, POINT p2);

IMAGE labelImage(IMAGE img, int *nbComp); /* image label + nb composantes connexes */

#endif
