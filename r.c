#include <stdio.h>
#include <stdlib.h>
#include "omp.h"
#include <math.h>

#define NUM_THREADS 50
#define MASK_SIZE 3

int blurring(){
    omp_set_num_threads(NUM_THREADS);
    const double startTime = omp_get_wtime();

    printf("Comienzo...\n");
    FILE *image, *outputImage;
    image = fopen("f.bmp","rb");          //Imagen original a transformar
    outputImage = fopen("fout.bmp","wb");    //Imagen transformada
    long ancho;
    long alto;
    unsigned char rr, g, b;               //Pixel

    unsigned char xx[54];
    for(int i=0; i<54; i++) {
      xx[i] = fgetc(image);
      fputc(xx[i], outputImage);   //Copia cabecera a nueva imagen
    }
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];
    printf("alto img %li\n",alto);
    printf("ancho img %li\n",ancho);

    unsigned char * ptr_imagen_temp = (unsigned char*)malloc(alto*ancho*sizeof(unsigned char));
    unsigned char * ptr_output_temp = (unsigned char*)malloc(alto*ancho*sizeof(unsigned char));
    unsigned char * ptr_out_copia = ptr_output_temp;

    printf("Cree los dos arrays correctos.\n");

    long cont_imag_temp = 0;

    for(int rows=0; rows<alto; rows++){
        for(int cols=0; cols<ancho; cols++){
            b = fgetc(image);
            g = fgetc(image);
            rr = fgetc(image);

            unsigned char pixel = 0.21*rr+0.72*g+0.07*b;

            ptr_imagen_temp[cont_imag_temp] = pixel;
            cont_imag_temp++;
        }
    }

                                       //Grises
    printf("magen_temp\n");

    int borde = floor(MASK_SIZE/2.0);
    printf("Mi borde es de: %d\n", borde);
    unsigned char valor_pixel;

    unsigned char * arriba_seguidor = ptr_imagen_temp;

    long sumaTemp;
    int i,k,r,c;

    printf("output_temp\n");
    #pragma omp parallel
    {
        #pragma omp for schedule(dynamic)
        for(int row=0; row<alto; row++){
            for(int col=0; col<ancho; col++){
                if( (borde<=col) && (col<(ancho-borde)) && (borde<=row) && (row<(alto-borde)) ){   
                    
                    for(i=0; i<borde; i++){         //me paro en medio hasta arriba
                        arriba_seguidor = ptr_imagen_temp - ancho;
                    }

                    for(k=0; k<borde; k++){         //me paro en la esquina superior izquierda
                        arriba_seguidor--;
                    }

                    sumaTemp = 0;
                    for(r=0; r<MASK_SIZE; r++){         //sumo todos los valores de la ventana
                        for(c=0; c<MASK_SIZE; c++){
                            sumaTemp = sumaTemp + *arriba_seguidor;
                            arriba_seguidor++;
                        }
                        arriba_seguidor--;
                        arriba_seguidor = arriba_seguidor + ancho - (MASK_SIZE-1);
                    }

                    arriba_seguidor = ptr_imagen_temp;
                    valor_pixel = sumaTemp/(MASK_SIZE*MASK_SIZE);

                    *ptr_output_temp = valor_pixel;

                    ptr_imagen_temp++;
                    ptr_output_temp++;
                    arriba_seguidor++;

                }
                else{              
                    *ptr_output_temp = *ptr_imagen_temp;
                    ptr_imagen_temp++;
                    ptr_output_temp++;

                    arriba_seguidor++;
                }
            }
        }
    }

    printf("si\n");


        for(int rows = 0; rows<alto; rows++){
            for(int cols = 0; cols<ancho; cols++){
                fputc(*ptr_out_copia, outputImage);
                fputc(*ptr_out_copia, outputImage);
                fputc(*ptr_out_copia, outputImage);

                ptr_out_copia++;
            }
        }

    const double endTime = omp_get_wtime();
    printf("tomo (%lf) segundos\n", (endTime -startTime));

    fclose(image);
    fclose(outputImage);
    printf("bien\n");
}

int main()
{
    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
                (void) blurring();                                            
        }
    }  

    return 0;
}