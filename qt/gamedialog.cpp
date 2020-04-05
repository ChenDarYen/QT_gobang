#include "gamedialog.h"
#include "ui_gamedialog.h"

GameDialog::GameDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::GameDialog)
{
  ui->setupUi(this);
}

GameDialog::~GameDialog()
{
  delete ui;
}
