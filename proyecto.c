//*****************************************************************
// EDGARDO ADRIÁN FRANCO MARTÍNEZ
//(C) Agosto 2010 Versión 1.5
// Lectura, escritura y tratamiento de imagenes BMP
// Compilación: "gcc BMP.c -o BMP"
// Ejecución: "./BMP imagen.bmp"
// Observaciones "imagen.bmp" es un BMP de 24 bits

// Archivo modificado por Mariela Curiel par leer toda la imagen en la memoria
// y hacer la conversion de los pixeles en una funci'on. Esto facilita la programacion posterior
// con hilos.

// Archivo nuevamente modificado por alejandro Uscátegui, ejecutando el programa con hilos y agregando un filtro que elimina los colores rojos.

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

// Estructura de la cabecera del bitmap y de su mapadePixeles (en 3 dimensiones para incluir dimensiones ancho*alto,
// la tercera dimensión representa los colores que que componen al pixel)

typedef struct BMP
{
	char bm[2];					//(2 Bytes) BM (Tipo de archivo)
	int tamano;					//(4 Bytes) Tamaño del archivo en bytes
	int reservado;				//(4 Bytes) Reservado
	int offset;					//(4 Bytes) offset, distancia en bytes entre la img y los píxeles
	int tamanoMetadatos;		//(4 Bytes) Tamaño de Metadatos (tamaño de esta estructura = 40)
	int alto;					//(4 Bytes) Ancho (numero de píxeles horizontales)
	int ancho;					//(4 Bytes) Alto (numero de pixeles verticales)
	short int numeroPlanos;		//(2 Bytes) Numero de planos de color
	short int profundidadColor; //(2 Bytes) Profundidad de color (debe ser 24 para nuestro caso)
	int tipoCompresion;			//(4 Bytes) Tipo de compresión (Vale 0, ya que el bmp es descomprimido)
	int tamanoEstructura;		//(4 Bytes) Tamaño de la estructura Imagen (Paleta)
	int pxmh;					//(4 Bytes) Píxeles por metro horizontal
	int pxmv;					//(4 Bytes) Píxeles por metro vertical
	int coloresUsados;			//(4 Bytes) Cantidad de colores usados
	int coloresImportantes;		//(4 Bytes) Cantidad de colores importantes
	unsigned char ***pixel;		// Puntero a una tabla dinamica de caracteres de 3 dimensiones para almacenar los pixeles
} BMP;

// Globales
// Cuando se inicializan las variables char* quedan en modo de sólo lectura y no se pueden modificar
// por esto las variables tipo "string" están inicializadas de esta manera (como si fueran un buffer)
char imagenIn[200];
char imagenOut[200];
int anchoactual = 0;
int multiplicadorAncho = 0;
int nhilos = 0;
int opcion = 0;
BMP img;
int parteResuelta = 0;

// Función extra para que la interfaz se vea decente
void mostrarASCII(char *nombredelarchivo)
{
	FILE *apuntadorAlArchivo;
	char linea[300];

	apuntadorAlArchivo = fopen(nombredelarchivo, "r");

	if (apuntadorAlArchivo == NULL)
	{
		printf("Error opening file.\n");
		return;
	}

	while (fgets(linea, 300, apuntadorAlArchivo) != NULL)
	{
		printf("%s", linea);
	}

	fclose(apuntadorAlArchivo);
}

void *filtroOpcionUno(void *ptr)
{
	// orden: blue, green, red
	int i, j, k;

	unsigned char temp;

	for (i = 0; i < img.alto; i++)
	{
		for (j = anchoactual * multiplicadorAncho; j < anchoactual; j++)
		{
			temp = (unsigned char)((img.pixel[i][j][2] * 0.3 + img.pixel[i][j][1] * 0.59 + img.pixel[i][j][0] * 0.11));

			for (k = 0; k < 3; k++)
			{
				img.pixel[i][j][k] = (unsigned char)temp;
			}
		}
	}

	multiplicadorAncho++;
}

