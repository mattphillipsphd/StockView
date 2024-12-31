#ifndef CHARTWIDGET_HPP
#define CHARTWIDGET_HPP

#include <QWidget>
#include <QVector>
#include <QPointF>

#include "QtUtils.hpp"

struct ChartSpec
{
    ChartSpec() = default;
    ChartSpec(double _width, double _height)
        : leftMargin{70}, rightMargin{50}, topMargin{50}, bottomMargin{130}
    {
        width = _width - (leftMargin + rightMargin);
        height = _height - (topMargin + bottomMargin);
    }

    void setScale(const QVector<QPointF>& data)
    {
        // Calculate data ranges
        minX = data.first().x();
        maxX = data.last().x();
        minY = data.first().y();
        maxY = data.first().y();

        for (const QPointF& point : data)
        {
            if (point.y() < minY) minY = point.y();
            if (point.y() > maxY) maxY = point.y();
        }

        if (minX == maxX) { minX -= 1; maxX += 1; }
        if (minY == maxY) { minY -= 1; maxY += 1; }

        xScale = width / (maxX - minX);
        yScale = height / (maxY - minY);
    }

    double leftMargin;
    double rightMargin;
    double topMargin;
    double bottomMargin;

    double width;
    double height;
    double maxX, maxY;
    double minX, minY;
    double xScale, yScale;
};

class ChartWidget : public QWidget
{

    Q_OBJECT

public:

    ChartWidget();

    explicit ChartWidget(QWidget* parent = nullptr);

    void setAllData(const sv::StockDataResult& result);
    void appendData(const QVector<QPointF>& data);
    void setData(const QVector<QPointF>& data, const QMap<QString, QString>& labelData);
    void setAxisTitles(const QString& xTitle, const QString& yTitle);
    void setLegendData(const QString& legendData);
    void setTitle(const QString& title);
protected:

    void paintEvent(QPaintEvent* event) override;

private:
    QPen drawCurve(QPainter& painter, Qt::GlobalColor penColor, const QVector<QPointF>& data, const ChartSpec& chartSpec);

    ChartSpec chartSpec;
    QVector<QPointF> rawData;
    QVector<QPointF> estimateData;
    QString xAxisTitle;
    QString yAxisTitle;
    QString chartTitle;
    QString legendData;
};

#endif // CHARTWIDGET_HPP
