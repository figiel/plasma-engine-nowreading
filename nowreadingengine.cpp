#include "nowreadingengine.h"

#include <KDE/KStandardDirs>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtCore/QXmlStreamReader>
#include <poppler-qt4.h>

#include <algorithm>
#include <iostream>
#include <syslog.h>

using namespace std;

/** \{ */

namespace nowreading {

NowReadingEngine::NowReadingEngine(QObject *parent, const QVariantList& args) :
    Plasma::DataEngine(parent)
{
    Q_UNUSED(args);

}


/** \class PagesNumberProvider
    \brief Provides a number of pages in a document. */
class PagesNumberProvider {
public:

    /** \brief Factory method, creates implementation of PagesNumberProvider interface
               by given filename.
        \note Returned object is allocated on heap, caller must free. */
    static PagesNumberProvider *createByFileName(const QString &fileName);

    /** \brief Return the number of pages in a document. */
    virtual bool getNumberOfPages(uint &pageNumberOut) = 0;
    virtual ~PagesNumberProvider() { }
};


/** \class PDFPagesNumberProvider
    \brief Provides a number of pages in a PDF document. */
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
private:
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


PagesNumberProvider *PagesNumberProvider::createByFileName(const QString &fileName)
{
    if (fileName.endsWith(".pdf", Qt::CaseInsensitive))
        return new PDFPagesNumberProvider(fileName);

    return NULL;
}

/** \brief For given file with Okular browsing history returns a single NowReadingEntry. */
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
            (PagesNumberProvider::createByFileName(outEntry.path));

    if (!pageNumberProvider)
        return false;

    if (!pageNumberProvider->getNumberOfPages(outEntry.totalPages))
        return false;

    outEntry.accessTime = QFileInfo(okularFile).lastModified();

    return true;
}

/** \brief Browses Okular folder to get the files with browsing history.
 *
 *   Normally there's single okular history file for single document being read.
 */
QFileInfoList getOkularXMLFiles()
{
    QDir okularHistoryDir (KStandardDirs::locateLocal( "data", "okular/docdata/" ));
    okularHistoryDir.setNameFilters(QStringList("*.xml"));
    okularHistoryDir.setFilter(QDir::Readable | QDir::Files);
    okularHistoryDir.setSorting(QDir::Time);
    QFileInfoList files = okularHistoryDir.entryInfoList();
    return files;
}

void NowReadingEngine::update()
{
    QFileInfoList okularFiles = getOkularXMLFiles();
    QFileInfoList::const_iterator it = okularFiles.constBegin();
    uint limit = MaxDataModelSize;

    for ( ; it != okularFiles.constEnd() && limit--; ++it) {
        QString okularFileName = it->absoluteFilePath();
        DataModel::iterator itDm = _dataModel.find(okularFileName);

        if ((itDm != _dataModel.end()) && /* We have an entry for this element */
            (itDm.value().accessTime >= it->lastModified())) { /* And it's up to date */
                /* No need to update */
                continue;
        }

        NowReadingEntry entry;
        if (convertOkularXMLFileToEntry(okularFileName, entry)) {
            setData(entry.path, "currentPage", entry.currentPage);
            setData(entry.path, "totalPages", entry.totalPages);
            setData(entry.path, "accessTime", entry.accessTime);
            _dataModel[okularFileName] = entry;
        }
    }

    /* Cleanup */
    if (_dataModel.size() > MaxDataModelSize) {
        // Find elements to evict
        // First - what is the modification time of the last element we want to preserve?

        QDateTime oldestElementDate;
        QList<DataModel::mapped_type> values = _dataModel.values();
        nth_element(values.begin(), values.begin()+MaxDataModelSize-1, values.end());
        oldestElementDate = (values.begin()+MaxDataModelSize-1)->accessTime;

        // Now find all elements older than the oldest allowable
        DataModel::iterator it = _dataModel.begin();

        while (it != _dataModel.end()) {
            if (it.value().accessTime <  oldestElementDate) {
                removeSource(it.value().path);
                it = _dataModel.erase(it);
            }
            else {
                ++it;
            }
        }

    }

    /* Single shot timer used as in case the update takes very long
       we can get overflow of timer events to handle */
    QTimer::singleShot(PollingInterval, this, SLOT(update()));
}

/** Initially populate the DataEngine with NowReadingEntries */
void NowReadingEngine::init()
{
    _dataModel.clear();

    update();
}

} /* namespace nowreading */

/** \} */

K_EXPORT_PLASMA_DATAENGINE(nowreading, nowreading::NowReadingEngine)
