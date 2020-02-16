#include <QEvent>
#include "aicontroller.h"
#include "placeevent.h"
#include "gobang.h"

AIController::AIController() : _model(Gobang::get()) {}

bool AIController::event(QEvent *event)
{
  if(event->type() == slf::PLACE_EVENT_TYPE)
  {
    _model->place_chess(true);
    return true;
  }

  return QObject::event(event);
}
