#ifndef BOARD_H
#define BOARD_H

#include <QWidget>
#include "board.h"

class Gobang;

class BoardView : public QWidget
{
public:
  explicit BoardView(QWidget *parent);
  void paintEvent(QPaintEvent*) override;
  void mousePressEvent(QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

private:
  Coord _calc_coord(QMouseEvent *e) const;
  int _lattice_width{30};
  Coord _mouse_coord{0, 0};

  // model
  Gobang *_model{nullptr};
};

#endif // BOARD_H
