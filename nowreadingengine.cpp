#include "nowreadingengine.h"

#include <KDE/KStandardDirs>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QXmlStreamReader>
#include <poppler-qt4.h>

#include <iostream>
#include <syslog.h>

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

class PagesNumberProvider {
public:
    virtual bool getNumberOfPages(uint &pageNumberOut) = 0;
    virtual ~PagesNumberProvider() { }
};

class PDFPagesNumberProvider : public PagesNumberProvider {
public:
    PDFPagesNumberProvider(const QString &filePath)
        : m_filePath(filePath)
    {
        static PopplerInitializer pInit;
    }

    bool getNumberOfPages(uint &pageNumberOut)
    {
        QScopedPointer<Poppler::Document> document(Poppler::Document::load(m_filePath));

        if (!document)
            return false;

        pageNumberOut = document->numPages();

        return true;
    }
    class PopplerInitializer {
    public:
        static void quietPoppler(const QString &, const QVariant &) { }
        PopplerInitializer() {
            Poppler::setDebugErrorFunction(quietPoppler, 0);
        }
    };

private:
    QString m_filePath;
};

class PagesNumberProviderFactory {
public:
    static PagesNumberProvider *createPagesNumberProviderByFileName(const QString &fileName)
    {
        if (fileName.endsWith(".pdf", Qt::CaseInsensitive))
            return new PDFPagesNumberProvider(fileName);

        return NULL;
    }
};

bool convertOkularXMLFileToEntry(const QString &okularFilePath, NowReadingEntry &outEntry)
{
    /* open the Okular document history file */

    QFile okularFile(okularFilePath);
    if (!okularFile.open(QFile::ReadOnly))
        return false;

    QXmlStreamReader xml(&okularFile);
    QString vp;

    outEntry.path = QString();

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

    if (outEntry.path.isEmpty())
        return false;

    /* get the page number from viewport string: */
    QStringList tokens = vp.split(";");
    if (tokens.empty())
        return false;

    bool pageOk = true;
    outEntry.currentPage = tokens.first().toUInt(&pageOk) + 1 /* the value in vp is 0-based */;

    if (!pageOk)
        return false;

    QScopedPointer<PagesNumberProvider> pageNumberProvider
            (PagesNumberProviderFactory::createPagesNumberProviderByFileName(outEntry.path));

    if (!pageNumberProvider)
        return false;

    if (!pageNumberProvider->getNumberOfPages(outEntry.totalPages))
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
    Q_UNUSED(argc);
    Q_UNUSED(argv);

    QStringList okularFiles = getOkularXMLFiles();
    QStringList::const_iterator it = okularFiles.constBegin();

    for ( ; it != okularFiles.constEnd(); ++it) {
        NowReadingEntry entry;
        if (convertOkularXMLFileToEntry(*it, entry)) {
            cout << entry.path.toStdString() << ":" << entry.currentPage << "/" << entry.totalPages << endl;
        }
    }

}


void NowReadingEngine::init()
{
    QStringList okularFiles = getOkularXMLFiles();
    QStringList::const_iterator it = okularFiles.constBegin();

    for ( ; it != okularFiles.constEnd(); ++it) {
        NowReadingEntry entry;
        if (convertOkularXMLFileToEntry(*it, entry)) {
            setData(entry.path, "currentPage", entry.currentPage);
            setData(entry.path, "totalPages", entry.totalPages);
        }
    }
}

K_EXPORT_PLASMA_DATAENGINE(nowreading, NowReadingEngine)
