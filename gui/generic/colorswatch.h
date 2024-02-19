#ifndef FUZZYDROPLETS_GUI_GENERIC_COLORSWATCH_H
#define FUZZYDROPLETS_GUI_GENERIC_COLORSWATCH_H

#include <QWidget>
#include <QSet>

class ColorSwatch : public QWidget
{
    Q_OBJECT

public:

    ColorSwatch(bool multipleSelection, int patchSize , QWidget *parent = nullptr);
    ~ColorSwatch();

    int swatchSize() const {return m_swatchSize;}
    int spacing() const {return m_spacing;}
    void setSwatchSize(int size);
    void setSpacing(int spacing);

    int colorCount() const {return m_colors.size();}
    void setColors(const QList<QColor> & colors);
    void setRgbColors(const QList<QRgb> & colors);
    void addColor(QColor color);
    void removeColor(QColor color);
    void setColor(size_t pos, QColor newColor);

    QList<QColor> currentColors() const {QList<QColor> result; for (auto s : m_selection) result.append(m_colors[s]); return result;}
    QList<size_t> currentIndices() const {return m_selection;}
    void setCurrentIndex(size_t i);
    void setCurrentIndices(QList<size_t> i);
    void setCurrentColor(QColor color);
    void setCurrentColors(QList<QColor> colors);
    void clearSelection();

    QList<QColor> colors() const {return m_colors;}
    QList<QRgb> rgbColors() const;

    void setPaintEmptySwatches(bool b);
    void useDarkTheme(bool b);

    void setDoubleClickEnabled(bool b) {m_dblClickEnabled = b;}

    void setFixedColor(QColor color) {m_fixedColors.insert(color.rgba()); update();};
    void removeFixedColor(QColor color) {m_fixedColors.remove(color.rgba()); update();}
    void clearFixedColors() {m_fixedColors.clear(); update();}

protected:

    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);
    void changeEvent(QEvent * e);

public slots:

    void guiColorSchemeChanged();

signals:

    void selectionChanged(QColor color);
    void multiSelectionChanged(QList<QColor> colors);
    void colorReplaced(QColor oldColor, QColor newColor);

private:

    int swatchIdFromScreenPos(QPoint screenPos);
    void updateRowsAndColumns();

    int m_numRows {0};
    int m_numCols {0};
    int m_swatchSize {22};
    int m_spacing {6};
    QList<size_t> m_selection;
    bool m_paintEmptySwatches {true};
    QList<QColor> m_colors;
    QSet<QRgb> m_fixedColors;
    bool m_useDarkTheme {false};
    bool m_multipleSelection {false};
    bool m_dblClickEnabled {true};
};
#endif // FUZZYDROPLETS_GUI_GENERIC_COLORSWATCH_H
