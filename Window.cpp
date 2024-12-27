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
    QString outputString;

    PythonLauncher::Launch(ui->FileSelect_LineEdit->text(), ui->InputArguments_LineEdit->text().split(" "), outputString);

    ui->ConsoleOutput_TextBrowser->setText(outputString);
}

void Window::on_GraphStocks_Button_clicked()
{
    connect(networkManager, &QNetworkAccessManager::finished, this, &Window::OnDataReceived);
    const auto stock = ui->TickerSymbols_LineEdit->text();
    fetchStockData(stock);
}

