#include "Labs/5-Visualization/Data.h"


namespace VCX::Labs::Visualization {

    glm::vec2 InteractProxy::MousePos() const {
        return _pos;
    }

    glm::vec2 InteractProxy::MouseDeltaPos() const {
        return _dPos;
    }

    bool InteractProxy::IsHovering() const {
        return _hover;
    }

    bool InteractProxy::IsClicking(bool left) const {
        return left ? _lClick : _rClick;
    }

    bool InteractProxy::IsDragging(bool left) const {
        return (left ? _lDrag : _rDrag) && _move;
    }

    glm::vec2 InteractProxy::DraggingStartPoint(bool left) const {
        return left ? _lStart : _rStart;
    }

    void InteractProxy::Update(ImVec2 const & size, ImVec2 const & pos, ImVec2 const & delta, bool hover, bool lHeld, bool rHeld) {
        if (! _lLastDrag && _lDrag) _lStart = glm::vec2(pos.x / size.x, pos.y / size.y);
        if (! _rLastDrag && _rDrag) _rStart = glm::vec2(pos.x / size.x, pos.y / size.y);
        _pos       = glm::vec2(pos.x / size.x, pos.y / size.y);
        _dPos      = glm::vec2(delta.x / size.x, delta.y / size.y);
        _hover     = hover && _pos.x <= 1.f && _pos.y <= 1.f && _pos.x >= 0.f && _pos.y >= 0.f;
        _move      = delta.x != 0 || delta.y != 0;
        _lClick    = _lLastDrag && ! _lDrag && _lStart == _pos;
        _rClick    = _rLastDrag && ! _rDrag && _rStart == _pos;
        _lLastDrag = _lDrag;
        _rLastDrag = _rDrag;
        _lDrag     = lHeld;
        _rDrag     = rHeld;
    }

    void SetBackGround(Common::ImageRGB & canvas, glm::vec4 color) {
        canvas.Fill(glm::vec3(color));
    }

    void DrawLine(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 from, glm::vec2 to, float width) {
        width        = std::max(1.0f, width * canvas.GetSizeX() * 0.001f);
        int   y1     = static_cast<int>(std::floor(to.y * (canvas.GetSizeY() - 1)));
        int   x1     = static_cast<int>(std::floor(to.x * (canvas.GetSizeX() - 1)));
        int   y0     = static_cast<int>(std::floor(from.y * (canvas.GetSizeY() - 1)));
        int   x0     = static_cast<int>(std::floor(from.x * (canvas.GetSizeX() - 1)));
        int   deltaX = std::abs(x1 - x0);
        int   deltaY = std::abs(y1 - y0);
        int   stepX  = x0 < x1 ? 1 : -1;
        int   stepY  = y0 < y1 ? 1 : -1;
        int   err    = deltaX - deltaY;
        float ed     = (deltaX + deltaY == 0) ? 1 : std::sqrt(deltaX * deltaX + deltaY * deltaY);

        while (true) {
            if (static_cast<std::size_t>(y0) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(x0) < canvas.GetSizeX() - 1) {
                auto && proxy = canvas.At(x0, y0);
                proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
            }

            int e2 = err;
            int x2 = x0;
            if (2 * e2 >= -deltaX) {
                e2 += deltaY;
                int y2 = y0;
                while (e2 < ed * width && (y1 != y2 || deltaX > deltaY)) {
                    y2 += stepY;
                    if (static_cast<std::size_t>(y2) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(x0) < canvas.GetSizeX() - 1) {
                        auto && proxy = canvas.At(x0, y2);
                        proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                    }
                    e2 += deltaX;
                }
                if (x0 == x1) break;
                e2 = err;
                err -= deltaY;
                x0 += stepX;
            }
            if (2 * e2 <= deltaY) {
                e2 = deltaX - e2;
                while (e2 < ed * width && (x1 != x2 || deltaX < deltaY)) {
                    x2 += stepX;
                    if (static_cast<std::size_t>(y0) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(x2) < canvas.GetSizeX() - 1) {
                        auto && proxy = canvas.At(x2, y0);
                        proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                    }
                    e2 += deltaY;
                }
                if (y0 == y1) break;
                err += deltaX;
                y0 += stepY;
            }
        }
    }

