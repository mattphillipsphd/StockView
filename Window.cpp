#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QRegularExpression>
#include "PythonLauncher.hpp"
#include "Window.hpp"

#include "./ui_window.h"
#include "QtUtils.hpp"

using namespace sv;

Window::Window(QWidget* parent) : QMainWindow(parent), ui(new Ui::Window), networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->TickerSymbols_LineEdit->setText("VUG");
    ui->FileSelect_LineEdit->setText("C:/Users/mattp/Documents/Qt/Projects/StockView/python/SimpleAnalysis.py");
    ui->InputArguments_LineEdit->setText("");
//    deleteTempFiles();
}

Window::~Window()
{
    delete ui;
}

void Window::on_FileSelector_Button_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open File", "C:/Users/mattp/Documents/Qt/Projects/StockView/python", "Python File (*.py)");

    if (!filePath.isEmpty())
        ui->FileSelect_LineEdit->setText(filePath);
}

void Window::on_Run_Button_clicked()
{
    QString scriptPath = ui->FileSelect_LineEdit->text();

    QStringList arguments;

    QStringList tickerSymbols = ui->TickerSymbols_LineEdit->text().split(';', Qt::SkipEmptyParts);
    QStringList extraArguments = ui->InputArguments_LineEdit->text().split(' ', Qt::SkipEmptyParts);
    arguments << tempFilePath;
    arguments += tickerSymbols;
    arguments += extraArguments;

    QString wslCommand = "python " + sv::convertToWslPath(scriptPath) + " " + sv::convertToWslPath(tempFilePath);
    wslCommand += " " + tickerSymbols.join(' ');
    wslCommand += " " + extraArguments.join(' ');
    ui->ConsoleOutput_TextBrowser->append("WSL command: \n" + wslCommand + "\n\n");
    ui->ConsoleOutput_TextBrowser->update();

    QSharedPointer<PythonLauncher> launcher = PythonLauncher::create(scriptPath, arguments);

    QString environmentPath = "C:/Users/mattp/Documents/StockView/StockAnalyzerEnvironment/";

    launcher->addVirtualEnvironment(environmentPath, {"numpy", "pandas", "scipy"});

    int exitCode = launcher->run();

    QString outputString = launcher->getOutput();

    ui->ConsoleOutput_TextBrowser->append(outputString);

    const QStringList toks = outputString.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    const bool hasEstimate = toks.contains("Estimate:") && toks.at( toks.indexOf("Estimate:")+1 ) == "Yes";
    if (hasEstimate)
    {
        const QString outPath = *std::find_if(toks.begin(), toks.end(),
                                              [](const QString& str){ return str.startsWith("C:"); });

        sv::StockDataResult estimate = readStockData(outPath, tickerSymbols.at(0));
        ui->StockView_Chart->setAllData(estimate);
    }

    if (exitCode != 0)
    {
        QMessageBox::warning(this, "Script Error",
                             QString("Python script exited with code %1.").arg(exitCode));
    }
}

void Window::on_GraphStocks_Button_clicked()
{
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &Window::OnDataReceived);

    const auto stock = ui->TickerSymbols_LineEdit->text();
    fetchStockData(stock);
}

void Window::deleteTempFiles() const
{
    QTemporaryFile tempFile;
    tempFile.setAutoRemove(false); // Don't delete automatically
    QTextStream stream(&tempFile);
    stream << "test\n";
    QString tempName = tempFile.fileName();
    tempFile.close();
    QString parentDir = QFileInfo{tempName}.dir().absolutePath();

    QDir directory(parentDir);
    if (!directory.exists()) {
        return;
    }

    QDirIterator it(parentDir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    size_t ct = 0;
    while (it.hasNext()) {
        QString filePath = it.next();
        QFileInfo fileInfo(filePath);
        if (fileInfo.fileName().contains("StockView", Qt::CaseInsensitive)) {
            QFile file(filePath);
            file.remove();
            ++ct;
        }
    }

    qDebug() << "Temporary files: " << ct << " removed from " << parentDir;
}
