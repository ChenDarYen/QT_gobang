#include "placeevent.h"
#include <QDebug>

PlaceEvent::PlaceEvent() : QEvent(static_cast<QEvent::Type>(slf::PLACE_EVENT_TYPE)) {}
