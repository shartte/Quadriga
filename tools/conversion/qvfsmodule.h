#ifndef QVFSMODULE_H
#define QVFSMODULE_H

#include <QObject>
#include <QJSValue>
#include <QDir>

class TroikaArchive;
class QCommonJSModule;
class QJSEngine;

class QVfsModule : public QObject
{
    Q_OBJECT
public:
    explicit QVfsModule(QJSEngine *engine);
    ~QVfsModule();
        
signals:
    
public slots:

    /**
      Removes all previously added archives and override directories.
      */
    void clear();

    /**
      Attempts to guess the path the game is installed in. This will only
      work on Windows systems.
      Returns undefined if the path could not be guessed.
      */
    QJSValue guessGamePath() const;

    /**
      Tries to validate the path as a valid installation directory for ToEE.
      */
    QJSValue validateGamePath(const QString &gamePath);

    /**
      Adds the default ToEE archives from the given game path.

      The default archives are ToEE[1-4].dat and modules/ToEE.dat
      */
    void addDefaultArchives(const QString &gamePath);

    /**
      Adds a single archive to this VFS.
      */
    void addArchive(const QString &archivePath);

    /**
      Adds an override directory.
      */
    void addOverrideDirectory(const QString &directory);

    /**
      Read a file as a String.
      */
    QJSValue readFileAsString(const QString &path);

    /**
      Read a file as a Buffer.
      */
    QJSValue readFile(const QString &path);

private:
    QJSEngine *mEngine;

    QList<TroikaArchive*> mArchives;
    QList<QDir> mDirectories;
    
};

#endif // QVFSMODULE_H