void *filtroOpcionDos(void *ptr)
{
	// orden: blue, green, red
	int i, j, k;

	unsigned char temp;

	for (i = 0; i < img.alto; i++)
	{
		for (j = anchoactual * multiplicadorAncho; j < anchoactual; j++)
		{
			temp = (unsigned char)((img.pixel[i][j][2] + img.pixel[i][j][1] + img.pixel[i][j][0]) / 3);

			for (k = 0; k < 3; k++)
			{
				img.pixel[i][j][k] = (unsigned char)temp;
			}
		}
	}

	multiplicadorAncho++;
}

void *filtroSinRojos(void *ptr)
{
	// orden: blue, green, red
	int i, j, k;

	for (i = 0; i < img.alto; i++)
	{
		for (j = anchoactual * multiplicadorAncho; j < anchoactual; j++)
		{
			for (k = 0; k < 3; k++)

				if (img.pixel[i][j][k] <= 255 && k == 0)
				{
					img.pixel[i][j][k] = (unsigned char)0; // pongo los pixeles azules bajos
				}
		}
	}

	multiplicadorAncho++;
}

void abrir_imagen(BMP *imagen, char *ruta)
{
	FILE *archivo; // Puntero FILE para el archivo de imágen a abrir
	int i, j, k;
	unsigned char P[3];

	// Abrir el archivo de imágen
	archivo = fopen(ruta, "rb+");
	if (!archivo)
	{
		// Si la imágen no se encuentra en la ruta dada
		printf("\n*****************************************************************************\n");
		mostrarASCII("codigoerror.txt");
		printf("\nLa imágen %s no se encontro\n", ruta);
		printf("\n*****************************************************************************\n");

		exit(1);
	}

	// Leer la cabecera de la imagen y almacenarla en la estructura a la que apunta imagen
	fseek(archivo, 0, SEEK_SET);
	fread(&imagen->bm, sizeof(char), 2, archivo);
	fread(&imagen->tamano, sizeof(int), 1, archivo);
	fread(&imagen->reservado, sizeof(int), 1, archivo);
	fread(&imagen->offset, sizeof(int), 1, archivo);
	fread(&imagen->tamanoMetadatos, sizeof(int), 1, archivo);
	fread(&imagen->alto, sizeof(int), 1, archivo);
	fread(&imagen->ancho, sizeof(int), 1, archivo);
	fread(&imagen->numeroPlanos, sizeof(short int), 1, archivo);
	fread(&imagen->profundidadColor, sizeof(short int), 1, archivo);
	fread(&imagen->tipoCompresion, sizeof(int), 1, archivo);
	fread(&imagen->tamanoEstructura, sizeof(int), 1, archivo);
	fread(&imagen->pxmh, sizeof(int), 1, archivo);
	fread(&imagen->pxmv, sizeof(int), 1, archivo);
	fread(&imagen->coloresUsados, sizeof(int), 1, archivo);
	fread(&imagen->coloresImportantes, sizeof(int), 1, archivo);

	// Validar ciertos datos de la cabecera de la imágen
	if (imagen->bm[0] != 'B' || imagen->bm[1] != 'M')
	{
		printf("\n*********************************************************\n");
		mostrarASCII("codigoerror.txt");
		printf("La imagen debe ser un bitmap.\n");
		printf("\n*********************************************************\n");

		exit(1);
	}
	if (imagen->profundidadColor != 24)
	{
		printf("\n*********************************************************\n");
		mostrarASCII("codigoerror.txt");
		printf("La imagen debe ser de 24 bits.\n");
		printf("\n*********************************************************\n");

		exit(1);
	}

	// Reservar memoria para la matriz de pixels

	imagen->pixel = malloc(imagen->alto * sizeof(char *));
	for (i = 0; i < imagen->alto; i++)
	{
		imagen->pixel[i] = malloc(imagen->ancho * sizeof(char *));
	}

	for (i = 0; i < imagen->alto; i++)
	{
		for (j = 0; j < imagen->ancho; j++)
			imagen->pixel[i][j] = malloc(3 * sizeof(char));
	}

	// Pasar la imágen a el arreglo reservado en escala de grises
	// unsigned char R,B,G;

	for (i = 0; i < imagen->alto; i++)
	{
		for (j = 0; j < imagen->ancho; j++)
		{
			for (k = 0; k < 3; k++)
			{
				fread(&P[k], sizeof(char), 1, archivo);		  // Byte Blue del pixel
				imagen->pixel[i][j][k] = (unsigned char)P[k]; // Formula correcta
			}
		}
	}

	// Cerrrar el archivo
	fclose(archivo);
}

