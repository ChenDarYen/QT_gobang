#ifndef MODEDIALOG_H
#define MODEDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class ModeDialog;
}

class ModeDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ModeDialog(int *mode,
                      const QString &content,
                      int step = 1,
                      QWidget *parent = nullptr);
  ~ModeDialog();

private Q_SLOT:
  void on_btn_fst_clicked();
  void on_btn_scd_clicked();


private:
  Ui::ModeDialog *ui;
  int *mode;
  int step{1};
};

#endif // MODEDIALOG_H
