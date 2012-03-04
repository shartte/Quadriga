#ifndef TROIKAARCHIVE_H
#define TROIKAARCHIVE_H

class TroikaArchivePrivate;

class TroikaArchive
{
public:
    TroikaArchive();
    ~TroikaArchive();

    bool open(const QString &filename);
    void close();

private:
    TroikaArchivePrivate *d;
};

#endif // TROIKAARCHIVE_H
