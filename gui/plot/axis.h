#ifndef FUZZYDROPLETS_GUI_PLOT_AXIS_H
#define FUZZYDROPLETS_GUI_PLOT_AXIS_H

#include "primitive.h"
#include <QPen>
#include <QFlags>
#include <bit>
#include <QFont>
#include <array>

namespace Plot
{

class Axis : public Primitive
{
public:

    enum Component
    {
        Line            = (1 << 0),
        Label           = (1 << 1),
        MajorTick       = (1 << 2),
        MediumTick      = (1 << 3),
        MinorTick       = (1 << 4),
        MajorTickLabel  = (1 << 5),
        MediumTickLabel = (1 << 6),
        MinorTickLabel  = (1 << 7)
    };
    Q_DECLARE_FLAGS(Components, Component)
    static const int ComponentCount = 8;

    enum Alignment
    {
        AlignLeft,
        AlignRight
    };

    Axis();
    virtual ~Axis() {}

    virtual void render(QPainter & painter);

    const QPen & pen(Component component) const {return m_pens[index(component)];}
    void setPen(Components components, const QPen & pen) {setProperty(m_pens, components, pen);}

    const bool & isVisible(Component component) const {return m_componentVisibility[index(component)];}
    void setVisibility(Components components, bool visible) {setProperty(m_componentVisibility, components, visible);}

    const Alignment & alignment(Component component) const {return m_alignment[index(component)];}
    void setAlignment(Components components, Alignment alignment) {setProperty(m_alignment, components, alignment);}
    void setAlignment(Alignment alignment) {m_alignment.fill(alignment);}

    Qt::Orientation orientation() const {return m_orientation;}
    void setOrientation(Qt::Orientation orientation) {m_orientation = orientation;}

    double padding() const {return m_padding;}
    void setPadding(double padding) {m_padding = padding;}

    const QString & label() const {return m_label;}
    void setLabel(const QString & label) {m_label = label; updateLabelBoundingRect(); updateLabelPixmap(m_devicePixelRatio);}

    double pixelLength() const {return m_pixelLength;}
    void setPixelLength(double pixelLength) {m_pixelLength = pixelLength;}

    const QFont & labelFont() const {return m_labelFont;}
    const QFont & majorTickLabelFont() const {return m_majorTickLabelFont;}
    const QFont & mediumTickLabelFont() const {return m_mediumTickLabelFont;}
    const QFont & minorTickLabelFont() const {return m_minorTickLabelFont;}
    void setLabelFont(const QFont & font) {m_labelFont = font; updateLabelBoundingRect(); updateLabelPixmap(m_devicePixelRatio);}
    void setMajorTickLabelFont(const QFont & font) {m_majorTickLabelFont = font;}
    void setMediumTickLabelFont(const QFont & font) {m_mediumTickLabelFont = font;}
    void setMinorTickLabelFont(const QFont & font) {m_minorTickLabelFont = font;}
    const QRectF & labelBoundingRect() const {return m_labelBoundingRect;}

    double majorTickLength() const {return m_majorTickLength;}
    double mediumTickLength() const {return m_mediumTickLength;}
    double minorTickLength() const {return m_minorTickLength;}
    void setMajorTickLength(double length) {m_majorTickLength = length;}
    void setMediumTickLength(double length) {m_mediumTickLength = length;}
    void setMinorTickLength(double length) {m_minorTickLength = length;}

    double leftOffset() const {return offset(AlignLeft);}
    double rightOffset() const {return offset(AlignRight);};

    void updateLabelPixmap(double devicePixelRatio);

protected:

    template <typename T>
    void setProperty(std::array<T, ComponentCount> & store, Components components, const T & value)
    {
        for (int i = 0; i < ComponentCount; ++i)
            if (components.testFlag(component(i)))
                store[i] = value;
    }

    size_t index(Component component) const {return std::countr_zero((size_t)component);}
    Component component(size_t index) const {return Component(1 << index);}
    void updateLabelBoundingRect();
    void setMajorTickLabelSize(const QSizeF & size) {m_majorTickLabelSize = size;}
    void setMediumTickLabelSize(const QSizeF & size) {m_mediumTickLabelSize = size;}
    void setMinorTickLabelSize(const QSizeF & size) {m_minorTickLabelSize = size;}
    const QSizeF & majorTickLabelSize() const {return m_majorTickLabelSize;}
    const QSizeF & mediumTickLabelSize() const {return m_mediumTickLabelSize;}
    const QSizeF & minorTickLabelSize() const {return m_minorTickLabelSize;}

private:

    double offset(Alignment a) const;

    Qt::Orientation m_orientation {Qt::Vertical};
    std::array<QPen, ComponentCount> m_pens;
    std::array<bool, ComponentCount> m_componentVisibility;
    std::array<Alignment, ComponentCount> m_alignment;

    QFont m_labelFont;
    QFont m_majorTickLabelFont;
    QFont m_mediumTickLabelFont;
    QFont m_minorTickLabelFont;

    QString m_label;
    QRectF m_labelBoundingRect {0,0,0,0};
    QPixmap m_labelPixmap;

    double m_majorTickLength {8};
    double m_mediumTickLength {4.5};
    double m_minorTickLength {2};

    QSizeF m_majorTickLabelSize {0,0};
    QSizeF m_mediumTickLabelSize {0,0};
    QSizeF m_minorTickLabelSize {0,0};

    double m_pixelLength {0};
    double m_padding {15};
    double m_devicePixelRatio {1};
};

} // namespace Plot

Q_DECLARE_OPERATORS_FOR_FLAGS(Plot::Axis::Components)

#endif // FUZZYDROPLETS_GUI_PLOT_AXIS_H