void crear_imagen(BMP *imagen, char ruta[])
{
	FILE *archivo; // Puntero FILE para el archivo de imágen a abrir

	int i, j, k;

	// Abrir el archivo de imágen
	archivo = fopen(ruta, "wb+");
	if (!archivo)
	{
		// Si la imágen no se encuentra en la ruta dada
		mostrarASCII("codigoerror.txt");
		printf("La imágen %s no se pudo crear\n", ruta);
		exit(1);
	}

	// Escribir la cabecera de la imagen en el archivo
	fseek(archivo, 0, SEEK_SET);
	fwrite(&imagen->bm, sizeof(char), 2, archivo);
	fwrite(&imagen->tamano, sizeof(int), 1, archivo);
	fwrite(&imagen->reservado, sizeof(int), 1, archivo);
	fwrite(&imagen->offset, sizeof(int), 1, archivo);
	fwrite(&imagen->tamanoMetadatos, sizeof(int), 1, archivo);
	fwrite(&imagen->alto, sizeof(int), 1, archivo);
	fwrite(&imagen->ancho, sizeof(int), 1, archivo);
	fwrite(&imagen->numeroPlanos, sizeof(short int), 1, archivo);
	fwrite(&imagen->profundidadColor, sizeof(short int), 1, archivo);
	fwrite(&imagen->tipoCompresion, sizeof(int), 1, archivo);
	fwrite(&imagen->tamanoEstructura, sizeof(int), 1, archivo);
	fwrite(&imagen->pxmh, sizeof(int), 1, archivo);
	fwrite(&imagen->pxmv, sizeof(int), 1, archivo);
	fwrite(&imagen->coloresUsados, sizeof(int), 1, archivo);
	fwrite(&imagen->coloresImportantes, sizeof(int), 1, archivo);

	// Pasar la imágen del arreglo reservado en escala de grises a el archivo (Deben escribirse los valores BGR)
	for (i = 0; i < imagen->alto; i++)
	{
		for (j = 0; j < imagen->ancho; j++)
		{

			for (k = 0; k < 3; k++)
				fwrite(&imagen->pixel[i][j][k], sizeof(char), 1, archivo); // Escribir el Byte Blue del pixel
		}
	}
	// Cerrrar el archivo
	fclose(archivo);
}



