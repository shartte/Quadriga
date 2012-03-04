
#include <QStringList>

class MaterialCommand {
public:

    enum Type {

        Unknown
    };

    Type type() const {
        return mType;
    }

    QString typeName() const {
        return mTypeName;
    }

    const QStringList &arguments() const {
        return mArguments;
    }

private:
    Type mType;
    QString mTypeName;
    QStringList mArguments;
};
