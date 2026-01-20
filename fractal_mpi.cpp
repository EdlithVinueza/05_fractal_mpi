#include "fractal_mpi.h"
#include "palette.h"

#include <cstdint>
#include <complex>

// variables externas definidas en main.cpp
extern int max_iterations;
extern std::complex<double> c;


void julia_mpi(double x_min, double y_min, double x_max, double y_max, int row_start, int row_end, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / (WIDTH);
    double dy = (y_max - y_min) / (HEIGHT);

    for (int j = row_start; j < row_end; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
            double x = x_min + i * dx;
            double y = y_min + j * dy;
            // no vamos a usar complejos

            auto color = divergente(x, y); // auto es igual a var --> inferencia de tipos

            pixel_buffer[(j - row_start) * WIDTH + i] = color; // asignamos el color al pixel
        }
    }

    for(int i =0; i<WIDTH;i++){
        pixel_buffer[i]=0xFF000000;
    }
}


int divergente(double x, double y)
{
    // calculos manuales
    int iter = 1;
    double zr = x;
    double zi = y;

    while ((zr * zr + zi * zi) < 4.0 && iter < max_iterations)
    {
        double dr = zr * zr - zi * zi + c.real(); // calculamos la parte real
        double di = 2.0 * zr * zi + c.imag();     // calculamos la parte imaginaria

        zr = dr;
        zi = di;
        iter++;
    }

    if (iter < max_iterations) // mandamos un color
    {
        int index = (iter % PALETTE_SIZE); // obtenemos el indice de la paleta
        return color_ramp[index];          // regresamos el color
    }

    return 0xFF000000; // color negro
}