#include "nowreadingengine.h"

#include <KDE/KStandardDirs>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtXml/QXmlDefaultHandler>
#include <QtXml/QXmlSimpleReader>

#include <syslog.h>

#include <iostream>

using namespace std;

NowReadingEngine::NowReadingEngine(QObject *parent, const QVariantList& args) :
    Plasma::DataEngine(parent)
{
    Q_UNUSED(args);

}

class OkularXMLNowReadingHandler : public QXmlDefaultHandler {
public:
    bool startElement(const QString &, const QString &localName, const QString &, const QXmlAttributes &atts) {
        if (localName == "documentInfo") {
            m_path = atts.value("url");
        } else if (localName == "current") {
            m_viewport = atts.value("viewport");
        }
        return true;
    }

    bool fatalError(const QXmlParseException &) {
        return false;
    }

    QString getPath() const {
        return m_path;
    }

    QString getViewport() const {
        return m_viewport;
    }
private:
    QString m_path;
    QString m_viewport;
};

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

    QXmlSimpleReader reader;
    QXmlInputSource source(&okularFile);
    OkularXMLNowReadingHandler myHandler;
    reader.setErrorHandler(&myHandler);
    reader.setContentHandler(&myHandler);
    reader.parse(source);

    /* get PDF file path: */
    outEntry.path = myHandler.getPath();
    if (outEntry.path.isEmpty())
        return false;

    /* get saved viewport for most recent entry: */
    QString vp = myHandler.getViewport();

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
