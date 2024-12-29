#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsScene>
#include "PythonLauncher.hpp"
#include "Window.hpp"
#include "./ui_window.h"

Window::Window(QWidget* parent) : QMainWindow(parent), ui(new Ui::Window), networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->TickerSymbols_LineEdit->setText("VUG");
    ui->FileSelect_LineEdit->setText("C:/Users/mattp/Documents/Qt/Projects/StockView/python/SimpleAnalysis.py");
    ui->InputArguments_LineEdit->setText("VUG");
    deleteTempFiles();
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

    arguments << tempFilePath;
    arguments << ui->InputArguments_LineEdit->text().split(' ');

    QSharedPointer<PythonLauncher> launcher = PythonLauncher::create(scriptPath, arguments);

    QString environmentPath = "C:/Users/mattp/Documents/StockView/StockAnalyzerEnvironment/";

    launcher->addVirtualEnvironment(environmentPath, {"numpy", "pandas"});

    int exitCode = launcher->run();

    QString outputString = launcher->getOutput();

    ui->ConsoleOutput_TextBrowser->setText(outputString);

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
