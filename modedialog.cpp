#include "modedialog.h"
#include "ui_modedialog.h"

ModeDialog::ModeDialog(int *mode, const QString &content, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ModeDialog),
  mode(mode)
{
  ui->setupUi(this);
  ui->content->setText(content);
}

ModeDialog::~ModeDialog()
{
  delete ui;
  close();
}

void ModeDialog::on_btn_fst_clicked()
{
  *mode = 0;
  close();
}

void ModeDialog::on_btn_scd_clicked()
{
  *mode = 1;
  close();
}
