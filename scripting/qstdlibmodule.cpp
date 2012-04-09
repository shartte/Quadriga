#include <QDebug>
#include <QStringList>
#include <QFile>

#include "qstdlibmodule.h"

void QStdLibModule::install(QCommonJSModule *modules)
{
    Q_INIT_RESOURCE(resources);

    QStringList files;
    files << "util" << "assert" << "buffer";

    foreach (QString file, files) {
        qDebug() << "Loading stdlib " << file;

        QString filename = ":/scripts/" + file + ".js";
        QFile f(filename);
        if (!f.open(QFile::ReadOnly)) {
            qDebug() << "Unable to open stdlib" << file;
            continue;
        }

        QString scriptData = QString::fromUtf8(f.readAll());
        modules->addModuleFromScript(file, filename, scriptData);
    }

}

