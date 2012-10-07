#include "nowreadingengine.h"

#include <KDE/KStandardDirs>

#include <QtGui/QApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtXmlPatterns/QXmlQuery>
#include <QtXmlPatterns/QXmlResultItems>

#include <syslog.h>

#include <iostream>

using namespace std;

NowReadingEngine::NowReadingEngine(QObject *parent, const QVariantList& args) :
    Plasma::DataEngine(parent)
{
    Q_UNUSED(args);

}

struct NowReadingEntry {
    QString path;
    uint currentPage;
    uint totalPages;
};

namespace {
QString getAttributeValueFromXML(const QString &xpath, QIODevice &file)
{
    QString ret("");
    QXmlQuery query(QXmlQuery::XQuery10);
    query.setFocus(&file);
    query.setQuery(xpath);

    if (!query.isValid())
        return ret;

    QXmlResultItems results;
    query.evaluateTo(&results);

    QXmlItem result(results.next());

    if (result.isNode())
        cout << "is Null" << endl;

    if (!result.isNull() && result.isAtomicValue() && result.toAtomicValue().isValid())
        ret = result.toAtomicValue().toString();

    return ret;

}
} // namespace

bool convertOkularXMLFileToEntry(const QString &okularFile, NowReadingEntry &outEntry)
{
    /* open the Okular document history file */
    QFile xmlFile(okularFile);
    if (!xmlFile.open(QFile::ReadOnly))
        return false;

    /* get PDF file path: */
    outEntry.path = getAttributeValueFromXML("//documentInfo/attribute::url/data(.)", xmlFile);
    if (outEntry.path.isEmpty())
        return false;

    xmlFile.reset();

    /* get saved viewport for most recent entry: */
    QString vp = getAttributeValueFromXML("//documentInfo/generalInfo/history/current/attribute::viewport/data(.)", xmlFile);

    /* get the page number from viewport string: */
    QStringList tokens = vp.split(";");
    if (tokens.empty())
        return false;

    bool pageOk = true;
    outEntry.currentPage = tokens.first().toUInt(&pageOk);

    if (!pageOk)
        return false;

    return true;
}

QStringList getOkularConfigurationFiles()
{
    QStringList ret;
    QDir okularHistoryDir (KStandardDirs::locateLocal( "data", "okular/docdata/" ));
    okularHistoryDir.setNameFilters(QStringList("*.xml"));
    okularHistoryDir.setFilter(QDir::Readable | QDir::Files);
    QFileInfoList files = okularHistoryDir.entryInfoList();

    QFileInfoList::const_iterator it = files.constBegin();

    for ( ; it != files.constEnd(); ++it) {
        ret << (*it).absoluteFilePath();
    }

    return ret;
}

int main(int argc, char **argv) {

    QApplication app(argc, argv);

    QStringList okularFiles = getOkularConfigurationFiles();
    QStringList::const_iterator it = okularFiles.constBegin();

    for ( ; it != okularFiles.constEnd(); ++it) {
        NowReadingEntry entry;
        if (convertOkularXMLFileToEntry(*it, entry)) {
            cout << entry.path.toStdString() << ":" << entry.currentPage << endl;
        }
    }

}



void NowReadingEngine::init()
{
    syslog(LOG_ALERT, "I'm started! :)");
}

K_EXPORT_PLASMA_DATAENGINE(nowreading, NowReadingEngine)
