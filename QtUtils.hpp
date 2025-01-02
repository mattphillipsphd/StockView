#ifndef QTUTILS_HPP
#define QTUTILS_HPP

#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QRegularExpression>

namespace sv
{

// Create a data structure to hold both points and metadata
struct StockDataResult
{
    QVector<QPointF> points;
    QMap<QString, QString> labels;
};

inline QString convertToWslPath(const QString& windowsPath)
{
    // First, normalize the path to use forward slashes
    QString normalized = QDir::fromNativeSeparators(windowsPath);

    // Check if it starts with a drive letter (e.g., "C:")
    QRegularExpression drivePattern("^([A-Za-z]):");
    auto match = drivePattern.match(normalized);

    if (!match.hasMatch()) {
        return normalized;  // Not a Windows path with drive letter
    }

    // Get the drive letter and convert to lowercase
    QString driveLetter = match.captured(1).toLower();

    // Remove the drive prefix and construct WSL path
    QString pathWithoutDrive = normalized.mid(2);  // Skip "C:"
    return QString("/mnt/%1%2").arg(driveLetter, pathWithoutDrive);
}

inline QString findProjectRoot()
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

inline QMap<QString, QString> loadEnvFile()
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

}

#endif // QTUTILS_HPP
