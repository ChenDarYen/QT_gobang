#ifndef GOBANGDIALOG_H
#define GOBANGDIALOG_H

#include <QDialog>

namespace Ui {
class GobangDialog;
}

class GobangDialog : public QDialog
{
  Q_OBJECT

public:
  explicit GobangDialog(QWidget *parent = nullptr);
  ~GobangDialog();

private:
  Ui::GobangDialog *ui{nullptr};
};

#endif // GOBANGDIALOG_H
