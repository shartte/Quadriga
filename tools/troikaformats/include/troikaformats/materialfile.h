#if !defined(TROIKAFORMATS_MATERIALFILE_H)
#define TROIKAFORMATS_MATERIALFILE_H

#include "materialdefinition.h"

class QIODevice;
class QString;

class TroikaMaterialFile {
public:

    /**
        \brief Parses a Troika Material Definition File (MDF)
        and returns an object that contains the material definition.
      */
    static TroikaMaterialDefinition parse(const QString &name, QIODevice *materialFile);

};

#endif // TROIKAFORMATS_MATERIALFILE_H
