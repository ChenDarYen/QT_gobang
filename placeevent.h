#ifndef PLACEEVENT_H
#define PLACEEVENT_H

#include <QEvent>

namespace slf
{
  const int PLACE_EVENT_TYPE = 1000;
}

class PlaceEvent : public QEvent
{
public:
  PlaceEvent();
};

#endif // PLACEEVENT_H
