#include "gamesystem.h"

GameSystem::GameSystem(QObject *parent) :
    QObject(parent)
{
}

void GameSystem::callMethod(const QString &text)
{
    qDebug("Hello World: %s", qPrintable(text));
}
