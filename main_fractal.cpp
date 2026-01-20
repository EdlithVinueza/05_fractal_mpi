#include <iostream>
#include <fmt/core.h>
#include <mpi.h>
#include "fractal_mpi.h"
#include "palette.h"
#include <complex>
#include <cstdint>
#include <SFML/Graphics.hpp>

#include "draw_text.h"
#include "arial_ttf2.h"

#ifdef _WIN32
#include <windows.h>
#endif

double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

int max_iterations = 10;

std::complex<double> c(-0.7, 0.27015);

uint32_t *pixel_buffer = nullptr;
uint32_t *texture_buffer = nullptr;

uint32_t *texture = nullptr;

int nprocs;
int rank;
int delta;
int padding;
int row_start;
int row_end;

int32_t running = 1;

void dibuja_text_rank()
{
    auto texto = fmt::format("RANK_{} ", rank);
    draw_text_to_texture(
        (unsigned char *)pixel_buffer, WIDTH, delta,
        texto.c_str(), 10, 25, 20);
}

void set_ui()
{
    fmt::println("Rank_{} setting un UI", rank);
    std::cout.flush();
    texture_buffer = new uint32_t[WIDTH * HEIGHT];
    std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));

    // inicializar la Ui

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Julia Set MPI",
                            sf::Style::Default);

#ifdef _WIN32
    HWND hwnd = window.getNativeHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    sf::Texture texture({WIDTH, HEIGHT});
    texture.update((const uint8_t *)texture_buffer);
    sf::Sprite sprite(texture);

    //-- textos
    const sf::Font font(arial_ttf2::data2, arial_ttf2::data_len2);

    sf::Text text(font, "Julia Set MPI", 24);
    text.setFillColor(sf::Color::White);
    text.setPosition({10, 10});
    text.setStyle(sf::Text::Bold);

    std::string options = "OPTIONS: [1] Serial 1 UP/DOWN: Change Iterations";
    sf::Text textOptions(font, options, 24);
    textOptions.setFillColor(sf::Color::White);
    textOptions.setStyle(sf::Text::Bold);
    textOptions.setPosition({10, window.getView().getSize().y - 40});

    // FPS
    int frame = 0;
    int fps = 0; // cuantos mas fps aumenta signifa que dibuja mucho mas rapido
    sf::Clock clockFrames;

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            // notificar a los otros RANKS
            if (event->is<sf::Event::Closed>())
            {
                running = false;
                window.close();
            }
            else if (event->is<sf::Event::KeyReleased>())
            {
                auto evt = event->getIf<sf::Event::KeyReleased>();
                switch (evt->scancode)
                {
                case sf::Keyboard::Scan::Up:
                    max_iterations += 10;
                    break;
                case sf::Keyboard::Scan::Down:
                    max_iterations -= 10;
                    if (max_iterations < 10)
                        max_iterations = 10;
                    break;
                }
                std::memset(texture_buffer, 0, WIDTH * HEIGHT * sizeof(uint32_t));
            }
        }
        // notificar
        int32_t data[2];
        data[0] = running;
        data[1] = max_iterations;
        MPI_Bcast(data, 2, MPI_INT, 0, MPI_COMM_WORLD);

        if (running == false)
        {
            break;
        }

        julia_mpi(x_min, y_min, x_max, y_max, row_start, row_end, pixel_buffer);
        dibuja_text_rank();
        std::memcpy(texture_buffer, pixel_buffer, WIDTH * delta * sizeof(uint32_t));

        for (int i = 1; i < nprocs; i++)
        {
            int new_delta = delta;
            if (i == nprocs - 1) // último rank podría ser más pequeño
            {
                new_delta = delta - padding;
            }

            MPI_Recv(
                texture_buffer + i * WIDTH * delta, WIDTH * new_delta, MPI_UNSIGNED, // en donde recibio?
                i, 0, MPI_COMM_WORLD,                                            // quien envia
                MPI_STATUS_IGNORE);
        }

        texture.update((const uint8_t *)texture_buffer);

        // contar FPS
        frame++;
        if (clockFrames.getElapsedTime().asSeconds() >= 1.0f)
        {
            fps = frame;
            frame = 0;
            clockFrames.restart();
        }

        auto msg = fmt::format("Julia Set: Iteraciones: {}, FPS: {}", max_iterations, fps);
        text.setString(msg);
        // escribir imagen
        // stbi_write_png("fractal.png", WIDTH, HEIGHT, STBI_rgb_alpha, texture_buffer, WIDTH * 4);
        window.clear();
        {
            window.draw(sprite);
            window.draw(text);
            window.draw(textOptions);
        }
        window.display();
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    init_freetype();

    delta = std::ceil(HEIGHT * 1.0 / nprocs);
    row_start = rank * delta;
    row_end = (rank + 1) * delta;

    padding = delta * nprocs - HEIGHT;

    if (row_end > HEIGHT)
    {
        row_end = HEIGHT;
    }

    // icicializar buffers
    pixel_buffer = new uint32_t[WIDTH * delta];
    std::memset(pixel_buffer, 0, WIDTH * delta * sizeof(uint32_t));

    fmt::print("Rank {}, nprocs {}, delta {}, row_start {}, row_end {}",
               rank, nprocs, delta, row_start, row_end);

    std::cout.flush();

    if (rank == 0)
    {
        fmt::println("Rank_0 starting UI");
        set_ui();
        
        // Notificar a todos los procesos que deben terminar
        int32_t data[2];
        data[0] = running;  // running = false
        data[1] = max_iterations;
        MPI_Bcast(data, 2, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
    {
        while (true)
        {
            int32_t data[2];

            MPI_Bcast(data, 2, MPI_INT, 0, MPI_COMM_WORLD);
            running = data[0];
            max_iterations = data[1];

            if (running == false)
            {
                break;
            }

            julia_mpi(x_min, y_min, x_max, y_max, row_start, row_end, pixel_buffer);
            dibuja_text_rank();

            // Calcular el número real de filas para este proceso
            int actual_rows = row_end - row_start;
            
            MPI_Send(
                pixel_buffer, WIDTH * actual_rows, MPI_UNSIGNED, // envio
                0, 0, MPI_COMM_WORLD);                           // recibe el rank 0
        }
    }

    MPI_Finalize();
    return 0;
}