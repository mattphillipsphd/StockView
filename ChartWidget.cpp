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

void ChartWidget::setData(const QVector<QPointF>& data, const QMap<QString, QString>& labelData)
{
    this->data = data;
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

    // Adjusted margins for tighter layout
    double leftMargin = 70;     // Space for y-axis labels
    double rightMargin = 50;
    double topMargin = 50;      // Space for title
    double bottomMargin = 130;  // Reduced to eliminate excess whitespace

    double width = this->width() - (leftMargin + rightMargin);
    double height = this->height() - (topMargin + bottomMargin);

    if (width <= 0 || height <= 0 || data.isEmpty())
        return;

    // Draw background
    painter.fillRect(rect(), Qt::white);

    // Calculate data ranges
    double minX = data.first().x();
    double maxX = data.last().x();
    double minY = data.first().y();
    double maxY = data.first().y();

    for (const QPointF& point : data)
    {
        if (point.y() < minY) minY = point.y();
        if (point.y() > maxY) maxY = point.y();
    }

    if (minX == maxX) { minX -= 1; maxX += 1; }
    if (minY == maxY) { minY -= 1; maxY += 1; }

    double xScale = width / (maxX - minX);
    double yScale = height / (maxY - minY);

    // Draw chart title
    QFont titleFont = painter.font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, chartTitle);

    // Draw axes
    QPen axisPen(Qt::black, 2);
    painter.setPen(axisPen);
    painter.drawLine(leftMargin, topMargin + height, leftMargin + width, topMargin + height);  // X axis
    painter.drawLine(leftMargin, topMargin, leftMargin, topMargin + height);                   // Y axis

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
        double x = leftMargin + i * (width / numXTicks);
        // Grid line
        painter.setPen(gridPen);
        painter.drawLine(x, topMargin, x, topMargin + height);

        // Tick mark
        painter.setPen(tickPen);
        painter.drawLine(x, topMargin + height - 5, x, topMargin + height + 5);

        // Convert UNIX timestamp to date string
        double timestamp = minX + i * (maxX - minX) / numXTicks;
        QDateTime dateTime;
        dateTime.setSecsSinceEpoch(static_cast<qint64>(timestamp));
        QString label = dateTime.toString("yyyy-MM-dd");

        // Save current painter state
        painter.save();

        // Position labels closer to the axis while still avoiding overlap
        painter.translate(x, topMargin + height + 75);
        painter.rotate(-90);

        // Draw rotated text
        painter.drawText(0, 0, label);

        // Restore painter state
        painter.restore();
    }

    // Y-axis ticks and labels (unchanged)
    for (int i = 0; i <= numYTicks; ++i)
    {
        double y = topMargin + height - i * (height / numYTicks);
        // Grid line
        painter.setPen(gridPen);
        painter.drawLine(leftMargin, y, leftMargin + width, y);

        // Tick mark
        painter.setPen(tickPen);
        painter.drawLine(leftMargin - 5, y, leftMargin + 5, y);

        double value = minY + i * (maxY - minY) / numYTicks;
        QString label = QString::number(value, 'f', 1);
        int labelWidth = fm.horizontalAdvance(label);
        painter.drawText(leftMargin - labelWidth - 10, y + fm.height() / 4, label);
    }

    // Draw axis titles
    painter.save();
    painter.translate(15, height / 2 + topMargin);
    painter.rotate(-90);
    painter.drawText(0, 0, yAxisTitle);
    painter.restore();

    painter.drawText(leftMargin + width / 2 - fm.horizontalAdvance(xAxisTitle) / 2,
                     height + topMargin + 100, xAxisTitle);

    // Draw the data line
    if (data.size() >= 2)
    {
        QPen chartPen(Qt::blue, 2);
        painter.setPen(chartPen);

        QPointF previousPoint;
        bool firstPoint = true;

        for (const QPointF& point : data)
        {
            double x = leftMargin + (point.x() - minX) * xScale;
            double y = topMargin + height - (point.y() - minY) * yScale;

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

        // Calculate legend box size based on text width
        int legendTextWidth = fm.horizontalAdvance(legendData);
        int legendBoxWidth = legendTextWidth + 40;  // Add padding for the line and spacing
        int legendBoxHeight = 30;

        // Draw legend with adjusted size
        QRect legendRect(leftMargin + width - legendBoxWidth - 10, topMargin + 10,
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