int main(int argc, char **argv)
{
	// # Validaciones

	if (argc != 9)
	{
		printf("\n*****************************************************************************\n");
		mostrarASCII("codigoerror.txt");
		printf("\n\nRecuerde que debe ingresar el nombre de la imagen que quiere modificar con su extensión '.bmp', \nasí como el nombre que desea que tenga su imagen ya tratada\n");
		printf("\ntambién debe seleccionar la opcion que desea (para este proyecto solo 3), y el número de hilos con los que desea ejecutar el programa");
		printf("\neste es un ejemplo de argumentos válidos: -i imagenin.bmp -t imagenout.bmp -o 1 -h 3\n");
		printf("\n*****************************************************************************\n");
		exit(0);
	}

	for (int i = 1; i < argc; i += 2)
	{
		if (strcmp(argv[i], "-i") == 0)
			strcpy(imagenIn, argv[i + 1]);
		else if (strcmp(argv[i], "-t") == 0)
			strcpy(imagenOut, argv[i + 1]);
		else if (strcmp(argv[i], "-o") == 0)
			opcion = atoi(argv[i + 1]);
		else if (strcmp(argv[i], "-h") == 0)
			nhilos = atoi(argv[i + 1]);
	}

	if (opcion < 1 || opcion > 3)
	{
		printf("\n*****************************************************************************\n");
		mostrarASCII("codigoerror.txt");
		printf("\nla opción -o que ingresó no es válida\n");
		printf("\n*****************************************************************************\n");
		exit(0);
	}

	//******************************************************************
	// Si no se introdujo la ruta de la imagen BMP
	//******************************************************************
	// Si no se introduce una ruta de imágen
	

	// # Variables del programa principal

	int i, j, k;			  // Variables auxiliares para loops
	char imagenOriginal[200]; // Almacenará la ruta de la imagen

	// Creación de Hilos

	pthread_t hilos[nhilos];

	char *ptr = strstr(imagenIn, ".bmp");
	if (ptr != NULL) /* Substring found */
	{
		// printf("\nLa imagen contiene .bmp");
		strcpy(imagenOriginal, imagenIn);
	}
	else /* Substring not found */
	{
		// printf("\nLa imagen no contiene .bmp");
		strcpy(imagenIn, strcat(imagenIn, ".bmp"));
		strcpy(imagenOriginal, imagenIn);
	}

	abrir_imagen(&img, imagenOriginal);

	if (nhilos > img.ancho)
	{
		printf("\n*****************************************************************************\n");
		mostrarASCII("codigoerror.txt");
		printf("\nEl programa se puede ejecutar con un número máximo de hilos equivalentes al ancho de la imagen, para este caso paticular se permiten máximo %d hilos\n", img.ancho);
		printf("\n*****************************************************************************\n");

		exit(0);
	}

	printf("\nBienvenidx al programa de tratamiento de imágenes BMP\n");
	printf("\n*****************************************************************************\n");
	printf("\n--------------------------------------------------------------------------\n\n");
	mostrarASCII("ascii.txt");
	printf("\n\n--------------------------------------------------------------------------\n");
	printf("\nImagen Original: %s\n", imagenOriginal);
	printf("\nDimensiones de la imágen:\tAlto=%d\t|\tAncho=%d\n", img.alto, img.ancho);

	// Estructura para meter más de un argumento a la función del hilo

	int ancho = 0;

	switch (opcion)
	{
	case 1:
		printf("\n--------------------------------------------------------------------------\n");
		printf("\n¡Usted ha elegido el filtro creado por ALEJANDRO USCÁTEGUI Y MARÍA JOSÉ GÓMEZ!\n");
		printf("\n--------------------------------------------------------------------------\n");

		for (int i = 0; i < nhilos; i++)
		{
			anchoactual = (i + 1) * (img.ancho / nhilos);
			pthread_create(&hilos[i], NULL, filtroSinRojos, (void *)&ancho);
		}

		for (int i = 0; i < nhilos; i++)
		{
			pthread_join(hilos[i], NULL);
		}
		break;

	case 2:
		printf("\n--------------------------------------------------------------------------\n");
		printf("\n¡Usted ha elegido el filtro creado por EDGARDO ADRIÁN FRANCO MARTÍNEZ!\n");
		printf("\n--------------------------------------------------------------------------\n");

		for (int i = 0; i < nhilos; i++)
		{
			anchoactual = (i + 1) * (img.ancho / nhilos);
			pthread_create(&hilos[i], NULL, filtroOpcionUno, (void *)&ancho);
		}

		for (int i = 0; i < nhilos; i++)
		{
			pthread_join(hilos[i], NULL);
		}
		break;

	case 3:
		printf("\n--------------------------------------------------------------------------\n");
		printf("\n¡Usted ha elegido el filtro creado por MARIEL CURIEL!\n");
		printf("\n--------------------------------------------------------------------------\n");

		for (int i = 0; i < nhilos; i++)
		{
			anchoactual = (i + 1) * (img.ancho / nhilos);
			pthread_create(&hilos[i], NULL, filtroOpcionDos, (void *)&ancho);
		}

		for (int i = 0; i < nhilos; i++)
		{
			pthread_join(hilos[i], NULL);
		}
		break;

	default:

		break;
	}

	//***************************************************************************************************************************
	// 1 Crear la imágen BMP a partir del arreglo img.pixel[][]
	//***************************************************************************************************************************

	char imagenTratada[200];
	char *ptr2 = strstr(imagenOut, ".bmp");

	if (ptr2 != NULL) /* Substring found */
	{
		// printf("\nLa imagen contiene .bmp");
		strcpy(imagenTratada, imagenOut);
	}
	else /* Substring not found */
	{
		// printf("\nLa imagen no contiene .bmp");
		strcpy(imagenOut, strcat(imagenOut, ".bmp"));
		strcpy(imagenTratada, imagenOut);
	}

	crear_imagen(&img, imagenTratada);
	printf("\n*********************************************************\n");
	printf("\nImágen BMP tratada en el archivo: %s\n", imagenTratada);
	printf("\n*********************************************************\n");

	// Terminar programa normalmente
	return 0;
}