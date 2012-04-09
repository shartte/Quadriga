#ifndef TROIKAFORMATS_ARCHIVEENTRY_H
#define TROIKAFORMATS_ARCHIVEENTRY_H

/**
  Represents an entry in the Troika archives.
  */
struct TroikaArchiveEntry {

    TroikaArchiveEntry();

    enum Type
    {
        File = 1,
        CompressedFile = 2,
        Directory = 1024
    };

    bool isDirectory() const {
        return type == Directory;
    }

    bool isCompressed() const {
        return type == CompressedFile;
    }

    static const char Separator = '/';

    Type type;
    QByteArray filename;
    TroikaArchiveEntry *parent;
    TroikaArchiveEntry *firstChild;
    TroikaArchiveEntry *nextSibling;

    int compressedSize;
    int uncompressedSize;
    int dataStart;
};

inline TroikaArchiveEntry::TroikaArchiveEntry()
    : parent(nullptr), firstChild(nullptr), nextSibling(nullptr),
      compressedSize(0), uncompressedSize(0), dataStart(0)
{
}

#endif // TROIKAFORMATS_ARCHIVEENTRY_H
