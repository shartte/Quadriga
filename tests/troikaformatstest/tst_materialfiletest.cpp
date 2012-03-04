#include <QtCore/QString>
#include <QtTest/QtTest>

#include "troikaformats/materialdefinition.h"
#include "troikaformats/materialfile.h"

inline bool operator ==(const TroikaColor &a, const TroikaColor &b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}

class MaterialFileTest : public QObject
{
    Q_OBJECT
    
public:
    MaterialFileTest();
    
private Q_SLOTS:
    void testSimpleFile();
};

MaterialFileTest::MaterialFileTest()
{
}

void MaterialFileTest::testSimpleFile()
{
    QByteArray data = "Color 255 255 255 255";
    QBuffer buffer(&data);
    TroikaMaterialDefinition def = TroikaMaterialFile::parse("myMaterialFile.mdf", &buffer);

    QCOMPARE(def.name(), QString("myMaterialFile.mdf"));
    QCOMPARE(def.color(), TroikaColor(255, 255, 255, 255));
}

QTEST_APPLESS_MAIN(MaterialFileTest)

#include "tst_materialfiletest.moc"
