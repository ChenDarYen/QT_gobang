#ifndef AIPLAYER_H
#define AIPLAYER_H

#include <QObject>

class Gobang;

class AIController : public QObject
{
public:
  AIController();
  bool event(QEvent *event) override;

private:
  Gobang *_model{nullptr};
};

#endif // AIPLAYER_H
