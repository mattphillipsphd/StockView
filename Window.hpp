#ifndef WINDOW_HPP
#define WINDOW_HPP

#include "ui_window.h"
#include <QDir>
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
#include <QTemporaryFile>
#include <QProcessEnvironment>

#include "QtUtils.hpp"
#include "QueryBuilder.hpp"

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

    void deleteTempFiles() const;

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
        sv::StockDataResult result = parseStockData(response);

        if (result.points.isEmpty())
        {
            qDebug() << "No valid stock data!";
            return;
        }

        // Update the chart with both data points and labels
        ui->StockView_Chart->setData(result.points, result.labels);

        // Create temporary file
        QTemporaryFile tempFile;
        tempFile.setAutoRemove(false); // Don't delete automatically

        if (!tempFile.open())
        {
            qDebug() << "Failed to create temporary file";
            return;
        }

        // Write data to file in CSV format
        QTextStream stream(&tempFile);
        stream << "timestamp,price\n"; // Header

        for (const QPointF& point : result.points)
        {
            stream << QString::number(point.x()) << ","
                   << QString::number(point.y()) << "\n";
        }

        tempFilePath = tempFile.fileName();
        tempFile.close();

        // Update the chart
        ui->StockView_Chart->setData(result.points, result.labels);

        ui->DataFile_LineEdit->setText( "Current Data File: " + tempFilePath );
//        QFile::remove(tempFilePath);
    }

    void on_GraphStocks_Button_clicked();

private:

    Ui::Window* ui;
    QNetworkAccessManager* networkManager;
    QGraphicsScene* scene;
    QString tempFilePath;

    void fetchStockData(const QString &symbol)
    {
        const QMap<QString, QString> env = loadEnvFile();

        const QString apiKey = env.value("API_KEY", "default_value_if_not_set");
        const QString sourceUrl = env.value("URL", "default_value_if_not_set");
        const QString function = env.value("FUNCTION", "default_value_if_not_set");

        if (env.contains("API_KEY"))
            qDebug() << "API_KEY:" << apiKey;
        else
            ui->ConsoleOutput_TextBrowser->setText("API_KEY not found in environment");
        if (env.contains("URL"))
            qDebug() << "URL:" << sourceUrl;
        else
            ui->ConsoleOutput_TextBrowser->setText("URL not found in environment");
        if (env.contains("FUNCTION"))
            qDebug() << "FUNCTION:" << function;
        else
            ui->ConsoleOutput_TextBrowser->setText("FUNCTION not found in environment");

        QueryBuilder& queryBuilder = QueryBuilder::instance()
            .setAnalyticsUrl(sourceUrl)
            .setFunction(function)
            .setTickerSymbol(symbol)
            .setApiKey(apiKey);
        QString query = queryBuilder.build();

        QNetworkRequest request{ QUrl{query} };
        networkManager->get(request);
    }

    sv::StockDataResult parseStockData(const QByteArray& jsonData)
    {
        sv::StockDataResult result;
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

    sv::StockDataResult readStockData(const QString& filename, const QString& symbol)
    {
        sv::StockDataResult result;
        result.labels = {
            {"x_axis", "Date"},
            {"y_axis", "Price (USD)"},
            {"legend", "est. " + symbol + " Stock Price"},
            {"title", symbol + " Daily Stock Prices"}
        };

        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return result;

        QTextStream in(&file);

        // Skip header line
        QString header = in.readLine();

        // Read data lines
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(',');

            if (fields.size() >= 2) {
                bool okTime, okPrice;
                double timestamp = fields[0].toDouble(&okTime);
                double price = fields[1].toDouble(&okPrice);

                if (okTime && okPrice) {
                    result.points.append(QPointF(timestamp, price));
                }
            }
        }

        file.close();
        return result;
    }

    QString findProjectRoot()
    {
        QDir dir(QCoreApplication::applicationDirPath());

        // Keep going up until we find a marker file (like .env or .git)
        while (!dir.exists(".env") && !dir.exists(".git")) {
            if (!dir.cdUp()) {
                // Reached root without finding marker
                return QString();
            }
        }

        return dir.absolutePath();
    }

    QMap<QString, QString> loadEnvFile()
    {
        QMap<QString, QString> envVars;
        QString path = findProjectRoot();
        QFile file(path + QDir::separator() + "stockview.env");

        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            while (!in.atEnd()) {
                QString line = in.readLine().trimmed();

                // Skip empty lines and comments
                if (line.isEmpty() || line.startsWith('#'))
                    continue;

                // Split on first '=' only
                int index = line.indexOf('=');
                if (index > 0) {
                    QString key = line.left(index).trimmed();
                    QString value = line.mid(index + 1).trimmed();

                    // Remove quotes if present
                    if (value.startsWith('"') && value.endsWith('"'))
                        value = value.mid(1, value.length() - 2);

                    envVars[key] = value;
                }
            }
            file.close();
        }
        return envVars;
    }

    void graphEstimate(const QString& estimatePath)
    {

    }
};

#endif // WINDOW_HPP
