#include <QEvent>
#include "aicontroller.h"
#include "placeevent.h"
#include "gobang.h"
#include <QDebug>

AIController::AIController() : _model(Gobang::get()) {}

bool AIController::event(QEvent *event)
{
  if(event->type() == slf::PLACE_EVENT_TYPE)
  {
    _model->place_chess();
    return true;
  }

  return QObject::event(event);
}
