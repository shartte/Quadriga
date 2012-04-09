#include "conversion.h"
#include "troikaformats/archive.h"

Conversion::Conversion()
{
}

void Conversion::run(const QString &gamePath)
{

    TroikaArchive archive;
    archive.open(gamePath + "/ToEE1.dat");

    TroikaArchive::EntryList entries = archive.listAllFiles("*.mdf");

}

