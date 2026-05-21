#include "cptchartwidget.h"
#include <QGraphicsScene>

CptChartWidget::CptChartWidget(QWidget *parent)
    : QChartView(parent)
    , m_chart(new QChart())
    , m_cptSeries(new QLineSeries())
    , m_frSeries(new QLineSeries())
    , m_xAxis(new QValueAxis())
    , m_yAxis(new QValueAxis())
    , m_frXAxis(new QValueAxis())
{
    m_chart->addSeries(m_cptSeries);
    m_chart->addSeries(m_frSeries);
    m_chart->setTitle("CPT Profile");
    m_chart->legend()->hide();

    m_cptSeries->setColor(QColor(0, 0, 255)); // Blue for qc
    m_frSeries->setColor(QColor(169, 169, 169));

    // Configure Axes
    m_xAxis->setTitleText("qc (MPa)");
    m_yAxis->setTitleText("Depth / Elevation (z)");

    m_frXAxis->setTitleText("Friction Ratio Fr (%)");
    m_frXAxis->setRange(0.0, 20.0);
    m_frXAxis->setLabelsColor(QColor(169, 169, 169));
    //m_frXAxis->setTitleColor(QColor(220, 20, 60));
    m_frXAxis->setGridLineVisible(false);


    // IMPORTANT FOR GEOTECHNICAL LOGS:
    // If your Z represents positive depth downward, leave it. If your Z values represent
    // negative depth values, you may want to call m_yAxis->setReverse(true); to force
    // deeper values downward on the chart space.
    m_yAxis->setReverse(false);

    m_chart->addAxis(m_xAxis, Qt::AlignBottom);
    m_chart->addAxis(m_frXAxis, Qt::AlignTop);
    m_chart->addAxis(m_yAxis, Qt::AlignLeft);

    m_cptSeries->attachAxis(m_xAxis);
    m_cptSeries->attachAxis(m_yAxis);

    m_frSeries->attachAxis(m_frXAxis); // Maps explicitly to the top axis
    m_frSeries->attachAxis(m_yAxis);

    setChart(m_chart);
    setRenderHint(QPainter::Antialiasing);
}

void CptChartWidget::setCpt(Cpt* cpt)
{
    if (!cpt) return;

    m_chart->setTitle(cpt->name());

    m_cptSeries->clear();
    m_frSeries->clear();
    m_layers.clear(); // Clear old rectangles if reloading data

    QList<double> qcValues = cpt->qc();
    QList<double> zValues = cpt->z();
    QList<double> frValues = cpt->fr();

    double minZ = 9999.0;
    double maxZ = -9999.0;

    // Populate data stream safely
    int dataSize = qMin(qcValues.size(), zValues.size());
    for (int i = 0; i < dataSize; ++i) {
        m_cptSeries->append(qcValues[i], zValues[i]);
        m_frSeries->append(frValues[i], zValues[i]);

        if (zValues[i] < minZ) minZ = zValues[i];
        if (zValues[i] > maxZ) maxZ = zValues[i];
    }

    m_xAxis->setRange(0, 20.0);
    double roundedMaxZ = std::ceil(maxZ);
    double roundedMinZ = std::floor(minZ);
    m_yAxis->setRange(roundedMinZ, roundedMaxZ);
    m_yAxis->setTickType(QValueAxis::TicksDynamic);
    m_yAxis->setTickAnchor(0.0);
    m_yAxis->setTickInterval(5.0);
}

// void CptChartWidget::addSoilLayer(double minQc, double maxQc, double topZ, double bottomZ, const QColor &color)
// {
//     // 1. Create native graphics scene item tracking
//     QGraphicsRectItem* rectItem = new QGraphicsRectItem();

//     // 2. Set fill color with alpha values passed directly via the QColor configuration
//     rectItem->setBrush(QBrush(color));
//     rectItem->setPen(Qt::NoPen); // No border line around the layer item

//     // Ensure the fill overlay places beneath the main blue line trace data
//     rectItem->setZValue(m_cptSeries->zValue() - 1);

//     // 3. Add to graphics scene
//     this->scene()->addItem(rectItem);

//     // 4. Save metadata so it can handle resize scaling conversions smoothly
//     m_layers.append({minQc, maxQc, topZ, bottomZ, color, rectItem});

//     updateRectangles();
// }

// void CptChartWidget::updateRectangles()
// {
//     for (const LayerData &layer : m_layers) {
//         // Convert the structural abstract numerical data bounds into visual pixel targets
//         QPointF topLeft = m_chart->mapToPosition(QPointF(layer.minQc, layer.topZ));
//         QPointF bottomRight = m_chart->mapToPosition(QPointF(layer.maxQc, layer.bottomZ));

//         // Define bounding rect area
//         QRectF rectBounds(topLeft, bottomRight);
//         layer.visualRect->setRect(rectBounds);
//     }
// }

void CptChartWidget::resizeEvent(QResizeEvent *event)
{
    QChartView::resizeEvent(event);
    // Recalculate pixel mapping coordinates anytime user pulls UI scale handles
    //updateRectangles();
}