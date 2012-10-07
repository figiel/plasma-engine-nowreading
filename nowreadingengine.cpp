#include "nowreadingengine.h"

#include <KDE/KStandardDirs>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QXmlStreamReader>

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


bool convertOkularXMLFileToEntry(const QString &okularFilePath, NowReadingEntry &outEntry)
{
    /* open the Okular document history file */

    QFile okularFile(okularFilePath);
    if (!okularFile.open(QFile::ReadOnly))
        return false;

    QXmlStreamReader xml(&okularFile);
    QString vp;

    while (!xml.atEnd()) {
        if (QXmlStreamReader::StartElement != xml.readNext())
            continue;
        if (xml.name() == "documentInfo") {
            outEntry.path = xml.attributes().value("url").toString();
        }
        else if (xml.name() == "current") {
            vp = xml.attributes().value("viewport").toString();
            break;
        }
    }

    /* get PDF file path: */

    if (outEntry.path.isEmpty())
        return false;

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

QStringList getOkularXMLFiles()
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
    QStringList okularFiles = getOkularXMLFiles();
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
