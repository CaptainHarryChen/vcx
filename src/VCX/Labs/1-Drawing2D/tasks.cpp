#include <random>

#include <spdlog/spdlog.h>

#include "Labs/1-Drawing2D/tasks.h"

using VCX::Labs::Common::ImageRGB;

namespace VCX::Labs::Drawing2D {
    /******************* 1.Image Dithering *****************/
    void DitheringThreshold(
        ImageRGB &       output,
        ImageRGB const & input) {
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                glm::vec3 color = input[{ x, y }];
                output.SetAt({ x, y }, {
                                           color.r > 0.5 ? 1 : 0,
                                           color.g > 0.5 ? 1 : 0,
                                           color.b > 0.5 ? 1 : 0,
                                       });
            }
    }

    void DitheringRandomUniform(
        ImageRGB &       output,
        ImageRGB const & input) {
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                glm::vec3 color = input[{ x, y }];
                color += (rand() % 10000) / 10000.0 - 0.5;
                output.SetAt({ x, y }, {
                                           color.r > 0.5 ? 1 : 0,
                                           color.g > 0.5 ? 1 : 0,
                                           color.b > 0.5 ? 1 : 0,
                                       });
            }
    }

    void DitheringRandomBlueNoise(
        ImageRGB &       output,
        ImageRGB const & input,
        ImageRGB const & noise) {
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                glm::vec3 color = input[{ x, y }];
                color += noise[{x,y}];
                color -= 0.5;
                output.SetAt({ x, y }, {
                                           color.r > 0.5 ? 1 : 0,
                                           color.g > 0.5 ? 1 : 0,
                                           color.b > 0.5 ? 1 : 0,
                                       });
            }
    }

    void DitheringOrdered(
        ImageRGB &       output,
        ImageRGB const & input) {
        int pos[9][2]={{1,1}, {1,0}, {2,1}, {1,2}, {0,2}, {2,0}, {2,2}, {0,1}};
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                glm::vec3 color = input[{ x, y }];
                int t = color.r * 9;
                for(int i=0;i<3;i++)
                    for(int j=0;j<3;j++)
                        output.SetAt({x*3+i,y*3+j}, {0,0,0});
                for(int i=0;i<t;i++)
                    output.SetAt({x*3+pos[i][0],y*3+pos[i][1]},{1,1,1});
            }
    }

    void DitheringErrorDiffuse(
        ImageRGB &       output,
        ImageRGB const & input) {
        int dir[4][2] = {{1,1}, {1,0}, {1,-1}, {0,1}};
        float rat[4] = {1.0/16, 5.0/16, 3.0/16, 7.0/16};
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y)
                output.SetAt({ x, y }, {0, 0, 0});
        for (std::size_t x = 0; x < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y < input.GetSizeY(); ++y) {
                float color = input[{ x, y }].r;
                auto error = output[{ x, y }];
                color += (error.r - error.g) * 10.0; // why round the value in SetAt  ?????  ****.
                for(int i=0; i<4; i++)
                {
                    std::size_t tx = x + dir[i][0];
                    std::size_t ty = y + dir[i][1];
                    if(tx >= 0 && ty >= 0 && tx < input.GetSizeX() && ty < input.GetSizeY())
                    {
                        float c = (color > 0.5 ? color - 1.0 : color) * rat[i] * 0.1;
                        auto tmp = output[{tx, ty}];
                        if( c < 0 )
                            tmp.g += -c;
                        else
                            tmp.r += c;
                        output.SetAt({tx, ty}, tmp);
                    }
                }
                output.SetAt({ x, y }, {
                                           color > 0.5 ? 1 : 0,
                                           color > 0.5 ? 1 : 0,
                                           color > 0.5 ? 1 : 0,
                                       });
            }
    }

    /******************* 2.Image Filtering *****************/
    void Blur(
        ImageRGB &       output,
        ImageRGB const & input) {
        // your code here:
        float kernel[3][3] = {{1.0/9.0, 1.0/9.0, 1.0/9.0},
                              {1.0/9.0, 1.0/9.0, 1.0/9.0},
                              {1.0/9.0, 1.0/9.0, 1.0/9.0}};
        for (std::size_t x = 0; x + 2 < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y + 2 < input.GetSizeY(); ++y)
            {
                glm::vec3 sum = {0, 0, 0};
                for (std::size_t i = 0; i < 3; ++i)
                    for (std::size_t j = 0; j < 3; ++j)
                        sum += kernel[i][j] * input[{x + i, y + j}];
                output.SetAt({x, y}, sum);
            }
    }

    void Edge(
        ImageRGB &       output,
        ImageRGB const & input) {
        float kernel1[3][3] = {{-1.0, 0, 1.0},
                               {-2.0, 0, 2.0},
                               {-1.0, 0, 1.0}};
        float kernel2[3][3] = {{ 1.0,  2.0,  1.0},
                               { 0.0,  0.0,  0.0},
                               {-1.0, -2.0, -1.0}};                 
        for (std::size_t x = 0; x + 2 < input.GetSizeX(); ++x)
            for (std::size_t y = 0; y + 2 < input.GetSizeY(); ++y)
            {
                glm::vec3 sum1 = {0, 0, 0}, sum2 = {0, 0, 0};
                for (std::size_t i = 0; i < 3; ++i)
                    for (std::size_t j = 0; j < 3; ++j)
                    {
                        sum1 += kernel1[i][j] * input[{x + i, y + j}];
                        sum2 += kernel2[i][j] * input[{x + i, y + j}];
                    }
                output.SetAt({x, y}, {sqrt(sum1.r * sum1.r + sum2.r * sum2.r),
                                      sqrt(sum1.g * sum1.g + sum2.g * sum2.g),
                                      sqrt(sum1.b * sum1.b + sum2.b * sum2.b)});
            }
    }

    /******************* 3. Image Inpainting *****************/
    void Inpainting(
        ImageRGB &         output,
        ImageRGB const &   inputBack,
        ImageRGB const &   inputFront,
        const glm::ivec2 & offset) {
        output             = inputBack;
        size_t      width  = inputFront.GetSizeX();
        size_t      height = inputFront.GetSizeY();
        glm::vec3 * g      = new glm::vec3[width * height];
        memset(g, 0, sizeof(glm::vec3) * width * height);
        // set boundary condition
        for (std::size_t y = 0; y < height; ++y) {
            g[y * width] = inputBack [{(std::size_t)offset.x, (std::size_t)offset.y + y}] - inputFront[{0, y}];
            g[y * width + width - 1] = inputBack [{(std::size_t)offset.x + inputFront.GetSizeX(), (std::size_t)offset.y + y}] - inputFront[{width - 1, y}];
        }
        for (std::size_t x = 0; x < width; ++x) {
            g[x] = inputBack [{(std::size_t)offset.x + x, (std::size_t)offset.y}] - inputFront[{x, 0}];
            g[(height - 1) * width + x] = inputBack [{(std::size_t)offset.x + x, (std::size_t)offset.y + inputFront.GetSizeY()}] - inputFront[{x, height - 1}];
        }

        // Jacobi iteration, solve Ag = b
        for (int iter = 0; iter < 8000; ++iter) {
            for (std::size_t y = 1; y < height - 1; ++y)
                for (std::size_t x = 1; x < width - 1; ++x) {
                    g[y * width + x] = (g[(y - 1) * width + x] + g[(y + 1) * width + x] + g[y * width + x - 1] + g[y * width + x + 1]);
                    g[y * width + x] = g[y * width + x] * glm::vec3(0.25);
                }
        }

        for (std::size_t y = 0; y < inputFront.GetSizeY(); ++y)
            for (std::size_t x = 0; x < inputFront.GetSizeX(); ++x) {
                glm::vec3 color = g[y * width + x] + inputFront.GetAt({ x, y });
                output.SetAt({ x + offset.x, y + offset.y }, color);
            }
        delete[] g;
    }

    /******************* 4. Line Drawing *****************/
    void DrawLine(
        ImageRGB &       canvas,
        glm::vec3 const  color,
        glm::ivec2 const p0,
        glm::ivec2 const p1) {
        // your code here:
        int x0 = p0.x, y0 = p0.y, x1 = p1.x, y1 = p1.y;
        bool need_swap = false;
        if (abs((int)x1 - x0) < abs((int)y1 - y0))
        {
            need_swap = true;
            std::swap(x0, y0);
            std::swap(x1, y1);
        }
        std::size_t left = std::min(x0, x1), right = std::max(x0, x1);
        std::size_t x = x0, y = y0;
        int flagx = x0 < x1 ? 1 : -1;
        int flagy = y0 < y1 ? 1 : -1;
        int dx = 2 * (x1 - x0) * flagx, dy = 2 * (y1 - y0) * flagy;
        int dydx = dy - dx, F = dy - dx / 2;
        while (left <= x && x <= right)
        {
            if (need_swap)
                canvas.SetAt({y, x}, color);
            else
                canvas.SetAt({x, y}, color);
            if (F < 0)
                F += dy;
            else
            {
                y += flagy;
                F += dydx;
            }
            x += flagx;
        }
    }

    /******************* 5. Triangle Drawing *****************/
    void DrawTriangleFilled(
        ImageRGB &       canvas,
        glm::vec3 const  color,
        glm::ivec2 const p0,
        glm::ivec2 const p1,
        glm::ivec2 const p2) {
        // your code here:
        glm::ivec2 q0 = p0, q1 = p1, q2 = p2;
        if (q0.x > q1.x) std::swap(q0, q1);
        if (q1.x > q2.x) std::swap(q1, q2);
        if (q0.x > q1.x) std::swap(q0, q1);
        float dy0 = 1.0f * (q2.y - q0.y) / (q2.x - q0.x);
        float dy1 = 1.0f * (q1.y - q0.y) / (q1.x - q0.x);
        for (std::size_t x = q0.x; x < q1.x && x < canvas.GetSizeX(); x++)
        {
            std::size_t y0 = floor(q0.y + dy0 * (x - q0.x) + 0.5);
            std::size_t y1 = floor(q0.y + dy1 * (x - q0.x) + 0.5);
            if (y0 > y1)
                std::swap(y0, y1);
            for (std::size_t y = y0; y < y1 && y < canvas.GetSizeY(); y++)
                canvas.SetAt({x, y}, color);
        }
        dy0 = 1.0f * (q2.y - q0.y) / (q2.x - q0.x);
        dy1 = 1.0f * (q2.y - q1.y) / (q2.x - q1.x);
        for (std::size_t x = q1.x; x <= q2.x && x < canvas.GetSizeX(); x++)
        {
            std::size_t y0 = floor(q2.y - dy0 * (q2.x - x) + 0.5);
            std::size_t y1 = floor(q2.y - dy1 * (q2.x - x) + 0.5);
            if (y0 > y1)
                std::swap(y0, y1);
            for (std::size_t y = y0; y < y1 && y < canvas.GetSizeY(); y++)
                canvas.SetAt({x, y}, color);
        }
    }

    /******************* 6. Image Supersampling *****************/
    void Supersample(
        ImageRGB &       output,
        ImageRGB const & input,
        int              rate) {
        // your code here:
        //printf("%d %d %d %d\n", input.GetSizeX(), input.GetSizeY(), output.GetSizeX(), output.GetSizeY());
        for (std::size_t x = 0; x < output.GetSizeX(); x++)
            for (std::size_t y = 0; y < output.GetSizeY(); y++)
            {
                glm::vec3 color = {0, 0, 0};
                float kx = 1.0f * input.GetSizeX() / output.GetSizeX();
                float ky = 1.0f * input.GetSizeY() / output.GetSizeY();
                float ix = x * kx;
                float iy = y * ky;
                for (int i = 0; i < rate; i++)
                    for (int j = 0; j < rate; j++)
                    {
                        float tx = ix - kx / rate * ((rate - 1) / 2.0f + i);
                        float ty = iy - ky / rate * ((rate - 1) / 2.0f + j);
                        tx = std::max(tx, 0.0f); tx = std::min(tx, input.GetSizeX() - 1.0f);
                        ty = std::max(ty, 0.0f); ty = std::min(ty, input.GetSizeY() - 1.0f);
                        color += input[{(std::size_t)floor(tx + 0.5), (std::size_t)floor(ty + 0.5)}] / (float)rate / float(rate);
                    }
                output.SetAt({x, y}, color);
            }
    }

    /******************* 7. Bezier Curve *****************/
    glm::vec2 CalculateBezierPoint(
        std::span<glm::vec2> points,
        float const          t) {
        // your code here:
        glm::vec2 *tpoints = new glm::vec2[points.size()];
        for(std::size_t i = 0; i < points.size(); i++)
            tpoints[i] = points[i];
        for(std::size_t i = 0; i < points.size(); i++)
            for(std::size_t j = 0; j < points.size() - i - 1; j++)
                tpoints[j] = (1 - t) * tpoints[j] + t * tpoints[j + 1];
        return tpoints[0];
    }
} // namespace VCX::Labs::Drawing2D