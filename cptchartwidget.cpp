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
    m_xAxis->setLabelsColor(QColor(0, 0, 255));
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

    m_chart->setBackgroundVisible(false);
    m_chart->setPlotAreaBackgroundVisible(false);

}

void CptChartWidget::setCpt(Cpt* cpt)
{
    m_currentCpt = cpt;
    if (!cpt) {
        m_cptSeries->clear();
        m_frSeries->clear();
        viewport()->update(); // Trigger a redraw to clear previous profiles
        return;
    }

    m_chart->setTitle(cpt->name());

    QList<double> qcValues = cpt->qc();
    QList<double> zValues = cpt->z();
    QList<double> frValues = cpt->fr();

    double minZ = 9999.0;
    double maxZ = -9999.0;
    int dataSize = qMin(qcValues.size(), zValues.size());

    // 1. Allocate memory upfront to prevent reallocations
    QList<QPointF> cptPoints;
    QList<QPointF> frPoints;
    cptPoints.reserve(dataSize);
    frPoints.reserve(dataSize);

    // 2. Compute points and min/max in a single rapid loop
    for (int i = 0; i < dataSize; ++i) {
        double z = zValues[i];
        cptPoints.append(QPointF(qcValues[i], z));
        frPoints.append(QPointF(frValues[i], z));

        if (z < minZ) minZ = z;
        if (z > maxZ) maxZ = z;
    }

    // 3. Update the series data atomically in a single operation
    m_cptSeries->replace(cptPoints);
    m_frSeries->replace(frPoints);

    // 4. Update ranges afterward
    m_xAxis->setRange(0, 20.0);
    double roundedMaxZ = std::ceil(maxZ);
    double roundedMinZ = std::floor(minZ);
    m_yAxis->setRange(roundedMinZ, roundedMaxZ);
    m_yAxis->setTickType(QValueAxis::TicksDynamic);
    m_yAxis->setTickAnchor(0.0);
    m_yAxis->setTickInterval(5.0);

    viewport()->update();
}

void CptChartWidget::drawBackground(QPainter *painter, const QRectF &rect)
{
    QChartView::drawBackground(painter, rect);

    // 2. Early exit if no CPT data or no profile exists
    if (!m_currentCpt || !m_currentCpt->soilProfile()) {
        return;
    }

    SoilProfile* profile = m_currentCpt->soilProfile();
    QList<QObject*> layers = profile->soilLayers();
    if (layers.isEmpty()) return;

    // 3. Get the bounding box of the inner plot area (where lines are drawn)
    QRectF plotArea = m_chart->plotArea();
    if (!plotArea.isValid()) return;

    painter->save();

    // Clip drawing strictly to the plot area so layers don't bleed out into axis labels
    painter->setClipRect(plotArea);
    painter->setPen(Qt::NoPen);

    for (QObject* obj : layers) {
        // Safe dynamic cast assuming your QList contains SoilLayer instances
        SoilLayer* layer = qobject_cast<SoilLayer*>(obj);
        if (!layer) continue;

        // Fetch layer depth parameters (adjust these property names if your SoilLayer uses different ones, e.g., depthStart/depthEnd)
        double topZ = layer->top();
        double bottomZ = layer->bottom();
        QString code = layer->soilCode(); // Assuming a getter string like "organic_clay"

        // 4. Map the Z values to screen Y coordinates using the chart's axes
        // Since X coordinates don't matter for depth mapping, we pass 0.0 dummy value
        double topYPixel = m_chart->mapToPosition(QPointF(0.0, topZ)).y();
        double bottomYPixel = m_chart->mapToPosition(QPointF(0.0, bottomZ)).y();

        // Account for reversed axes safely: ensure top/bottom pixel tracking matches viewport coordinates
        double yStart = qMin(topYPixel, bottomYPixel);
        double yEnd = qMax(topYPixel, bottomYPixel);
        double rectHeight = yEnd - yStart;

        if (rectHeight <= 0) continue;

        // 5. Fetch color from palette and inject alpha (transparency)
        QColor layerColor = m_palette.getColor(code);
        layerColor.setAlpha(100); // 0 (transparent) to 255 (opaque). 100 is roughly 40% opacity.

        // 6. Draw the rectangle spanning the full width of the plotArea
        QRectF layerRect(plotArea.left(), yStart, plotArea.width(), rectHeight);
        painter->setBrush(layerColor);
        painter->drawRect(layerRect);
    }

    painter->restore();
}



void CptChartWidget::resizeEvent(QResizeEvent *event)
{
    QChartView::resizeEvent(event);
    viewport()->update();
}