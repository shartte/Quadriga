
#include <QDebug>
#include <QImage>
#include <QBuffer>

#include "imagemodule.h"
#include "qbuffermodule.h"

#include "troikaformats/targaimage.h"

static const int tileWidth = 256;
static const int tileHeight = tileWidth;

QJSValue ImageModule::convertTargasToPng(int width, int height, const QJSValue &imageBuffers)
{
    if (!imageBuffers.isArray()) {
        qDebug() << "Third arg must be an array.";
        return QJSValue(); // TODO: Exception
    }

    QImage image(width, height, QImage::Format_ARGB32);

    int xTiles = (width + 255) / 256; // Round up
    int yTiles = (height + 255) / 256; // Also round up

    int destY = height; // We're counting down here.

    int index = 0; // Index in the imageBuffers array.

    for (int y = 0; y < yTiles; ++y) {

        int destX = 0;
        int lastTileHeight;
        for (int x = 0; x < xTiles; ++x) {

            QJSValue element = imageBuffers.property(index++);

            if (!element.isObject()) {
                qDebug() << "Missing image tile for " << x << y;
                continue;
            }

            QByteArray imgData = QBufferModule::getData(element);

            TargaImage tgaImage(imgData);

            if (!tgaImage.load()) {
                qWarning("Unable to load tile %d,%d (TGA) of combined image.", x, y);
                continue;
            }

            uint pixelSize = tgaImage.bitsPerPixel() / 8;

            for (uint sy = 0; sy < tgaImage.height(); ++sy) {
                uchar *destScanline = image.scanLine(destY - tgaImage.height() + sy);
                destScanline += destX * sizeof(QRgb);
                uchar *srcScanline = (uchar*)tgaImage.data() + (tgaImage.height() - 1 - sy) * tgaImage.width() * pixelSize;

                if (pixelSize == sizeof(QRgb)) {
                    qMemCopy(destScanline, srcScanline, pixelSize * tgaImage.width());
                } else {
                    Q_ASSERT(pixelSize == 3);

                    for (uint i = 0; i < pixelSize * tgaImage.width(); i += pixelSize) {
                        *(destScanline++) = *(srcScanline++);
                        *(destScanline++) = *(srcScanline++);
                        *(destScanline++) = *(srcScanline++);
                        *(destScanline++) = 0xFF;
                    }
                }
            }

            destX += tileWidth;
            lastTileHeight = tgaImage.height();
        }

        destY -= tileHeight; // We're counting down
    }

    QByteArray pngData;
    QBuffer pngBuffer(&pngData);

    if (!image.save(&pngBuffer, "png")) {
        qWarning("Unable to save image (PNG).");
        return QJSValue(); // TODO: Exception
    }

    pngBuffer.close();

    return QBufferModule::newBuffer(mCommonJsModule, pngData);
}

QJSValue ImageModule::convertTargaToPng(const QJSValue &targaBuffer)
{
    QByteArray tgaData = QBufferModule::getData(targaBuffer);

    TargaImage tgaImage(tgaData);

    if (!tgaImage.load()) {
        qDebug() << "Unable to load targa image" << tgaImage.error();
        return QJSValue();
    }

    // Convert to QImage
    QByteArray pngData;
    QBuffer pngBuffer(&pngData);

    QImage image(tgaImage.width(), tgaImage.height(), QImage::Format_ARGB32);

    auto pixelSize = (tgaImage.bitsPerPixel() == 32) ? 4 : 3;

    for (uint sy = 0; sy < tgaImage.height(); ++sy) {
        uchar *destScanline = image.scanLine(sy);
        uchar *srcScanline = (uchar*)tgaImage.data() + (tgaImage.height() - 1 - sy) * tgaImage.width() * pixelSize;

        if (pixelSize == sizeof(QRgb)) {
            qMemCopy(destScanline, srcScanline, pixelSize * tgaImage.width());
        } else {
            Q_ASSERT(pixelSize == 3);

            for (uint i = 0; i < pixelSize * tgaImage.width(); i += pixelSize) {
                *(destScanline++) = 0xFF;
                *(destScanline++) = *(srcScanline++);
                *(destScanline++) = *(srcScanline++);
                *(destScanline++) = *(srcScanline++);
            }
        }
    }

    if (!image.save(&pngBuffer, "png"))
        return false;

    pngBuffer.close();

    return QBufferModule::newBuffer(mCommonJsModule, pngData);
}

ImageModule::ImageModule(QCommonJSModule *commonJsModule) : mCommonJsModule(commonJsModule)
{
}