    void DrawRect(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 leftTop, glm::vec2 size, float width) {
        width                 = std::max(1.0f, width * canvas.GetSizeX() * 0.001f);
        glm::vec2 rightBottom = leftTop + size;
        int       halfWidth   = (width < 1) ? 0 : static_cast<int>(glm::round(width * 0.5f));
        int       y1          = static_cast<int>(std::floor(rightBottom.y * (canvas.GetSizeY() - 1)));
        int       x1          = static_cast<int>(std::floor(rightBottom.x * (canvas.GetSizeX() - 1)));
        int       y0          = static_cast<int>(std::floor(leftTop.y * (canvas.GetSizeY() - 1)));
        int       x0          = static_cast<int>(std::floor(leftTop.x * (canvas.GetSizeX() - 1)));
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        for (int x = x0; x <= x1; ++x) {
            for (int y = y0 - halfWidth; y <= y0 + halfWidth; ++y) {
                if (static_cast<std::size_t>(y) >= canvas.GetSizeY() - 1 || static_cast<std::size_t>(x) >= canvas.GetSizeX() - 1) continue;
                auto && proxy = canvas.At(x, y);
                proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
            }
            for (int y = std::max(y0 + halfWidth + 1, y1 - halfWidth); y <= y1 + halfWidth; ++y) {
                if (static_cast<std::size_t>(y) >= canvas.GetSizeY() - 1 || static_cast<std::size_t>(x) >= canvas.GetSizeX() - 1) continue;
                auto && proxy = canvas.At(x, y);
                proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
            }
        }
        for (int y = y0 + halfWidth + 1; y < y1 - halfWidth; ++y) {
            for (int x = x0 - halfWidth; x <= x0 + halfWidth; ++x) {
                if (static_cast<std::size_t>(y) >= canvas.GetSizeY() - 1 || static_cast<std::size_t>(x) >= canvas.GetSizeX() - 1) continue;
                auto && proxy = canvas.At(x, y);
                proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
            }
            for (int x = std::max(x0 + halfWidth + 1, x1 - halfWidth); x <= x1 + halfWidth; ++x) {
                if (static_cast<std::size_t>(y) >= canvas.GetSizeY() - 1 || static_cast<std::size_t>(x) >= canvas.GetSizeX() - 1) continue;
                auto && proxy = canvas.At(x, y);
                proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
            }
        }
    }

    void DrawFilledRect(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 leftTop, glm::vec2 size) {
        glm::vec2 rightBottom = leftTop + size;
        int       y1          = static_cast<int>(std::floor(rightBottom.y * (canvas.GetSizeY() - 1)));
        int       x1          = static_cast<int>(std::floor(rightBottom.x * (canvas.GetSizeX() - 1)));
        int       y0          = static_cast<int>(std::floor(leftTop.y * (canvas.GetSizeY() - 1)));
        int       x0          = static_cast<int>(std::floor(leftTop.x * (canvas.GetSizeX() - 1)));
        if (x0 > x1) std::swap(x0, x1);
        if (y0 > y1) std::swap(y0, y1);
        for (int x = x0; x <= x1; ++x) {
            for (int y = y0; y <= y1; ++y) {
                if (static_cast<std::size_t>(y) >= canvas.GetSizeY() - 1 || static_cast<std::size_t>(x) >= canvas.GetSizeX() - 1) continue;
                auto && proxy = canvas.At(x, y);
                proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
            }
        }
    }


    void DrawCircle(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 origin, float radius, float width) {
        width   = std::max(1.0f, width * canvas.GetSizeX() * 0.001f);
        int y0  = static_cast<int>(std::floor(origin.y * (canvas.GetSizeY() - 1)));
        int x0  = static_cast<int>(std::floor(origin.x * (canvas.GetSizeX() - 1)));
        int r1  = static_cast<int>(std::round(radius * (canvas.GetSizeX() - 1) - width * 0.5f));
        int y1  = r1;
        int D1  = 5 - 4 * r1;
        int r2  = static_cast<int>(std::round(radius * (canvas.GetSizeX() - 1) + width * 0.5f));
        int x2  = 0;
        int y2  = r2;
        int D2  = 5 - 4 * r2;
        for (int y = y1; y <= y2; ++y) {
            for (auto yy : std::array<int, 2>({ y0 + y, y0 - y })) {
                if (static_cast<std::size_t>(yy) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(x0) < canvas.GetSizeX() - 1) {
                    auto && proxy = canvas.At(x0, yy);
                    proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                }
                if (x2 != y && static_cast<std::size_t>(x0) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(yy) < canvas.GetSizeX() - 1) {
                    auto && proxy = canvas.At(yy, x0);
                    proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                }
                if (y == 0) break;
            }
        }
        while (y2 > x2) {
            x2++;
            if (D2 > 0) {
                y2--;
                D2 += 8 * (x2 - y2) + 4;
            } else D2 += 8 * x2 + 4;
            if (y1 < x2) {
                y1 = x2;
            } else if (D1 > 0) {
                y1--;
                D1 += 8 * (x2 - y1) + 4;
            } else D1 += 8 * x2 + 4;
            for (auto xx : std::array<int, 2>({ x0 + x2, x0 - x2 })) {
                for (int y = y1; y <= y2; ++y) {
                    for (auto yy : std::array<int, 2>({ y0 + y, y0 - y })) {
                        if (static_cast<std::size_t>(yy) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(xx) < canvas.GetSizeX() - 1) {
                            auto && proxy = canvas.At(xx, yy);
                            proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                        }
                        if (x2 != y && static_cast<std::size_t>(xx) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(yy) < canvas.GetSizeX() - 1) {
                            auto && proxy = canvas.At(yy, xx);
                            proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                        }
                        if (y == 0) break;
                    }
                }
                if (x2 == 0) break;
            }
        }
    }

