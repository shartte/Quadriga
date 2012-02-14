#ifndef GAMESYSTEM_H
#define GAMESYSTEM_H

#include <QObject>
#include <QMetaType>

class GameSystem : public QObject
{
    Q_OBJECT
public:
    explicit GameSystem(QObject *parent = 0);
    
signals:
    
public slots:

    void callMethod(const QString &text);
private:
    Q_DISABLE_COPY(GameSystem)
};

Q_DECLARE_METATYPE(GameSystem*)

#endif // GAMESYSTEM_H
