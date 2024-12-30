#ifndef QTUTILS_HPP
#define QTUTILS_HPP

#include <QDir>
#include <QString>
#include <QRegularExpression>

namespace sv
{

QString convertToWslPath(const QString& windowsPath) {
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

}

#endif // QTUTILS_HPP
