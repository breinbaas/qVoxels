#ifndef CPTCHARTWIDGET_H
#define CPTCHARTWIDGET_H

#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QGraphicsRectItem>
#include "cpt.h"

class CptChartWidget : public QChartView
{
    Q_OBJECT
public:
    explicit CptChartWidget(QWidget *parent = nullptr);

    // Core function to display the line data
    void setCpt(Cpt* cpt);

    // Function to add colored intervals (like soil layers)
    //void addSoilLayer(double minQc, double maxQc, double topZ, double bottomZ, const QColor &color);

    // Call this if the window resizes to keep rectangles perfectly snapped
    //void updateRectangles();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QChart* m_chart;
    QLineSeries* m_cptSeries;
    QLineSeries* m_frSeries;
    QValueAxis* m_xAxis;
    QValueAxis* m_yAxis;
    QValueAxis* m_frXAxis;

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
