#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fluidSimulation.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	sim = new fluidSimulation(ui->viewer);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_goButton_clicked()
{
    sim->runSimulation();
}

void MainWindow::on_quitButton_clicked()
{
    close();
}
