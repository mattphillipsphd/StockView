#ifndef QUERYBUILDER_HPP
#define QUERYBUILDER_HPP

#include <QString>
#include <QJsonDocument>

class QueryBuilder
{
public:
    static QueryBuilder create()
    {
        return QueryBuilder();
    }

    QueryBuilder& setApiKey(const QString& apiKey)
    {
        this->apiKey = apiKey;
        return *this;
    }

    QueryBuilder& setAnalyticsUrl(const QString& analyticsUrl)
    {
        this->analyticsUrl = analyticsUrl;
        return *this;
    }

    QueryBuilder& setJsonFormat(const QJsonDocument::JsonFormat& jsonFormat)
    {
        this->jsonFormat = jsonFormat;
        return *this;
    }

    QueryBuilder& setFunction(const QString& function)
    {
        this->function = function;
        return *this;
    }

    QueryBuilder& setTickerSymbol(const QString& tickerSymbol)
    {
        this->tickerSymbol = tickerSymbol;
        return *this;
    }

    QString build() const
    {
        QString query = QString("%1/query?function=%2&symbol=%3&apikey=%4").arg(
            analyticsUrl, function, tickerSymbol, apiKey);
        return query;
    }

private:
    QueryBuilder() = default;
    QueryBuilder(const QueryBuilder&) = default;
    QueryBuilder(QueryBuilder&&) = default;
    QueryBuilder& operator=(const QueryBuilder&) = default;
    QueryBuilder& operator=(QueryBuilder&&) = default;

    QString apiKey;
    QString analyticsUrl;
    QString tickerSymbol;
    QString function;
    QJsonDocument::JsonFormat jsonFormat;
};

#endif // QUERYBUILDER_HPP
