
#include <QDateTime>
#include <QByteArray>
#include <QMutex>

#include "zip.h"

#include "zipwriter.h"

inline uLong compressBound (uLong sourceLen)
{
    return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) + 11;
}

class ZipWriterData
{
public:
    zipFile zFile;
    QMutex mutex;
};

ZipWriter::ZipWriter(const QString &filename) : d_ptr(new ZipWriterData)
{
    d_ptr->zFile = zipOpen64(filename.toLocal8Bit(), APPEND_STATUS_CREATE);

    if (!d_ptr->zFile) {
        qWarning("Unable to create zip file %s.", qPrintable(filename));
    }
}

ZipWriter::~ZipWriter()
{
    close(); // Force close if object is destroyed
}

static void setToCurrentDate(tm_zip *zipTime) {
    QDateTime now = QDateTime::currentDateTime();

    QTime nowTime = now.time();
    zipTime->tm_hour = nowTime.hour();
    zipTime->tm_min = nowTime.minute();
    zipTime->tm_sec = nowTime.second();

    QDate nowDate = now.date();
    zipTime->tm_year = nowDate.year();
    zipTime->tm_mon = nowDate.month()-1;
    zipTime->tm_mday = nowDate.day();
}

static QByteArray compressdata(const QByteArray &data)
{
    QByteArray result(compressBound(data.size()), Qt::Uninitialized);

    z_stream stream;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
    stream.avail_in = (uInt)data.size();
    stream.avail_out = (uInt)result.size();
    stream.next_out = (Bytef*)result.data();
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.constData()));
    stream.total_in = 0;
    stream.total_out = 0;
    stream.data_type = Z_BINARY;
    
    int err = deflateInit2(&stream, 9, Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY);

    if (err != Z_OK) {
        qWarning("Unable init zlib deflate2.");
        return QByteArray();
    }

    err = deflate(&stream, Z_NO_FLUSH);

    if (err != Z_OK) {
        qWarning("Unable to deflate.");
        return QByteArray();
    }

    err = deflate(&stream,  Z_FINISH);

    if (err != Z_STREAM_END) {
        qWarning("Unable to deflate.");
        return QByteArray();
    }

    err = deflateEnd(&stream);

    if (err != Z_OK) {
        qWarning("Unable to finish deflate.");
        return QByteArray();
    }

    result.resize(stream.total_out);
    return result;
}

bool ZipWriter::addFile(const QString &filename, const QByteArray &data, int compressionLevel)
{
    QByteArray filenameInZip = filename.toLocal8Bit();

    // Filenames must not start with a slash, otherwise the ZIP file is not readable by WinXP
    while (filenameInZip.startsWith('/') || filenameInZip.startsWith('\\'))
        filenameInZip = filenameInZip.right(filenameInZip.size() - 1);

    zip_fileinfo fileInfo;
    memset(&fileInfo, 0, sizeof(fileInfo));
    setToCurrentDate(&fileInfo.tmz_date);

    int raw = 1;

    uLong crc = 0;
    crc = crc32(crc, reinterpret_cast<const Bytef*>(data.constData()), data.size());

    QByteArray rawData;
    if (compressionLevel != 0 && raw == 1) {
        compressionLevel = 9;

        rawData = compressdata(data);
    }

    QMutexLocker locker(&d_ptr->mutex);

    int err = zipOpenNewFileInZip2_64(d_ptr->zFile, filenameInZip, &fileInfo,
                                     NULL, 0, NULL, 0, NULL /* comment*/,
                                     (compressionLevel != 0) ? Z_DEFLATED : 0,
                                     compressionLevel, raw, 0);

    if (err != ZIP_OK) {
        qWarning("Unable to add %s to zip file. Error Code: %d", qPrintable(filename), err);
        return false;
    }

    if (compressionLevel != 0 && raw == 1)
        err = zipWriteInFileInZip(d_ptr->zFile, (const Bytef*)rawData.constData(), rawData.size());
    else
        err = zipWriteInFileInZip(d_ptr->zFile, data.constData(), data.size());

    if (err != ZIP_OK) {
        qWarning("Unable to write to %s in zip file. Error code: %d.", qPrintable(filename), err);
        return false;
    }

    if (raw == 1)
        err = zipCloseFileInZipRaw64(d_ptr->zFile, data.size(), crc);
    else
        err = zipCloseFileInZip(d_ptr->zFile);

    if (err != ZIP_OK) {
        qWarning("Unable to close %s in zip file. Error: %d", qPrintable(filename), err);
    }

    return true;
}

void ZipWriter::close()
{
    if (d_ptr->zFile) {
        int err = zipClose(d_ptr->zFile, 0);
        if (err != ZIP_OK) {
            qWarning("Unable to close zip file. Error code: %d.", err);
        }
        d_ptr->zFile = 0;
    }
}
