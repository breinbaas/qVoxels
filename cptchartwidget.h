#ifndef CPTCHARTWIDGET_H
#define CPTCHARTWIDGET_H

#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QGraphicsRectItem>
#include "cpt.h"
#include "soilcolorpalette.h"

class CptChartWidget : public QChartView
{
    Q_OBJECT
public:
    explicit CptChartWidget(QWidget *parent = nullptr);
    void setCpt(Cpt* cpt);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QChart* m_chart;
    QLineSeries* m_cptSeries;
    QLineSeries* m_frSeries;
    QValueAxis* m_xAxis;
    QValueAxis* m_yAxis;
    QValueAxis* m_frXAxis;

    Cpt* m_currentCpt = nullptr;
    SoilColorPalette m_palette;

    // Struct to remember abstract graph coordinates for layers so we can reposition them on resize
    struct LayerData {
        double minQc;
        double maxQc;
        double topZ;
        double bottomZ;
        QColor color;
        QGraphicsRectItem* visualRect;
    };
    QList<LayerData> m_layers;
};

#endif // CPTCHARTWIDGET_H
