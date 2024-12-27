#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "ui_window.h"
#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QVector>
#include <QPointF>
#include <QDebug>

QT_BEGIN_NAMESPACE

namespace Ui
{
    class Window;
}

QT_END_NAMESPACE


class Window : public QMainWindow
{
    Q_OBJECT

public:

    Window(QWidget* parent = nullptr);
    ~Window();

    // Create a data structure to hold both points and metadata
    struct StockDataResult {
        QVector<QPointF> points;
        QMap<QString, QString> labels;
    };

private slots:

    void on_FileSelector_Button_clicked();

    void on_Run_Button_clicked();

    void OnDataReceived(QNetworkReply* reply)
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Failed to fetch stock data: " << reply->errorString();
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        reply->deleteLater();

        // Parse the data and get labels
        StockDataResult result = parseStockData(response);

        if (result.points.isEmpty())
        {
            qDebug() << "No valid stock data!";
            return;
        }

        // Update the chart with both data points and labels
        ui->StockView_Chart->setData(result.points, result.labels);
    }

    void on_GraphStocks_Button_clicked();

private:

    Ui::Window* ui;
    QNetworkAccessManager* networkManager;
    QGraphicsScene* scene;

    void fetchStockData(const QString &symbol)
    {
        QString apiKey = "RZDJD774MDM8F478";
        QString url = QString("https://www.alphavantage.co/query?function=TIME_SERIES_DAILY&symbol=%1&apikey=%2").arg(symbol, apiKey);

        QNetworkRequest request{ QUrl{url} };
        networkManager->get(request);
    }

    StockDataResult parseStockData(const QByteArray& jsonData)
    {
        StockDataResult result;
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);

        if (!doc.isObject())
        {
            qDebug() << "Invalid JSON format";
            return result;
        }

        QJsonObject root = doc.object();

        // Get the metadata
        QJsonObject metadata = root["Meta Data"].toObject();
        QString symbol = metadata["2. Symbol"].toString();

        // The time series data is under "Time Series (Daily)"
        QJsonObject timeSeriesData = root["Time Series (Daily)"].toObject();

        // Convert dates to timestamps and extract prices
        for (auto it = timeSeriesData.begin(); it != timeSeriesData.end(); ++it)
        {
            QString dateStr = it.key();
            QJsonObject dayData = it.value().toObject();

            // Convert date to timestamp
            QDateTime dateTime = QDateTime::fromString(dateStr, "yyyy-MM-dd");
            double timestamp = dateTime.toSecsSinceEpoch();

            // Get the closing price
            double price = dayData["4. close"].toString().toDouble();

            result.points.append(QPointF(timestamp, price));
        }

        // Sort points by timestamp
        std::sort(result.points.begin(), result.points.end(),
                  [](const QPointF& a, const QPointF& b) { return a.x() < b.x(); });

        // Set up the labels map
        result.labels = {
            {"x_axis", "Date"},
            {"y_axis", "Price (USD)"},
            {"legend", symbol + " Stock Price"},
            {"title", symbol + " Daily Stock Prices"}
        };

        return result;
    }

};

#endif // WINDOW_HPP
