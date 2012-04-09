
#include <QString>
#include <QDebug>
#include <QIODevice>

#include "troikaformats/materialfile.h"
#include "troikaformats/materialdefinition.h"

#include "materialcommand.h"

// Character used to quote arguments in material files
static const char QuoteChar = '"';

// Parses a line and applies its effects to the material definition
static void parseLine(const QByteArray &line, TroikaMaterialDefinition &material);
static QStringList tokenizeLine(const QByteArray &line);
static bool applyCommand(const QString &command, const QStringList &args, TroikaMaterialDefinition &material);

TroikaMaterialDefinition TroikaMaterialFile::parse(const QString &name, QIODevice *materialFile)
{
    Q_ASSERT(materialFile->canReadLine());

    TroikaMaterialDefinition result;
    result.setName(name);
    result.setType(TroikaMaterialDefinition::UserDefined);

    while (!materialFile->atEnd()) {
        QByteArray line = materialFile->readLine();
        parseLine(line, result);
    }

    return result;
}

static void parseLine(const QByteArray &line, TroikaMaterialDefinition &material)
{
    QStringList args = tokenizeLine(line);

    if (args.isEmpty())
        return;

    QString command = args.takeFirst();

    if (!applyCommand(command, args, material))
        qWarning() << "Material %s has incorrect command: %s" << material.name() << line;
}

static QStringList tokenizeLine(const QByteArray &line)
{
    bool inQuotes = false; // No escape characters allowed
    QString tokenBuffer;
    QStringList tokens;

    foreach (char asciiCh, line) {
        QChar ch = QChar::fromAscii(asciiCh);
        if (ch.isSpace() && !inQuotes) {
            if (!tokenBuffer.isEmpty())
                tokens.append(tokenBuffer);
            tokenBuffer.clear();
        } else if (ch == QuoteChar) {
            if (inQuotes) {
                tokens.append(tokenBuffer);
                tokenBuffer.clear();
            }
            inQuotes = !inQuotes;
        } else {
            tokenBuffer.append(ch);
        }
    }

    // Dont forget about the last token
    if (!tokenBuffer.isEmpty()) {
        tokens.append(tokenBuffer);
    }

    return tokens;
}

