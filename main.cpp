#include <iostream>
#include <cstring>
#include <fmt/core.h>

#include <mpi.h>
#include <complex> // permite trabajar con números complejos

#include "fractal_mpi.h"

#include "draw_text.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Parámetros: límites del rectángulo complejo que vamos a muestrear
double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

// Control de iteraciones y constante c para el conjunto de Julia
int max_iterations = 10;
std::complex<double> c(-0.7, 0.27015);

uint32_t *pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr;

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int nprocs;
    int rank;

    // ranks

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    init_freetype();

    int delta = std::ceil(HEIGHT * 1.0 / nprocs);

    int row_start = rank * delta;
    int row_end = (rank + 1) * delta;

    int padding = delta * nprocs - HEIGHT;

    if (row_end > HEIGHT)
    { // ajusta el último
        row_end = HEIGHT;
    }

    //-- inicializar los buffers
    pixel_buffer = new uint32_t[WIDTH * delta];                     // va en todos los ranck
    std::memset(pixel_buffer, 0, WIDTH * delta * sizeof(uint32_t)); // inicializamos a 0

    fmt::println("RANK = {}, nprocs = {}, delta 0 {},  start = {},end = {}", rank, nprocs, delta, row_start, row_end);

    std::cout.flush();

    if (rank == 0)
    {
        // necesita el buffer de textura por que es el que guarda todo
        texture_buffer = new uint32_t[WIDTH * HEIGHT];
        std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t)); // inicializamos a 0

        julia_mpi(x_min, y_min, x_max, y_max, row_start, row_end, pixel_buffer);

        std::memcpy(texture_buffer + 0 * WIDTH * delta, pixel_buffer, WIDTH * delta * sizeof(uint32_t));

        for (int i = 1; i < nprocs; ++i)
        {
            MPI_Recv(
                (char*)texture_buffer + i * WIDTH * delta * sizeof(uint32_t), WIDTH * delta * sizeof(uint32_t), MPI_BYTE, // en donde recibe
                i, 0, MPI_COMM_WORLD,                          // quien envia?
                MPI_STATUS_IGNORE 
           );
        }

        //escribir la imagen
        stbi_write_png("fractal.png", WIDTH, HEIGHT, 4, (unsigned char *)texture_buffer, WIDTH * 4);  
    }
    else
    { // vamos con comunicación punto a punto
        julia_mpi(x_min, y_min, x_max, y_max, row_start, row_end, pixel_buffer);

        auto texto = fmt::format("RANK {} ", rank);
        draw_text_to_texture(
            (unsigned char *)pixel_buffer, WIDTH, delta,
            texto.c_str(), 10, 25, 20);

        MPI_Send(
            (char*)pixel_buffer, WIDTH * delta * sizeof(uint32_t), MPI_BYTE, // envio
            0, 0, MPI_COMM_WORLD                       // recibe
        );

    }

    MPI_Finalize();

    return 0;
}