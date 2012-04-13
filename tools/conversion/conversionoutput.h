#ifndef CONVERSIONRESULT_H
#define CONVERSIONRESULT_H

#include <QObject>
#include <QJSValue>
#include <QString>

class QCommonJSModule;
class ZipWriter;

class ConversionOutput: public QObject
{
    Q_OBJECT
public:
    explicit ConversionOutput(QCommonJSModule *commonJsModule);
    
public slots:

    /**
     * @brief Defines an output category as a directory on the disk.
     * @param category The name of the output category to define.
     * @param baseDirectory The directory where added files will be created. This directory will be created if it does not exist (including all parent directories).
     */
    QJSValue openCategoryAsDirectory(const QString &category, const QString &baseDirectory);

    /**
     * @brief Adds a category which will be written to a ZIP archive.
     * @param category The name of the output category to define.
     * @param archiveFilename The full path name of the ZIP archive that will be created or added to. The same archive filename can be used by multiple categories.
     */
    QJSValue openCategoryAsArchive(const QString &category, const QString &archiveFilename);

    /**
     * @brief Closes and removes all opened categories, which is required to finalize any pending ZIP archives correctly.
     */
    QJSValue close();

    /**
     * @brief Adds a JS buffer to the conversion output under the given name.
     * @param category The conversion output category. This determines where the file will be written (or which archive it will be written to).
     * @param filename The filename for the buffer.
     * @param buffer The buffer to write.
     * @return Undefined normally, otherwise an exception will be thrown.
     */
    QJSValue addBuffer(const QString &category, const QString &filename, const QJSValue &buffer);

    /**
     * @brief UTF-8 encodes a string and adds it to the conversion output.
     * @param category The conversion output category. This determines where the file will be written (or which archive it will be written to).
     * @param filename The filename of the file that will contain the written string.
     * @param content The string to write to the file.
     * @return Undefined normally, otherwise an exception will be thrown.
     */
    QJSValue addString(const QString &category, const QString &filename, const QString &content);

private:
    QCommonJSModule *mCommonJsModule;

    QHash<QString, QString> mCategoryArchiveMap;
    QHash<QString, QDir> mCategoryDirectoryMap;
    QHash<QString, ZipWriter*> mArchiveMap;
    
};

#endif // CONVERSIONRESULT_H
