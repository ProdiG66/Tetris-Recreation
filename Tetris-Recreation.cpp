#include <chrono>
#include <random>
#include <SFML/Graphics.hpp>

#include "Headers/DrawText.hpp"
#include "Headers/Global.hpp"
#include "Headers/GetTetromino.hpp"
#include "Headers/GetWallKickData.hpp"
#include "Headers/Tetromino.hpp"

int main()
{
    bool game_over = false;
    bool hard_drop_pressed = false;
    bool rotate_pressed = false;
    unsigned lag = 0;
    unsigned lines_cleared = 0;
    unsigned char clear_effect_timer = 0;
    unsigned char current_fall_speed = START_FALL_SPEED;
    unsigned char fall_timer = 0;
    unsigned char move_timer = 0;
    unsigned char next_shape;
    unsigned char soft_drop_timer = 0;

    std::chrono::time_point<std::chrono::steady_clock> previous_time;
    std::random_device random_device;

    std::default_random_engine random_engine(random_device());

    std::uniform_int_distribution<unsigned short> shape_distribution(0, 6);

    std::vector<bool> clear_lines(ROWS, false);

    std::vector<sf::Color> cell_colors = {
        sf::Color(36, 36, 85),
        sf::Color(0, 219, 255),
        sf::Color(0, 36, 255),
        sf::Color(255, 146, 0),
        sf::Color(255, 219, 0),
        sf::Color(0, 219, 0),
        sf::Color(146, 0, 255),
        sf::Color(219, 0, 0),
        sf::Color(73, 73, 85)
    };

    std::vector<std::vector<unsigned char>> matrix(COLUMNS, std::vector<unsigned char>(ROWS));

    sf::Event event;

    sf::RenderWindow window(sf::VideoMode(2 * CELL_SIZE * COLUMNS * SCREEN_RESIZE, CELL_SIZE * ROWS * SCREEN_RESIZE), "Tetris", sf::Style::Close);
    window.setView(sf::View(sf::FloatRect(0, 0, 2 * CELL_SIZE * COLUMNS, CELL_SIZE * ROWS)));

    Tetromino tetromino(static_cast<unsigned char>(shape_distribution(random_engine)), matrix);

    next_shape = static_cast<unsigned char>(shape_distribution(random_engine));

    previous_time = std::chrono::steady_clock::now();

    while (window.isOpen())
    {
        unsigned delta_time = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - previous_time).count();

        lag += delta_time;

        previous_time += std::chrono::microseconds(delta_time);

        while (FRAME_DURATION <= lag)
        {
            lag -= FRAME_DURATION;

            while (window.pollEvent(event))
            {
                switch (event.type)
                {
                case sf::Event::Closed:
                    window.close();
                    break;

                case sf::Event::KeyReleased:
                    switch (event.key.code)
                    {
                    case sf::Keyboard::C:
                    case sf::Keyboard::Z:
                        rotate_pressed = false;
                        break;

                    case sf::Keyboard::Down:
                        soft_drop_timer = 0;
                        break;

                    case sf::Keyboard::Left:
                    case sf::Keyboard::Right:
                        move_timer = 0;
                        break;

                    case sf::Keyboard::Space:
                        hard_drop_pressed = false;
                    }
                }
            }

            if (clear_effect_timer == 0)
            {
                if (game_over == false)
                {
                    if (rotate_pressed == false)
                    {
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::C) == 1)
                        {
                            rotate_pressed = true;
                            tetromino.rotate(1, matrix);
                        }
                        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z) == 1)
                        {
                            rotate_pressed = true;
                            tetromino.rotate(0, matrix);
                        }
                    }

                    if (move_timer == 0)
                    {
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) == 1)
                        {
                            move_timer = 1;
                            tetromino.move_left(matrix);
                        }
                        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) == 1)
                        {
                            move_timer = 1;
                            tetromino.move_right(matrix);
                        }
                    }
                    else
                    {
                        move_timer = (1 + move_timer) % MOVE_SPEED;
                    }

                    if (hard_drop_pressed == false)
                    {
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) == 1)
                        {
                            hard_drop_pressed = true;
                            fall_timer = current_fall_speed;
                            tetromino.hard_drop(matrix);
                        }
                    }

                    if (soft_drop_timer == 0)
                    {
                        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) == 1)
                        {
                            if (tetromino.move_down(matrix) == 1)
                            {
                                fall_timer = 0;
                                soft_drop_timer = 1;
                            }
                        }
                    }
                    else
                    {
                        soft_drop_timer = (1 + soft_drop_timer) % SOFT_DROP_SPEED;
                    }

                    if (current_fall_speed == fall_timer)
                    {
                        if (tetromino.move_down(matrix) == 0)
                        {
                            tetromino.update_matrix(matrix);

                            for (unsigned char a = 0; a < ROWS; a++)
                            {
                                bool clear_line = true;

                                for (unsigned char b = 0; b < COLUMNS; b++)
                                {
                                    if (matrix[b][a] == 0)
                                    {
                                        clear_line = false;
                                        break;
                                    }
                                }

                                if (clear_line == true)
                                {
                                    lines_cleared++;

                                    clear_effect_timer = CLEAR_EFFECT_DURATION;

                                    clear_lines[a] = true;

                                    if (lines_cleared % LINES_TO_INCREASE_SPEED == 0)
                                    {
                                        current_fall_speed = std::max<unsigned char>(SOFT_DROP_SPEED, current_fall_speed - 1);
                                    }
                                }
                            }

                            if (clear_effect_timer == 0)
                            {
                                game_over = tetromino.reset(next_shape, matrix) == 0;

                                next_shape = static_cast<unsigned char>(shape_distribution(random_engine));

                                std::fill(clear_lines.begin(), clear_lines.end(), false);
                            }
                        }

                        fall_timer = 0;
                    }
                    else
                    {
                        fall_timer++;
                    }
                }
                else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) == 1)
                {
                    game_over = false;
                    hard_drop_pressed = false;
                    rotate_pressed = false;

                    lines_cleared = 0;

                    current_fall_speed = START_FALL_SPEED;
                    fall_timer = 0;
                    move_timer = 0;
                    soft_drop_timer = 0;

                    for (std::vector<unsigned char>& a : matrix)
                    {
                        std::fill(a.begin(), a.end(), 0);
                    }
                }
            }
            else
            {
                clear_effect_timer--;

                if (clear_effect_timer == 0)
                {
                    for (unsigned char a = 0; a < ROWS; a++)
                    {
                        if (clear_lines[a] == true)
                        {
                            for (unsigned char b = 0; b < COLUMNS; b++)
                            {
                                matrix[b][a] = 0;

                                for (unsigned char c = a; c > 0; c--)
                                {
                                    matrix[b][c] = matrix[b][c - 1];
                                    matrix[b][c - 1] = 0;
                                }
                            }
                        }
                    }

                    game_over = tetromino.reset(next_shape, matrix) == 0;

                    next_shape = static_cast<unsigned char>(shape_distribution(random_engine));

                    std::fill(clear_lines.begin(), clear_lines.end(), false);
                }
            }

            if (FRAME_DURATION > lag)
            {
                unsigned char clear_cell_size = static_cast<unsigned char>(2 * round(0.5f * CELL_SIZE * (clear_effect_timer / static_cast<float>(CLEAR_EFFECT_DURATION))));

                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
                sf::RectangleShape preview_border(sf::Vector2f(5 * CELL_SIZE, 5 * CELL_SIZE));
                preview_border.setFillColor(sf::Color(0, 0, 0));
                preview_border.setOutlineThickness(-1);
                preview_border.setPosition(CELL_SIZE * (1.5f * COLUMNS - 2.5f), CELL_SIZE * (0.36f * ROWS - 2.5f));

                window.clear();

                for (unsigned char a = 0; a < COLUMNS; a++)
                {
                    for (unsigned char b = 0; b < ROWS; b++)
                    {
                        if (clear_lines[b] == false)
                        {
                            cell.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));

                            if (game_over == true && matrix[a][b] > 0)
                            {
                                cell.setFillColor(cell_colors[8]);
                            }
                            else
                            {
                                cell.setFillColor(cell_colors[matrix[a][b]]);
                            }

                            window.draw(cell);
                        }
                    }
                }

                cell.setFillColor(cell_colors[8]);

                if (game_over == false)
                {
                    for (Position& mino : tetromino.get_ghost_minos(matrix))
                    {
                        cell.setPosition(static_cast<float>(CELL_SIZE * mino.x), static_cast<float>(CELL_SIZE * mino.y));
                        window.draw(cell);
                    }

                    cell.setFillColor(cell_colors[1 + tetromino.get_shape()]);
                }

                for (Position& mino : tetromino.get_minos())
                {
                    cell.setPosition(static_cast<float>(CELL_SIZE * mino.x), static_cast<float>(CELL_SIZE * mino.y));
                    window.draw(cell);
                }

                for (unsigned char a = 0; a < COLUMNS; a++)
                {
                    for (unsigned char b = 0; b < ROWS; b++)
                    {
                        if (clear_lines[b] == true)
                        {
                            cell.setFillColor(cell_colors[0]);
                            cell.setPosition(static_cast<float>(CELL_SIZE * a), static_cast<float>(CELL_SIZE * b));
                            cell.setSize(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));
                            window.draw(cell);

                            cell.setFillColor(sf::Color(255, 255, 255));
                            cell.setPosition(floor(CELL_SIZE * (0.5f + a) - 0.5f * clear_cell_size), floor(CELL_SIZE * (0.5f + b) - 0.5f * clear_cell_size));
                            cell.setSize(sf::Vector2f(clear_cell_size, clear_cell_size));
                            window.draw(cell);
                        }
                    }
                }

                cell.setFillColor(cell_colors[1 + next_shape]);
                cell.setSize(sf::Vector2f(CELL_SIZE - 1, CELL_SIZE - 1));

                window.draw(preview_border);

                for (Position& mino : get_tetromino(next_shape, static_cast<unsigned char>(1.5f * COLUMNS), static_cast<unsigned char>(0.36f * ROWS)))
                {
                    unsigned short next_tetromino_x = CELL_SIZE * mino.x;
                    unsigned short next_tetromino_y = CELL_SIZE * mino.y;

                    if (next_shape == 0)
                    {
                        next_tetromino_y += static_cast<unsigned char>(round(0.5f * CELL_SIZE));
                    }
                    else if (next_shape != 3)
                    {
                        next_tetromino_x -= static_cast<unsigned char>(round(0.5f * CELL_SIZE));
                    }

                    cell.setPosition(next_tetromino_x, next_tetromino_y);
                    window.draw(cell);
                }

                draw_text(
                    static_cast<unsigned short>(CELL_SIZE * (0.5f + COLUMNS)),
                    static_cast<unsigned short>(0.04f * CELL_SIZE * ROWS),
                    "Tetris Recreation\nDeveloped By:\nJan Enri Arquero",
                    window,
                    0.5f
                );

                draw_text(
                    static_cast<unsigned short>(CELL_SIZE * (0.5f + COLUMNS)),
                    static_cast<unsigned short>(0.5f * CELL_SIZE * ROWS),
                    "Score:" + std::to_string(lines_cleared) + "\nSpeed:" + std::to_string(START_FALL_SPEED / current_fall_speed) + 'x',
                    window,
                    1.0f
                );

                draw_text(
                    static_cast<unsigned short>(CELL_SIZE * (0.25f + COLUMNS)),
                    static_cast<unsigned short>(0.7f * CELL_SIZE * ROWS),
                    "Input Controls:\nLeft/Right - Move\nDown - Speed Up\nSpace - Hard Drop\nZ/C - Rotate",
                    window,
                    0.5f
                );
                window.display();
            }
        }
    }
}