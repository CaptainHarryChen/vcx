#include "Labs/5-Visualization/tasks.h"

#include <numbers>

using VCX::Labs::Common::ImageRGB;
namespace VCX::Labs::Visualization {

    const int property_num = 7;
    const glm::vec2 property_bar_start = {60, 70};
    const int property_bar_length = 800;
    const float property_bar_width = 3;
    const float property_bar_chosen_width = 15;
    const glm::vec4 property_bar_color = {0.7f, 0.7f, 0.7f, 1.0f};
    const glm::vec4 property_bar_hover_color = {0.0f, 0.7f, 0.0f, 1.0f};
    const glm::vec4 property_bar_chosen_color = {0.9f, 0.9f, 0.9f, 0.3f};
    const int property_bar_interval = 140;
    const glm::vec4 property_text_color = {0.7f, 0.7f, 0.7f, 1.0f};
    const int property_text_lineHeight = 18;
    const glm::vec2 property_text_offset = {0.0f, -1.0f * property_text_lineHeight};
    const char *property_text[] = {"mileage", "cylinders", "displacement", "horsepower", "weight", "acceleration", "year"};
    const int property_type[] = {1, 0, 1, 1, 1, 1, 0}; // 0: int, 1: float
    const float property_range[][2] = {{5, 51}, {2, 9}, {29, 494}, {27, 249}, {1260, 5493}, {6, 27}, {68, 84}};

    const glm::vec2 mark_offset = {0.0f, 15.0f};
    const float mark_radius = 7.0f;
    const glm::vec4 mark_color = {0.1f, 0.7f, 0.1f, 1.0f};
    
    const float data_line_width = 1.0f;
    const float data_line_alpha = 0.7f;
    // const glm::vec4 data_line_color = {0.95f, 0.95f, 0.95f, 0.7f};

    struct CoordinateStates {
        // your code here
        std::vector<float*> data;
        glm::vec3 jet[256];

        int chosen_property = 1;
        int hover_property = -1;
        
        bool has_updated = false;
        
        CoordinateStates(std::vector<Car> const &car_data) {
            for(auto car: car_data) {
                float *tmp = new float[7];
                tmp[0] = car.mileage;
                tmp[1] = car.cylinders;
                tmp[2] = car.displacement;
                tmp[3] = car.horsepower;
                tmp[4] = car.weight;
                tmp[5] = car.acceleration;
                tmp[6] = car.year;
                data.push_back(tmp);
            }
            MakeJetColor();
        }

        ~CoordinateStates() {
            for(auto car: data) {
                delete[] car;
            }
        }

        void MakeJetColor() {
            for(int i = 0; i < 32; i++)
                jet[i] = glm::vec3(0, 0, 128 + 4 * i) / 255.0f;
            jet[32] = glm::vec3(0, 0, 1);
            for(int i = 33; i < 96; i++)
                jet[i] = glm::vec3(0, (i - 32) * 4, 255) / 255.0f;
            jet[96] = glm::vec3(2, 255, 254) / 255.0f;
            for(int i = 97; i < 159; i++)
                jet[i] = glm::vec3(2 + 4 * (i - 96), 255, 254 - 4 * (i - 96)) / 255.0f;
            jet[159] = glm::vec3(254, 255, 1) / 255.0f;
            for(int i = 160; i < 224; i++)
                jet[i] = glm::vec3(255, 256 - 4 * (i - 159), 0) / 255.0f;
            for(int i = 224; i < 256; i++)
                jet[i] = glm::vec3(256 - 4 * (i - 223), 0, 0) / 255.0f;
        }

        glm::vec3 JetColor(float r) {
            int x = r * 255;
            return (jet[x] + glm::vec3(0.5f)) / 2.0f;
        }
    };

