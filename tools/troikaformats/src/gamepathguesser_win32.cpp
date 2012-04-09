
#include <QSettings>

#include "troikaformats/gamepathguesser.h"

/*
  Registry keys of the vanilla installer.
  */
static const QString VanillaRegistryKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{AD80F06B-0F21-4EEE-934D-BEF0D21E6383}");

static const QString VanillaRegistryValue("InstallLocation");

/*
  Registry keys of the GoG installer.
  */
static const QString GoGRegistryKey("HKEY_LOCAL_MACHINE\\SOFTWARE\\GOG.com\\GOGTEMPLEOFELEMENTAEVIL");

static const QString GoGRegistryValue("PATH");

QString GamePathGuesser::guessGamePath()
{
    /* We prefer the GoG version */

    QSettings gogSettings(GoGRegistryKey, QSettings::NativeFormat);

    if (gogSettings.contains(GoGRegistryValue))
        return gogSettings.value(GoGRegistryValue).toString();

    QSettings vanillaSettings(VanillaRegistryKey, QSettings::NativeFormat);

    /* Then try the Vanilla version */
    if (vanillaSettings.contains(VanillaRegistryValue))
        return vanillaSettings.value(VanillaRegistryValue).toString();

    return QString::null;

}
