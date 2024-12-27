#include "Window.hpp"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();

    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "StockView_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            application.installTranslator(&translator);
            break;
        }
    }

    Window window;

    window.show();

    return application.exec();
}
