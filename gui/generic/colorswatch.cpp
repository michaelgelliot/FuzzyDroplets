#include "colorswatch.h"
#include <QPainter>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QStyleHints>
#include <QColorDialog>

ColorSwatch::ColorSwatch(bool multipleSelection, int patchSize, QWidget *parent)
    : QWidget(parent),
      m_swatchSize(patchSize),
    m_multipleSelection(multipleSelection)
{
    guiColorSchemeChanged();
    setMouseTracking(true);
    updateRowsAndColumns();

    connect(qGuiApp->styleHints(), &QStyleHints::colorSchemeChanged, this, &ColorSwatch::guiColorSchemeChanged, Qt::QueuedConnection);
}

ColorSwatch::~ColorSwatch()
{
}

void ColorSwatch::guiColorSchemeChanged()
{
    useDarkTheme(qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark);
}

void ColorSwatch::useDarkTheme(bool b)
{
    m_useDarkTheme = b;
    setPalette(qGuiApp->palette());
    update();
}

void ColorSwatch::setSwatchSize(int size)
{
    m_swatchSize = size;
    updateRowsAndColumns();
    update();
}

void ColorSwatch::setSpacing(int spacing)
{
    m_spacing = spacing;
    updateRowsAndColumns();
    update();
}

void ColorSwatch::setColors(const QList<QColor> & colors)
{
    m_colors = colors;
    m_selection.clear();
    updateRowsAndColumns();
    update();
    emit selectionChanged(QColor());
}

void ColorSwatch::setRgbColors(const QList<QRgb> & colors)
{
    QList<QColor> c(colors.begin(), colors.end());
    setColors(c);
}

void ColorSwatch::setCurrentIndex(size_t i)
{
    if (i >= (size_t)m_colors.size()) {
        clearSelection();
    } else {
        m_selection = {i};
        update();
        emit selectionChanged(m_colors[i]);
    }
}

void ColorSwatch::setCurrentIndices(QList<size_t> i)
{
    if (i.size() == 0) {
        clearSelection();
    } else {
        m_selection = i;
        update();
        emit multiSelectionChanged(currentColors());
    }
}

void ColorSwatch::setCurrentColor(QColor color)
{
    int i = m_colors.indexOf(color);
    if (i == -1) {
        clearSelection();
        emit selectionChanged(QColor());
    } else {
        setCurrentIndex(i);
        emit selectionChanged(color);
    }
}

void ColorSwatch::setCurrentColors(QList<QColor> colors)
{
    QList<size_t> idx;
    for (auto col : colors) {
        auto i = m_colors.indexOf(col);
        if (i >= 0)
            idx.append(i);
    }
    setCurrentIndices(idx);
    emit multiSelectionChanged(colors);
}

void ColorSwatch::clearSelection()
{
    m_selection.clear();
    update();
    emit selectionChanged(QColor());
}

void ColorSwatch::setPaintEmptySwatches(bool b)
{
    m_paintEmptySwatches = b;
    update();
}

void ColorSwatch::updateRowsAndColumns()
{
    m_numCols = std::max(1, (width() - 2 + m_spacing) / (m_swatchSize + m_spacing));
    m_numRows = m_colors.size() / m_numCols + ((m_colors.size() % m_numCols > 0) ? 1 : 0);
    if (m_numRows == 0) m_numRows = 1;
    setMinimumHeight(2 + m_numRows * m_swatchSize + (m_numRows - 1) * m_spacing);
}

void ColorSwatch::addColor(QColor color)
{
    m_colors.append(color);
    updateRowsAndColumns();
    update();
}

void ColorSwatch::removeColor(QColor color)
{
    int pos = m_colors.indexOf(color);
    if (pos > 0) {
        m_colors.removeAll(color);
        //m_selection.removeAll(pos);
        updateRowsAndColumns();
        update();
        if (m_colors.size() > 0) {
            if (m_multipleSelection) {
                if (m_selection.size() > 0) {
                    for (auto & sel : m_selection) {
                        if (sel >= pos) --sel;
                    }
                }
                emit multiSelectionChanged(currentColors());
            } else if (m_selection.size() > 0) {
                if (m_selection[0] >= (size_t)m_colors.size() && m_colors.size() > 0) {
                    m_selection[0] = m_colors.size() - 1;
                }
                emit selectionChanged(m_colors[m_selection[0]]);
            }
        } else {
            if (m_multipleSelection) {
                emit multiSelectionChanged(QList<QColor>());
            } else {
                emit selectionChanged(QColor());
            }
        }
    }
}

void ColorSwatch::setColor(size_t pos, QColor newColor)
{
    m_colors[pos] = newColor;
    if (m_selection.contains(pos)) {
        if (m_multipleSelection) {
            emit multiSelectionChanged(currentColors());
        } else {
            emit selectionChanged(newColor);
        }
    }
    update();
}

int ColorSwatch::swatchIdFromScreenPos(QPoint screenPos)
{
    int x = screenPos.x() / (m_swatchSize + m_spacing);
    int zx = screenPos.x() - x * (m_swatchSize + m_spacing);
    int y = screenPos.y() / (m_swatchSize + m_spacing);
    int zy = screenPos.y() - y * (m_swatchSize + m_spacing);
    int i = y * m_numCols + x;
    if (x < m_numCols && y < m_numRows && zx <= m_swatchSize && zy <= m_swatchSize && i < m_colors.size())
        return i;
    else
        return -1;
}

