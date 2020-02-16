#include <cmath>
#include "boardview.h"
#include "gobang.h"

#include <QPen>
#include <QBrush>
#include <QPainter>

#include <QColor>
#include <QColorDialog>

#include <QPaintEvent>
#include <QMouseEvent>

using std::abs;
using std::round;

BoardView::BoardView(QWidget *parent) : QWidget(parent), _model(Gobang::get())
{
  _model->setBoardView(this);

  QPalette palette;
  palette.setBrush(QPalette::Background,  QBrush(QColor(216, 156, 72)));

  setPalette(palette);

  setAutoFillBackground(true);
  setMinimumSize(_lattice_width * 19, _lattice_width * 19);
  setMaximumSize(_lattice_width * 19, _lattice_width * 19);
}

void BoardView::paintEvent(QPaintEvent*)
{
  const int W = 2;

  // define Qpens, Qbrushes
  QPen black_pen(Qt::black, W, Qt::SolidLine);
  QPen white_pen(Qt::white, W, Qt::SolidLine);

  QBrush black_brush(Qt::black, Qt::SolidPattern);
  QBrush white_brush(Qt::white, Qt::SolidPattern);

  // define Qpainter
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(black_pen);

  // draw lines
  for(int i = 0; i < 19; ++i)
  {
    // vertical
    painter.drawLine((.5 + i) * _lattice_width, .5 * _lattice_width,
                     (.5 + i) * _lattice_width, 18.5 * _lattice_width);
    // horizontal
    painter.drawLine(.5 * _lattice_width, (.5 + i) * _lattice_width,
                     18.5 * _lattice_width, (.5 + i) * _lattice_width);
  }

  // draw star
  painter.setBrush(black_brush);

  int star_pos[3] = {3, 9, 15};
  for(int i = 0; i < 3; ++i)
    for(int j = 0; j < 3; ++j)
      painter.drawEllipse((.5 + star_pos[i]) * _lattice_width - 1.5 * W,
                          (.5 + star_pos[j]) * _lattice_width - 1.5 * W,
                          3 * W,
                          3 * W);

  // draw chess
  for(int i = 0; i < 361; ++i)
  {
    int status = (*_model->board())[i];
    if(status != -1)
    {
      if(status == 0) // black chess
      {
        painter.setPen(black_pen);
        painter.setBrush(black_brush);
      }
      else // white chess
      {
        painter.setPen(white_pen);
        painter.setBrush(white_brush);
      }

      int x = i % 19, y = i / 19;
      painter.drawEllipse((.5 + x) * _lattice_width - 6 * W,
                          (.5 + y) * _lattice_width - 6 * W,
                          12 * W,
                          12 * W);
    }
  }
}

void BoardView::mousePressEvent(QMouseEvent *e)
{
  _mouse_coord = _calc_coord(e);
}

void BoardView::mouseReleaseEvent(QMouseEvent *e)
{
  if(_mouse_coord.x != 0 && _calc_coord(e) == _mouse_coord)
    _model->place_chess(false, _mouse_coord); // false means it's not AI
}

Coord BoardView::_calc_coord(QMouseEvent *e) const
{
  double x = e->x(), y = e->y();
  x = x / _lattice_width - .5;
  y = y / _lattice_width - .5;

  return abs(x - round(x)) < .4 && abs(y - round(y)) < .4 ?
        Coord(round(x) + 1, round(y) + 1) : Coord(0, 0);
}
