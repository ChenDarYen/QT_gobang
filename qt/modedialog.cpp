#include "modedialog.h"
#include "ui_modedialog.h"
#include <QtDebug>

ModeDialog::ModeDialog(int *mode, const QString &content, int step, QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ModeDialog),
  mode(mode),
  step(step)
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
  if(step == 1)
  {
    step += 1;
    ui->content->setText("You want to play as black or white?");
  }
  else
  {
    *mode = 1;
    close();
  }
}

void ModeDialog::on_btn_scd_clicked()
{
  if(step == 1)
    *mode = 0;
  else
    *mode = 2;

  close();
}
