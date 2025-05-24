#ifndef DATAFETCHER_HPP
#define DATAFETCHER_HPP

#include <exception>

#include <QUrl>
#include <QNetworkAccessManager>

#include "QueryBuilder.hpp"
#include "QtUtils.hpp"

class DataFetcher : public QObject
{
    Q_OBJECT

public:
    DataFetcher()
    {
        const QMap<QString, QString> env = sv::loadEnvFile();

        apiKey = env.value("API_KEY", "default_value_if_not_set");
        sourceUrl = env.value("URL", "default_value_if_not_set");
        function = env.value("FUNCTION", "default_value_if_not_set");

        if (env.contains("API_KEY"))
            qDebug() << "API_KEY:" << apiKey;
        else
            throw std::runtime_error{"API_KEY not found in environment"};
        if (env.contains("URL"))
            qDebug() << "URL:" << sourceUrl;
        else
            throw std::runtime_error{"URL not found in environment"};
        if (env.contains("FUNCTION"))
            qDebug() << "FUNCTION:" << function;
        else
            throw std::runtime_error{"FUNCTION not found in environment"};

    }

    void MakeQuery(const QString& tickerSymbol) const
    {
        QueryBuilder queryBuilder = QueryBuilder::create()
            .setAnalyticsUrl(sourceUrl)
            .setFunction(function)
            .setTickerSymbol(tickerSymbol)
            .setApiKey(apiKey);
        QString query = queryBuilder.build();

        QNetworkRequest request{ QUrl{query} };
        networkManager->get(request);

    }

    void setNetworkManager(QNetworkAccessManager* networkManager)
    {
        this->networkManager = networkManager;
    }

    QNetworkAccessManager* getNetworkManager() const
    {
        return networkManager;
    }

private:
    QNetworkAccessManager* networkManager;
    QString apiKey;
    QString sourceUrl;
    QString function;
};

#endif // DATAFETCHER_HPP
