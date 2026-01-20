cmake_minimum_required(VERSION 3.16)

project(fractalMPI)

find_package(fmt CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(Freetype REQUIRED)
find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)



# --conf MPI
set(MPI_ROOT "c:/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPI_ROOT}/include")
SET(MPI_LIB "${MPI_ROOT}/lib")

include_directories(
    ${MPI_INCLUDE}
    ) 

    #--
add_executable(fractal 
    main_fractal.cpp
    fractal_mpi.cpp 
    palette.cpp
    draw_text.cpp
    )

target_link_libraries(fractal  PRIVATE 
    fmt::fmt
    Freetype::Freetype
    ${MPI_LIB}/impi.lib
    SFML::Graphics)