    void DrawFilledCircle(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 origin, float radius) {
        int y0 = static_cast<int>(std::floor(origin.y * (canvas.GetSizeY() - 1)));
        int x0 = static_cast<int>(std::floor(origin.x * (canvas.GetSizeX() - 1)));
        int r2 = static_cast<int>(std::round(radius * (canvas.GetSizeX() - 1)));
        int x2 = 0;
        int y2 = r2;
        int D2 = 5 - 4 * r2;
        for (int y = x2; y <= y2; ++y) {
            for (auto yy : std::array<int, 2>({ y0 + y, y0 - y })) {
                if (static_cast<std::size_t>(yy) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(x0) < canvas.GetSizeX() - 1) {
                    auto && proxy = canvas.At(x0, yy);
                    proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                }
                if (x2 != y && static_cast<std::size_t>(x0) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(yy) < canvas.GetSizeX() - 1) {
                    auto && proxy = canvas.At(yy, x0);
                    proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                }
                if (y == 0) break;
            }
        }
        while (y2 > x2) {
            x2++;
            if (D2 > 0) {
                y2--;
                D2 += 8 * (x2 - y2) + 4;
            } else D2 += 8 * x2 + 4;
            for (auto xx : std::array<int, 2>({ x0 + x2, x0 - x2 })) {
                for (int y = x2; y <= y2; ++y) {
                    for (auto yy : std::array<int, 2>({ y0 + y, y0 - y })) {
                        if (static_cast<std::size_t>(yy) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(xx) < canvas.GetSizeX() - 1) {
                            auto && proxy = canvas.At(xx, yy);
                            proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                        }
                        if (x2 != y && static_cast<std::size_t>(xx) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(yy) < canvas.GetSizeX() - 1) {
                            auto && proxy = canvas.At(yy, xx);
                            proxy         = glm::vec3(color) * color.a + static_cast<glm::vec3>(proxy) * (1.0f - color.a);
                        }
                        if (y == 0) break;
                    }
                }
                if (x2 == 0) break;
            }
        }
    }

    bool PrintText(Common::ImageRGB & canvas, glm::vec4 color, glm::vec2 pos, float lineHeight, std::string const & caption) {
        lineHeight           = lineHeight * (canvas.GetSizeY() - 1);
        auto &         io    = ImGui::GetIO();
        std::uint8_t * tex   = nullptr;
        int            w     = 0;
        int            h     = 0;
        float          x     = std::floor(pos.x * (canvas.GetSizeX() - 1));
        float          y     = std::floor(pos.y * (canvas.GetSizeY() - 1)) - lineHeight * 0.5f;
        float          scale = lineHeight * 0.05f;
        io.Fonts->GetTexDataAsAlpha8(&tex, &w, &h, nullptr);
        for (std::size_t s = 0; s < caption.length(); ++s) {
            char c = caption[s];
            if (! (c >= 32 && c < 0x80)) return false;
            ImFontGlyph const * glyph = io.Fonts->Fonts[0]->FindGlyph(c);
            x -= scale * glyph->AdvanceX * 0.5f;
        }
        for (std::size_t s = 0; s < caption.length(); ++s) {
            char c = caption[s];
            ImFontGlyph const * glyph = io.Fonts->Fonts[0]->FindGlyph(c);
            if (glyph == NULL) return false;
            if (glyph->Visible) {
                float dU        = (glyph->U1 - glyph->U0) / ((glyph->X1 - glyph->X0) * scale);
                float dV        = (glyph->V1 - glyph->V0) / ((glyph->Y1 - glyph->Y0) * scale);
                float x1        = x + glyph->X0 * scale;
                float x2        = x + glyph->X1 * scale;
                float y1        = y + glyph->Y0 * scale;
                float y2        = y + glyph->Y1 * scale;
                for (int xx = static_cast<int>(std::ceil(x1)); xx < x2; ++xx) {
                    for (int yy = static_cast<int>(std::ceil(y1)); yy < y2; ++yy) {
                        if (static_cast<std::size_t>(yy) < canvas.GetSizeY() - 1 && static_cast<std::size_t>(xx) < canvas.GetSizeX() - 1) {
                            float     u      = (glyph->U0 + (xx - x1) * dU) * (w - 1);
                            float     v      = (glyph->V0 + (yy - y1) * dV) * (h - 1);
                            int       u0     = static_cast<int>(std::floor(u));
                            int       v0     = static_cast<int>(std::floor(v));
                            u                = u - u0;
                            v                = v - v0;
                            glm::vec4 sp     = glm::vec4(tex[v0 * w + u0], tex[v0 * w + u0 + 1], tex[(v0 + 1) * w + u0], tex[(v0 + 1) * w + u0 + 1]);
                            glm::vec4 ws     = glm::vec4((1 - u) * (1 - v), u * (1 - v), v * (1 - u), u * v);
                            float     weight = glm::dot(sp, ws) * color.a / 255.f;
                            auto &&   proxy  = canvas.At(xx, yy);
                            proxy            = glm::vec3(color) * weight + static_cast<glm::vec3>(proxy) * (1.0f - weight);
                        }
                    }
                }
            }
            x += scale * glyph->AdvanceX;
        }
        return true;
    }

} // namespace VCX::Labs::Visualization