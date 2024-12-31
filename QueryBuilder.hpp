#ifndef QUERYBUILDER_HPP
#define QUERYBUILDER_HPP

#include <QString>
#include <QJsonDocument>

class QueryBuilder
{
public:
    static QueryBuilder& instance()
    {
        if (!queryBuilder)
            queryBuilder = new QueryBuilder();
        return *queryBuilder;
    }

    QueryBuilder& setApiKey(const QString& apiKey) const
    {
        queryBuilder->apiKey = apiKey;
        return *queryBuilder;
    }

    QueryBuilder& setAnalyticsUrl(const QString& analyticsUrl) const
    {
        queryBuilder->analyticsUrl = analyticsUrl;
        return *queryBuilder;
    }

    QueryBuilder& setJsonFormat(const QJsonDocument::JsonFormat& jsonFormat) const
    {
        queryBuilder->jsonFormat = jsonFormat;
        return *queryBuilder;
    }

    QueryBuilder& setFunction(const QString& function) const
    {
        queryBuilder->function = function;
        return *queryBuilder;
    }

    QueryBuilder& setTickerSymbol(const QString& tickerSymbol) const
    {
        queryBuilder->tickerSymbol = tickerSymbol;
        return *queryBuilder;
    }

    QString build() const
    {
        QString query = QString("%1/query?function=%2&symbol=%3&apikey=%4").arg(
            analyticsUrl, function, tickerSymbol, apiKey);
        return query;
    }

private:
    static QueryBuilder* queryBuilder;

    QueryBuilder() = default;
    QString apiKey;
    QString analyticsUrl;
    QString tickerSymbol;
    QString function;
    QJsonDocument::JsonFormat jsonFormat;
};

inline QueryBuilder* QueryBuilder::queryBuilder = nullptr;

#endif // QUERYBUILDER_HPP
