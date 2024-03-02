#ifndef FUZZYDROPLETS_GUI_PLOT_BOXGRAPHBASE_H
#define FUZZYDROPLETS_GUI_PLOT_BOXGRAPHBASE_H

#include "primitive.h"
#include <QWidget>

namespace Plot
{

class Axis;

class BoxGraphBase : public QWidget, public Primitive
{
    Q_OBJECT

public:

    explicit BoxGraphBase(QWidget *parent = nullptr);
    virtual ~BoxGraphBase();

    double padding() const {return m_padding;}
    void setPadding(double padding) {m_padding = padding; update();}

    const QBrush & backgroundBrush() const {return m_backgroundBrush;}
    void setBackgroundBrush(const QBrush & brush) {m_backgroundBrush = brush; update();}

    const QBrush & viewportBrush() const {return m_viewportBrush;}
    void setViewportBrush(const QBrush & brush) {m_viewportBrush = brush; update();}

    void setDarkTheme();
    void setLightTheme();
    void setUserTheme();

    QRectF viewportRect() const;

    void recalculateLayout();

    void addStaticPrimitive(Primitive * primitive);
    void removeStaticPrimitive(Primitive * primitive);
    QList<Primitive*> & staticPrimitives() {return m_staticPrimitives;}

    void addDynamicPrimitive(Primitive * primitive);
    void removeDynamicPrimitive(Primitive * primitive);
    QList<Primitive*> & dynamicPrimitives() {return m_dynamicPrimitives;}

    virtual void render(QPainter & paint, bool highQualityOutput = false) override;
    virtual void updateStaticPrimitives(const QRectF & clipRect = QRectF());

    QSize bufferSize() const {return m_staticPixmap.size();}

    const Axis * leftAxis() const {return m_leftAxis;}
    const Axis * rightAxis() const {return m_rightAxis;}
    const Axis * topAxis() const {return m_topAxis;}
    const Axis * bottomAxis() const {return m_bottomAxis;}

    Axis * leftAxis() {return m_leftAxis;}
    Axis * rightAxis() {return m_rightAxis;}
    Axis * topAxis() {return m_topAxis;}
    Axis * bottomAxis() {return m_bottomAxis;}

    QImage image(double scale);

protected:

    void setLeftAxis(Axis * axis);
    void setRightAxis(Axis * axis);
    void setTopAxis(Axis * axis);
    void setBottomAxis(Axis * axis);

public slots:

    void guiColorSchemeChanged();

protected:

    virtual void resizeEvent(QResizeEvent * e) override;
    virtual void paintEvent(QPaintEvent *) override;

private:

    Axis * m_leftAxis   {nullptr};
    Axis * m_rightAxis  {nullptr};
    Axis * m_topAxis    {nullptr};
    Axis * m_bottomAxis {nullptr};
    double m_padding    {25};

    QBrush m_backgroundBrush {Qt::white};
    QBrush m_viewportBrush {Qt::white};

    QList<Primitive*> m_staticPrimitives;
    QList<Primitive*> m_dynamicPrimitives;

    QPixmap m_staticPixmap;
};

} // namespace Plot

#endif // FUZZYDROPLETS_GUI_PLOT_BOXGRAPHBASE_H