    bool PaintParallelCoordinates(Common::ImageRGB & input, InteractProxy const & proxy, std::vector<Car> const & data, bool force) {
        // your code here
        static CoordinateStates states(data);
        glm::vec2 canvas_size = {input.GetSizeX(), input.GetSizeY()};

        bool need_update = !states.has_updated;
        if(!need_update)
        {
            if(!proxy.IsHovering())
                return false;
            glm::vec2 mouse_pos = proxy.MousePos() * canvas_size;
            // printf("%.2f %.2f\n", mouse_pos.x, mouse_pos.y);
            int hover_property = -1;
            for(int i = 0; i < property_num; i++) {
                glm::vec2 st = property_bar_start + glm::vec2(1.0f * i * property_bar_interval, 0.0f);
                glm::vec2 ed = property_bar_start + glm::vec2(1.0f * i * property_bar_interval, 1.0f * property_bar_length);
                if(st.x - 15 <= mouse_pos.x && mouse_pos.x <= ed.x + 15 && st.y - 15 <= mouse_pos.y && mouse_pos.y <= ed.y + 15) {
                    hover_property = i;
                    break;
                }
            }
            if(hover_property != -1 || hover_property != states.hover_property)
                need_update = true;    
            states.hover_property = hover_property;
            if(proxy.IsClicking() && hover_property != -1)
                states.chosen_property = hover_property;
        }
        if(!need_update)
            return false;
        

        SetBackGround(input, glm::vec4(0.1f, 0.1f, 0.1f, 1.0f));

        // Draw data lines
        for(auto car: states.data) {
            glm::vec2 bar_st, bar_ed;
            glm::vec2 line_st, line_ed;
            float ratio0, ratio1, ratio2;
            ratio0 = (car[states.chosen_property] - property_range[states.chosen_property][0]) / (property_range[states.chosen_property][1] - property_range[states.chosen_property][0]);
            bar_st = property_bar_start / canvas_size;
            bar_ed = (property_bar_start + glm::vec2(0.0f, 1.0f * property_bar_length)) / canvas_size;
            ratio1 = (car[0] - property_range[0][0]) / (property_range[0][1] - property_range[0][0]);
            for(int i = 1; i < property_num; i++) {
                ratio2 = (car[i] - property_range[i][0]) / (property_range[i][1] - property_range[i][0]);
                line_st = bar_st * ratio1 + bar_ed * (1 - ratio1);
                line_ed = bar_st * ratio2 + bar_ed * (1 - ratio2) + glm::vec2(1.0f * property_bar_interval / canvas_size.x, 0.0f);
                DrawLine(input, glm::vec4(states.JetColor(ratio0), data_line_alpha), line_st, line_ed, data_line_width);
                ratio1 = ratio2;
                bar_st.x += property_bar_interval / canvas_size.x;
                bar_ed.x += property_bar_interval / canvas_size.x;
            }
        }

        // Draw property bars
        for(int i = 0; i < property_num; i++) {
            glm::vec2 st = (property_bar_start + glm::vec2(1.0f * i * property_bar_interval, 0.0f)) / canvas_size;
            glm::vec2 ed = (property_bar_start + glm::vec2(1.0f * i * property_bar_interval, 1.0f * property_bar_length)) / canvas_size;
            glm::vec2 text_pos = (property_bar_start + glm::vec2(1.0f * i * property_bar_interval, 0.0f) + property_text_offset) / canvas_size;
            if (states.chosen_property == i)
            {
                glm::vec2 offset = glm::vec2(property_bar_chosen_width / 2 - property_bar_width / 2, 0.0f) / canvas_size;
                DrawLine(input, property_bar_chosen_color, st + offset, ed + offset, property_bar_chosen_width);
            }
            if (states.hover_property == i) {
                DrawLine(input, property_bar_hover_color, st, ed, property_bar_width);
                char num[10];
                sprintf(num, "%.0f", (proxy.MousePos().y * canvas_size.y - property_bar_start.y) / property_bar_length * (property_range[i][1] - property_range[i][0]) + property_range[i][0]);
                PrintText(input, {1.0f, 1.0f, 1.0f, 1.0f}, proxy.MousePos() + glm::vec2(20, -10) / canvas_size, property_text_lineHeight / canvas_size[1], num);
            }
            else
                DrawLine(input, property_bar_color, st, ed, property_bar_width);
            PrintText(input, property_text_color, text_pos, property_text_lineHeight / canvas_size[1], property_text[i]);
        }
        
        states.has_updated = true;

        return true;
    }

    void LIC(ImageRGB & output, Common::ImageRGB const & noise, VectorField2D const & field, int const & step) {
        // your code here
    }
}; // namespace VCX::Labs::Visualization