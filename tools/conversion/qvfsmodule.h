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
    explicit QVfsModule(QCommonJSModule *commonJs);
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

    /**
     * @brief Reads a file as a buffer, but does not decompress it, if it is compressed already.
     * @param path The path of the file to ready.
     * @return An object that contains three properties. One is named "buffer" and contains the buffer with the file data,
     *         while the other is named "compressed" and is a flag whether the buffer contains zlib compressed data.
     *         The third is "uncompressedSize", which contains the size of the data when it is uncompressed.
     */
    QJSValue readFileRaw(const QString &path);

    /**
      Returns a list of all files in the virtual file system, filtered using the given filename filter.
      */
    QJSValue listAllFiles(const QString &filenameFilter) const;

    /**
      Returns a list of all files in the given directory and applies the given filename filter.
      */
    QJSValue listDirectory(const QString &directory) const;

private:
    QJSEngine *mEngine;
    QCommonJSModule *mCommonJsModule;

    QList<TroikaArchive*> mArchives;
    QList<QDir> mDirectories;
    
};

#endif // QVFSMODULE_H
