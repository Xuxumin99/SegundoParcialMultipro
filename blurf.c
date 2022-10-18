#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

void main()
{

  FILE *image, *outputImage, *lecturas;
  image = fopen("f5.1.bmp", "rb");                   // Imagen original a transformar
  outputImage = fopen("outred.bmp", "wb"); // Imagen transformada
  long ancho;
  long alto;
  unsigned char r, g, b; // Pixel
  omp_set_num_threads(1);

  unsigned char xx[54];
  for (int i = 0; i < 54; i++)
  {
    xx[i] = fgetc(image);
    fputc(xx[i], outputImage); // Copia cabecera a nueva imagen
  }

  ancho = (long)xx[20] * 65536 + (long)xx[19] * 256 + (long)xx[18];
  alto = (abs)((long)(xx[25] << 24) + (long)(xx[24] << 16) + (long)(xx[23] << 8) + (long)(xx[22] << 0));
  printf("alto img %li\n", alto);
  printf("ancho img %li\n", ancho);

  long dimension_vector = ancho * alto; // Cantidad total de pixeles RGB

  // Se reserva memoria considerando que cada pixel es RGB y ocupa 8 bits
  unsigned char *tmp_pixel_vector = malloc(((dimension_vector)*3) * sizeof(unsigned char));
  // Vector usado para guardar temporalmente el vector girado verticalmente
  unsigned char *tmp_flip_V_vector = malloc(((dimension_vector)*3) * sizeof(unsigned char)); //[0,255].

#pragma omp parallel
  {
    #pragma omp for schedule(dynamic)
    for (int i = 0; i < dimension_vector; i++)
    {
      #pragma omp critical
      b = fgetc(image);
      #pragma omp critical
      g = fgetc(image);
      #pragma omp critical
      r = fgetc(image);

      unsigned char pixel = 0.21 * r + 0.72 * g + 0.07 * b;
      tmp_pixel_vector[i] = pixel;     // b
      tmp_pixel_vector[i + 1] = pixel; // g
      tmp_pixel_vector[i + 2] = pixel; // r
    }
  }

  // Matriz de 9x9
  int linea = 0;
  int Counter2 = 0;
  int inicioLin = 0;
  int finLin = ancho - 1;
  int valor = 0;
  int a = 1;
  const double startTime = omp_get_wtime();
  #pragma omp parallel
  {
    // Rotado verticalmente sobre su eje de simetria
    #pragma omp for schedule(dynamic) collapse(2)
    for (int i = 0; i < (alto); i++)
    {
      for (int j = 0; j < (ancho); j++)
      {
        tmp_flip_V_vector[j + (ancho * i)] = tmp_pixel_vector[(ancho - j) + (i * ancho)];
        tmp_flip_V_vector[(j + (ancho * i)) + 1] = tmp_pixel_vector[((ancho - j) + (i * ancho)) - 1];
        tmp_flip_V_vector[(j + (ancho * i)) + 2] = tmp_pixel_vector[((ancho - j) + (i * ancho)) - 2];
      }
    }


    //////////////// SIN FLIP: COMENTAR PARA USAL LA FUNCION DE ABAJO
    // #pragma omp for schedule(dynamic)
    // for (int i = 0; i < alto * ancho; i++)
    // {

    //   if (linea <= 3 || (i == (inicioLin) && i <= (inicioLin + 3)) || (i >= (finLin - 3) && i == (finLin)) || linea >= (alto - 4))
    //   {
    //     fputc(tmp_pixel_vector[i], outputImage);
    //     fputc(tmp_pixel_vector[i], outputImage);
    //     fputc(tmp_pixel_vector[i], outputImage);
    //   }
    //   else
    //   {

    //     for (int j = -4; j <= 4; j++)
    //     {
    //       a = i + (ancho * j);
    //       valor = valor + ((tmp_pixel_vector[a] + tmp_pixel_vector[a - 1] + tmp_pixel_vector[a - 2] + tmp_pixel_vector[a - 3] + tmp_pixel_vector[a - 4] + tmp_pixel_vector[a + 1] + tmp_pixel_vector[a + 2] + tmp_pixel_vector[a + 3] + tmp_pixel_vector[a + 4]) / 81);
    //     }

    //     fputc(valor, outputImage);
    //     fputc(valor, outputImage);
    //     fputc(valor, outputImage);
    //     valor = 0;
    //   }
    //   Counter2++;
    //   if (Counter2 == ancho)
    //   { // fin de linea
    //     Counter2 = 0;
    //     linea++;
    //     inicioLin = linea * ancho;
    //     finLin = inicioLin + (ancho)-1;
    //   }
    // }
    ///////////////////////////////////////////////// FIN SIN FLIP:




    //////FLIP VERTICAL: COMENTAR PARA USAL LA FUNCION DE ARRIBA
    #pragma omp for schedule(dynamic)
    for (int i = 0; i < alto * ancho; i++)
    {

      if (linea <= 3 || (i == (inicioLin) && i <= (inicioLin + 3)) || (i >= (finLin - 3) && i == (finLin)) || linea >= (alto - 4))
      {
        fputc(tmp_flip_V_vector[i], outputImage);
        fputc(tmp_flip_V_vector[i], outputImage);
        fputc(tmp_flip_V_vector[i], outputImage);
      }
      else
      {

        for (int j = -4; j <= 4; j++)
        {
          a = i + (ancho * j);
          valor = valor + ((tmp_flip_V_vector[a] + tmp_flip_V_vector[a - 1] + tmp_flip_V_vector[a - 2] + tmp_flip_V_vector[a - 3] + tmp_flip_V_vector[a - 4] + tmp_flip_V_vector[a + 1] + tmp_flip_V_vector[a + 2] + tmp_flip_V_vector[a + 3] + tmp_flip_V_vector[a + 4]) / 81);
        }

        fputc(valor, outputImage);
        fputc(valor, outputImage);
        fputc(valor, outputImage);
        valor = 0;
      }
      Counter2++;
      if (Counter2 == ancho)
      { // fin de linea
        Counter2 = 0;
        linea++;
        inicioLin = linea * ancho;
        finLin = inicioLin + (ancho)-1;
      }
    }
    //////////////////////////////////////////////////FIN FLIP VERTICAL: 


  }

  const double endTime = omp_get_wtime();
  printf("Tiempo de ejecucion: %lf\n", (endTime - startTime));

  free(tmp_pixel_vector);
  fclose(image);
  fclose(outputImage);
}


