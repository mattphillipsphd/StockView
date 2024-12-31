#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QFontMetrics>
#include <QDebug>
#include <QDateTime>
#include "ChartWidget.hpp"

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent);
}

void ChartWidget::setAllData(const sv::StockDataResult& result)
{
    const auto& labels = result.labels;
    if (this->rawData.empty())
        setData(result.points, labels);
    else
        appendData(result.points);
    setAxisTitles( labels["x_axis"], labels["y_axis"] );
    setTitle( labels["title"] );
    setLegendData( labels["legend"] );
}

void ChartWidget::appendData(const QVector<QPointF>& data)
{
    estimateData = data;
    update();
}

void ChartWidget::setData(const QVector<QPointF>& data, const QMap<QString, QString>& labelData)
{
    rawData = data;
    setAxisTitles(labelData.value("x_axis"), labelData.value("y_axis"));
    setLegendData(labelData.value("legend"));
    update();
}

void ChartWidget::setAxisTitles(const QString& xTitle, const QString& yTitle)
{
    this->xAxisTitle = xTitle;
    this->yAxisTitle = yTitle;
    update();
}

void ChartWidget::setTitle(const QString& title)
{
    this->chartTitle = title;
    update();
}

void ChartWidget::setLegendData(const QString& legendData)
{
    this->legendData = legendData;
    update();
}

void ChartWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (rawData.isEmpty())
        return;

    chartSpec = ChartSpec{(double)this->width(), (double)this->height()};
    chartSpec.setScale( estimateData.empty() ? rawData : estimateData );

    // Draw background
    painter.fillRect(rect(), Qt::white);

    // Draw chart title
    QFont titleFont = painter.font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, chartTitle);

    // Draw axes
    QPen axisPen(Qt::black, 2);
    painter.setPen(axisPen);
    painter.drawLine(chartSpec.leftMargin, chartSpec.topMargin + chartSpec.height, chartSpec.leftMargin + chartSpec.width, chartSpec.topMargin + chartSpec.height);  // X axis
    painter.drawLine(chartSpec.leftMargin, chartSpec.topMargin, chartSpec.leftMargin, chartSpec.topMargin + chartSpec.height);                   // Y axis

    // Draw grid
    QPen gridPen(Qt::lightGray, 1, Qt::DotLine);
    painter.setPen(gridPen);

    int numXTicks = 10;
    int numYTicks = 10;

    // Draw ticks and labels
    QPen tickPen(Qt::black, 1);
    painter.setPen(tickPen);

    QFont labelFont = painter.font();
    labelFont.setPointSize(9);
    painter.setFont(labelFont);
    QFontMetrics fm(labelFont);

    // X-axis ticks and labels
    for (int i = 0; i <= numXTicks; ++i)
    {
        double x = chartSpec.leftMargin + i * (chartSpec.width / numXTicks);
        // Grid line
        painter.setPen(gridPen);
        painter.drawLine(x, chartSpec.topMargin, x, chartSpec.topMargin + chartSpec.height);

        // Tick mark
        painter.setPen(tickPen);
        painter.drawLine(x, chartSpec.topMargin + chartSpec.height - 5, x, chartSpec.topMargin + chartSpec.height + 5);

        // Convert UNIX timestamp to date string
        double timestamp = chartSpec.minX + i * (chartSpec.maxX - chartSpec.minX) / numXTicks;
        QDateTime dateTime;
        dateTime.setSecsSinceEpoch(static_cast<qint64>(timestamp));
        QString label = dateTime.toString("yyyy-MM-dd");

        // Save current painter state
        painter.save();

        // Position labels closer to the axis while still avoiding overlap
        painter.translate(x, chartSpec.topMargin + chartSpec.height + 75);
        painter.rotate(-90);

        // Draw rotated text
        painter.drawText(0, 0, label);

        // Restore painter state
        painter.restore();
    }

    // Y-axis ticks and labels (unchanged)
    for (int i = 0; i <= numYTicks; ++i)
    {
        double y = chartSpec.topMargin + chartSpec.height - i * (chartSpec.height / numYTicks);
        // Grid line
        painter.setPen(gridPen);
        painter.drawLine(chartSpec.leftMargin, y, chartSpec.leftMargin + chartSpec.width, y);

        // Tick mark
        painter.setPen(tickPen);
        painter.drawLine(chartSpec.leftMargin - 5, y, chartSpec.leftMargin + 5, y);

        double value = chartSpec.minY + i * (chartSpec.maxY - chartSpec.minY) / numYTicks;
        QString label = QString::number(value, 'f', 1);
        int labelWidth = fm.horizontalAdvance(label);
        painter.drawText(chartSpec.leftMargin - labelWidth - 10, y + fm.height() / 4, label);
    }

    // Draw axis titles
    painter.save();
    painter.translate(15, chartSpec.height / 2 + chartSpec.topMargin);
    painter.rotate(-90);
    painter.drawText(0, 0, yAxisTitle);
    painter.restore();

    painter.drawText(chartSpec.leftMargin + chartSpec.width / 2 - fm.horizontalAdvance(xAxisTitle) / 2,
                     chartSpec.height + chartSpec.topMargin + 100, xAxisTitle);

    // Draw the data line
    if (rawData.size() >= 2)
    {
        QPen chartPen = drawCurve(painter, Qt::red, estimateData, chartSpec);
        drawCurve(painter, Qt::blue, rawData, chartSpec);

        // Calculate legend box size based on text width
        int legendTextWidth = fm.horizontalAdvance(legendData);
        int legendBoxWidth = legendTextWidth + 40;  // Add padding for the line and spacing
        int legendBoxHeight = 30;

        // Draw legend with adjusted size
        QRect legendRect(chartSpec.leftMargin + chartSpec.width - legendBoxWidth - 10, chartSpec.topMargin + 10,
                         legendBoxWidth, legendBoxHeight);
        painter.fillRect(legendRect, Qt::white);
        painter.setPen(Qt::black);
        painter.drawRect(legendRect);

        // Legend line
        painter.setPen(chartPen);
        painter.drawLine(legendRect.left() + 5, legendRect.center().y(),
                         legendRect.left() + 25, legendRect.center().y());

        // Legend text
        painter.setPen(Qt::black);
        painter.drawText(legendRect.left() + 30, legendRect.center().y() + fm.height() / 3, legendData);
    }
}

QPen ChartWidget::drawCurve(QPainter& painter, Qt::GlobalColor penColor, const QVector<QPointF>& data,
                            const ChartSpec& chartSpec)
{
    QPen chartPen(penColor, 2);
    painter.setPen(chartPen);

    QPointF previousPoint;
    bool firstPoint = true;

    for (const QPointF& point : data)
    {
        const double x = chartSpec.leftMargin + (point.x() - chartSpec.minX) * chartSpec.xScale;
        const double y = chartSpec.topMargin + chartSpec.height - (point.y() - chartSpec.minY) * chartSpec.yScale;

        if (firstPoint)
        {
            previousPoint = QPointF(x, y);
            firstPoint = false;
        }
        else
        {
            painter.drawLine(previousPoint, QPointF(x, y));
            previousPoint = QPointF(x, y);
        }
    }

    return chartPen;
}