void ColorSwatch::resizeEvent(QResizeEvent *)
{
    updateRowsAndColumns();
}

void ColorSwatch::changeEvent(QEvent * e)
{
    if (e->type() == QEvent::EnabledChange) {
        update();
    }
}

void ColorSwatch::paintEvent(QPaintEvent * e)
{
    QPainter paint(this);
    paint.setRenderHint(QPainter::Antialiasing);
    paint.fillRect(rect(), palette().window().color());
    int i = 0, x = 1, y = 1;
    for (int row = 0; row < m_numRows; ++row) {
        x = 1;
        for (int col = 0; col < m_numCols; ++col) {
            if (m_selection.contains(i) && i < m_colors.size()) {
                paint.setPen(m_useDarkTheme ? m_colors[i].lighter(190) : m_colors[i].darker(190));
                paint.setBrush(m_colors[i]);
                paint.drawRoundedRect(x, y, m_swatchSize, m_swatchSize, 3, 3);
                paint.setPen(m_colors[i] == Qt::white ? Qt::black : Qt::white);
                paint.drawRoundedRect(x + 1, y + 1, m_swatchSize - 2, m_swatchSize - 2, 2.5, 2.5);
            } else {
                if (i < m_colors.size() || m_paintEmptySwatches) {
                    if (m_useDarkTheme) {
                        paint.setPen(i >= m_colors.size() ? QColor(255,255,255,40) : m_colors[i].lighter(140));
                        paint.setBrush(i >= m_colors.size() ? QColor(255,255,255,7) : m_colors[i]);
                    } else {
                        paint.setPen(i >= m_colors.size() ? QColor(0,0,0,40) : m_colors[i].darker(140));
                        paint.setBrush(i >= m_colors.size() ? QColor(0,0,0,7) : m_colors[i]);
                    }
                    paint.drawRoundedRect(x, y, m_swatchSize, m_swatchSize, 3, 3);
                }
            }
            if (!isEnabled() && (i < m_colors.size() || m_paintEmptySwatches)) {
                auto mode = paint.compositionMode();
                paint.setCompositionMode(QPainter::CompositionMode_Screen);
                paint.setBrush(m_useDarkTheme ? QColor(0,0,0,120) : QColor(255,255,255,120));
                paint.setPen(m_useDarkTheme ? QColor(0,0,0,120) : QColor(255,255,255,120));
                paint.drawRoundedRect(x, y, m_swatchSize, m_swatchSize, 3, 3);
                paint.setCompositionMode(mode);
            }
            if (i < m_colors.size() && m_fixedColors.contains(m_colors[i].rgba())) {
                paint.setPen(QPen(Qt::red, 1));
                paint.drawLine(QPoint(x + 6, y + 5.5), QPoint(x + m_swatchSize - 4.5, y + m_swatchSize - 4.5));// todo ensure this is consistent with swatch size...
                paint.drawLine(QPoint(x + m_swatchSize - 4.5, y + 5.5), QPoint(x+6, y + m_swatchSize - 4.5));
            }
            ++i;
            x += m_swatchSize + m_spacing;
        }
        y += m_swatchSize + m_spacing;
    }
}

void ColorSwatch::mouseMoveEvent(QMouseEvent * e)
{
    setCursor((swatchIdFromScreenPos(e->pos()) >= 0) ? Qt::PointingHandCursor : Qt::ArrowCursor);
}

void ColorSwatch::mousePressEvent(QMouseEvent * e)
{
    int i = swatchIdFromScreenPos(e->pos());
    if (i >= 0 && i < m_colors.size()) {
        if (m_multipleSelection) {
            if (!m_selection.contains(i)) {
                m_selection.append(i);
            } else {
                m_selection.removeAll(i);
            }
            emit multiSelectionChanged(currentColors());
        } else {
            if (m_selection.size() > 0 && m_selection[0] == i) {
                clearSelection();
                emit selectionChanged(QColor());
            } else {
                m_selection = {(size_t)i};
                emit selectionChanged(m_colors[i]);
            }
        }
    } else if (!m_multipleSelection) {
        m_selection.clear();
        emit selectionChanged(QColor());
    }
    update();
}

void ColorSwatch::mouseDoubleClickEvent(QMouseEvent * e)
{
    if (!m_dblClickEnabled) return;
    int i = swatchIdFromScreenPos(e->pos());
    if (m_fixedColors.contains(m_colors[i].rgba())) return;
    if (i >= 0 && i < m_colors.size()) {
        m_selection = {(size_t)i};
        QColor c = QColorDialog::getColor(m_colors[i]);
        if (c.isValid()) {
            QColor oldCol = m_colors[i];
            setColor(i, c);
            emit colorReplaced(oldCol, c);
        }
        if (m_multipleSelection) {
            emit multiSelectionChanged(currentColors());
        } else {
            emit selectionChanged(m_colors[i]);
        }
        update();
    }
}

QList<QRgb> ColorSwatch::rgbColors() const
{
    QList<QRgb> result;
    for (int i = 0; i < m_colors.size(); ++i) {
        result.append(m_colors[i].rgb());
    }
    return result;
}
