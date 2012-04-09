#ifndef IMAGECOMBINER_H
#define IMAGECOMBINER_H

#include <QObject>
#include <QJSValue>
#include <qcommonjsmodule.h>

/**
 * @brief The ImageCombiner class
 */
class ImageModule : public QObject
{
    Q_OBJECT
public:
    ImageModule(QCommonJSModule *commonJsModule);
public slots:
    
    /**
     * @brief Combines several TGA image tiles into a single PNG image. (left to right, top to bottom)
     * @param totalWidth Total width of the image.
     * @param totalHeight Total height of the image.
     * @param imageBuffers An JS array of buffers that contain the TGA image tiles.
     * @return Image encoded as PNG.
     */
    QJSValue convertTargasToPng(int width, int height, const QJSValue &imageBuffers);

    /**
     * @brief Converts Targa images to PNG.
     * @param targaData The TGA image data.
     * @return The PNG image data.
     */
    QJSValue convertTargaToPng(const QJSValue &targaData);

private:
    QCommonJSModule *mCommonJsModule;

};

#endif // IMAGECOMBINER_H
