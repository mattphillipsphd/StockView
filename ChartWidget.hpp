#ifndef CHARTWIDGET_HPP
#define CHARTWIDGET_HPP

#include <QWidget>
#include <QVector>
#include <QPointF>

class ChartWidget : public QWidget
{

    Q_OBJECT

public:

    ChartWidget();

    explicit ChartWidget(QWidget* parent = nullptr);

    void setData(const QVector<QPointF>& data, const QMap<QString, QString>& labelData);
    void setAxisTitles(const QString& xTitle, const QString& yTitle);
    void setLegendData(const QString& legendData);
    void setTitle(const QString& title);
protected:

    void paintEvent(QPaintEvent* event) override;

private:

    QVector<QPointF> data;
    QString xAxisTitle;
    QString yAxisTitle;
    QString chartTitle;
    QString legendData;
};

#endif // CHARTWIDGET_HPP