static bool applyCommand(const QString &command, const QStringList &args, TroikaMaterialDefinition &material)
{
    if (!command.compare("Texture", Qt::CaseInsensitive)) {
        int unit;
        QString texture;

        if (args.size() == 1) {
            unit = 0;
            texture = args[0];

        } else if (args.size() == 2) {
            bool ok;
            unit = args[0].toInt(&ok);

            if (!ok || unit < 0 || unit >= TroikaMaterialDefinition::MaxTextureStages)
                return false; // Wrong texture unit format

            texture = args[1];
        } else {
            return false; // More arguments than needed
        }

        material.textureStage(unit).setFilename(texture);
        return true;

    } else if (!command.compare("Glossmap", Qt::CaseInsensitive)) {

        if (args.size() != 1) {
            qWarning("Glossmap has invalid args: %s", qPrintable(args.join(" ")));
            return false;
        }

        material.setGlossMap(args[0]);

        return true;

    } else if (!command.compare("ColorFillOnly", Qt::CaseInsensitive)) {
        material.setDepthTestDisabled(true);

    } else if (!command.compare("BlendType", Qt::CaseInsensitive)) {

        if (args.size() != 2) {
            qWarning("BlendType has invalid args: %s", qPrintable(args.join(" ")));
            return false;
        }

        bool ok;
        int unit = args[0].toInt(&ok);

        // Check the texture unit id for sanity
        if (!ok || unit < 0 || unit >= TroikaMaterialDefinition::MaxTextureStages)
            return false;

        TroikaTextureStageInfo &stage = material.textureStage(unit);
        QString type = args[1].toLower();

        if (type == "modulate") {
            stage.setBlendType(TroikaTextureStageInfo::Modulate);
        } else if (type == "add") {
            stage.setBlendType(TroikaTextureStageInfo::Add);
        } else if (type == "texturealpha") {
            stage.setBlendType(TroikaTextureStageInfo::TextureAlpha);
        } else if (type == "currentalpha") {
            stage.setBlendType(TroikaTextureStageInfo::CurrentAlpha);
        } else if (type == "currentalphaadd") {
            stage.setBlendType(TroikaTextureStageInfo::CurrentAlphaAdd);
        } else {
            qWarning("Unknown blend type for texture stage %d: %s", unit, qPrintable(type));
            return false;
        }

        return true;

    } else if (!command.compare("Speed", Qt::CaseInsensitive)) {
        // Sets both U&V speed for all stages
        if (args.size() == 1) {
            bool ok;
            float speed = args[0].toFloat(&ok);

            if (!ok) {
                qWarning("Invalid UV speed.");
                return false;
            }

            for (int i = 0; i < TroikaMaterialDefinition::MaxTextureStages; ++i) {
                material.textureStage(i).setSpeedU(speed);
                material.textureStage(i).setSpeedV(speed);
            }

            return true;
        } else {
            qWarning("Invalid arguments for texture command %s", qPrintable(command));
            return false;
        }

    } else if (!command.compare("Specularpower", Qt::CaseInsensitive)) {
        if (args.size() == 1)
        {
            bool ok;
            material.setSpecularPower(args[0].toFloat(&ok));

            if (!ok) {
                qWarning("Invalid specular power.");
                return false;
            }

            return true;
        }
        else
        {
            qWarning("Invalid arguments for texture command %s", qPrintable(command));
            return false;
        }

    } else if (!command.compare("SpeedU", Qt::CaseInsensitive)
        || !command.compare("SpeedV", Qt::CaseInsensitive)) {

        // Sets a transform speed for one stage
        if (args.size() == 2)
        {
            bool ok;
            float speed = args[1].toFloat(&ok);

            if (!ok) {
                qWarning("Invalid UV speed.");
                return false;
            }

            int stage = args[0].toInt(&ok);

            if (!ok || stage < 0 || stage >= TroikaMaterialDefinition::MaxTextureStages)
            {
                qWarning("Invalid texture stage %s.", qPrintable(args[0]));
                return false;
            }

            if (!command.compare("SpeedU", Qt::CaseInsensitive))
                material.textureStage(stage).setSpeedU(speed);
            else
                material.textureStage(stage).setSpeedV(speed);

            return true;
        }
        else
        {
            qWarning("Invalid arguments for texture command %s", qPrintable(command));
            return false;
        }

    } else if (!command.compare("UVType", Qt::CaseInsensitive)) {

        // Sets the transform type for one stage
        if (args.size() == 2)
        {
            bool ok;
            int stage = args[0].toInt(&ok);

            if (!ok || stage < 0 || stage >= TroikaMaterialDefinition::MaxTextureStages)
            {
                qWarning("Invalid texture stage %s.", qPrintable(args[0]));
                return false;
            }

            QString type = args[1].toLower();

            if (type == "mesh")
                material.textureStage(stage).setUvType(TroikaTextureStageInfo::Mesh);
            else if (type == "drift")
                material.textureStage(stage).setUvType(TroikaTextureStageInfo::Drift);
            else if (type == "swirl")
                material.textureStage(stage).setUvType(TroikaTextureStageInfo::Swirl);
            else if (type == "wavey")
                material.textureStage(stage).setUvType(TroikaTextureStageInfo::Wavey);
            else if (type == "environment")
                material.textureStage(stage).setUvType(TroikaTextureStageInfo::Environment);
            else
                return false;

            return true;
        }
        else
        {
            qWarning("Invalid arguments for texture command %s", qPrintable(command));
            return false;
        }

    } else if (!command.compare("MaterialBlendType", Qt::CaseInsensitive)) {
        if (args.size() == 1) {
            QString type = args[0].toLower();

            if (type == "none") {
                material.setBlendType(TroikaMaterialDefinition::None);
            } else if (type == "alpha") {
                material.setBlendType(TroikaMaterialDefinition::Alpha);
            } else if (type == "add") {
                material.setBlendType(TroikaMaterialDefinition::Add);
            } else if (type == "alphaadd") {
                material.setBlendType(TroikaMaterialDefinition::AlphaAdd);
            } else {
                qWarning("Unknown MaterialBlendType type: %s", qPrintable(type));
                return false;
            }

            return true;
        } else {
            qWarning("Material blend type has invalid arguments: %s", qPrintable(args.join(" ")));
            return false;
        }
    } else if (!command.compare("Double", Qt::CaseInsensitive)) {
        material.setFaceCullingDisabled(true);
        return true;
    } else if (!command.compare("notlit", Qt::CaseInsensitive)
        || !command.compare("notlite", Qt::CaseInsensitive)) {
        // We also compare against notlite, since it's a very common mistake in the vanilla MDFs
        material.setLightingDisabled(true);
        return true;
    } else if (!command.compare("DisableZ", Qt::CaseInsensitive)) {
        material.setDepthTestDisabled(true);
        return true;
    } else if (!command.compare("General", Qt::CaseInsensitive)
        || !command.compare("HighQuality", Qt::CaseInsensitive)) {
        // This was previously used by the material system to define materials
        // for different quality settings. The current hardware performance makes this
        // unneccessary. A better way to deal with this could be to ignore the "general"
        // definition completely.
        return true;
    } else if (!command.compare("LinearFiltering", Qt::CaseInsensitive)) {
        material.setLinearFiltering(true);
        return true;
    } else if (!command.compare("Textured", Qt::CaseInsensitive)) {
        // Unused
        return true;
    } else if (!command.compare("RecalculateNormals", Qt::CaseInsensitive)) {
        material.setRecalculateNormals(true);
        return true;
    } else if (!command.compare("Color", Qt::CaseInsensitive)) {
        if (args.count() != 4) {
            qWarning("Color needs 4 arguments: %s", qPrintable(args.join(" ")));
            return false;
        }

        int rgba[4];
        for (int i = 0; i < 4; ++i) {
            bool ok;
            rgba[i] = args[i].toUInt(&ok);
            if (!ok) {
                qWarning("Color argument %d is invalid: %s", i, qPrintable(args[i]));
                return false;
            }
            if (rgba[i] > 255) {
                qWarning("Color argument %d is out of range (0-255): %d", i, rgba[i]);
                return false;
            }
        }

        TroikaColor color(rgba[0], rgba[1], rgba[2], rgba[3]);
        material.setColor(color);

        return true;
    }

    return false; // Unknown command
}
