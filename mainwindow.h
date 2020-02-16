#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class BoardView;
class Gobang;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
   Q_OBJECT // QT 預設的 marco, 用於編譯

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void startGame() const;
  void showModeDialog(int *mode, const QString &content);

private:
  Ui::MainWindow *ui; // ui 的資料

  // model
  Gobang *_model{nullptr};
};
#endif // MAINWINDOW_H
