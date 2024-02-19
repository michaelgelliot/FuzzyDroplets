#ifndef FUZZYDROPLETS_GUI_PLOT_PRIMITIVE_H
#define FUZZYDROPLETS_GUI_PLOT_PRIMITIVE_H

#include <QPalette>
#include <algorithm>

namespace Plot
{

class Primitive
{
public:

    Primitive() {}
    virtual ~Primitive() {}

    bool isVisible() const {return m_visible;}
    void setVisibility(bool visible) {m_visible = visible;}

    bool autoColor() const {return m_autoColor;}
    void setAutoColor(bool autoColor) {m_autoColor = autoColor;}

    bool clipToViewport() const {return m_clipToViewport;}
    void setClipToViewport(bool clip) {m_clipToViewport = clip;}

    bool antialiased() const {return m_antialiased;}
    void setAntialiased(bool antialiased) {m_antialiased = antialiased;}

    virtual void render(QPainter & painter) = 0;

protected:

    void apply(QPainter & painter, const QPen & pen, QPalette::ColorRole colorRole);
    void apply(QPainter & painter, const QBrush & brush, QPalette::ColorRole colorRole);
    void apply(QPainter & painter, const QPen & pen, const QBrush & brush, QPalette::ColorRole penColorRole, QPalette::ColorRole brushColorRole);

private:

    bool m_visible {true};
    bool m_autoColor {true};
    bool m_clipToViewport {true};
    bool m_antialiased {true};
};

} // namespace Plot

#endif // FUZZYDROPLETS_GUI_PLOT_PRIMITIVE_H
