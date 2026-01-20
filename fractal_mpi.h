#ifndef _FRACTAL_MPI_H_
#define _FRACTAL_MPI_H_

#include <cstdint>

#define WIDTH 1600
#define HEIGHT 900

void julia_mpi(
    double x_min,
    double y_min,
    double x_max,
    double y_max,
    int row_start, //fila inicial
    int row_end, //fila final
    uint32_t * pixel_buffer

);

int divergente(double x, double y);

#endif 