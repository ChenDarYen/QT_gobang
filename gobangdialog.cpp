#include "gobangdialog.h"
#include "ui_gobangdialog.h"

GobangDialog::GobangDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::GobangDialog)
{
  ui->setupUi(this);
}

GobangDialog::~GobangDialog()
{
  delete ui;
}
