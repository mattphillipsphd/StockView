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
    ui->FileSelect_LineEdit->setText("//wsl.localhost/Ubuntu/home/matt/SimpleAnalysis.py");
    ui->InputArguments_LineEdit->setText("VUG");
}

Window::~Window()
{
    delete ui;
}

void Window::on_FileSelector_Button_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open File", "//wsl.localhost/Ubuntu/home/matt", "Python File (*.py)");

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
