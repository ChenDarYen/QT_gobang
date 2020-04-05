#include <QLayout>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "modedialog.h"
#include "boardview.h"
#include "gobang.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow),
                                          _model(Gobang::get())
{
  _model->setWindow(this);

  ui->setupUi(this);

  auto c_widget = new QWidget(this);
  auto board = new BoardView(c_widget);
  auto layout = new QGridLayout;

  layout->addWidget(new QWidget(c_widget), 0, 0);
  layout->addWidget(board, 1, 1);
  layout->addWidget(new QWidget(c_widget), 2, 2);

  c_widget->setLayout(layout);

  setCentralWidget(c_widget);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::startGame() const
{
  _model->play();
}

void MainWindow::showModeDialog(int *mode, const QString &content)
{
  ModeDialog dialog(mode, content, 1, this);
  dialog.exec();
}
