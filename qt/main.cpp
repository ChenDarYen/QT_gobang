#include "mainwindow.cpp"
#include "aicontroller.cpp"
#include "gobang.cpp"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  MainWindow w;

  AIController ai_controller;
  Gobang::get()->setAIController(&ai_controller);

  w.show();
  w.startGame();

  return a.exec();
}
