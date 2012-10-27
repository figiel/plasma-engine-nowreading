#ifndef NOWREADINGENGINE_H
#define NOWREADINGENGINE_H

#include <Plasma/DataEngine>

/** @class NowReadingEngine
  * @short Provide list of recently read documents and progress in reading.
  *
  * This engine exposes interface to get information about:
  *  - recently read files
  *  - progress in reading (i.e. overall number of pages and pages read so far)
  *
  * The information about actual tool used for reading and source of this information is not
  * actually exposed by the interface.
  *
  * At the moment only reading of PDF files by Okular is supported.
  *
  * Interface itself consists of sources and keyed data.
  * Single document is represented as single source, name of the source is the filename.
  *
  * For single source there are following data available:
  *  currentPage  - number of page the reader is currently on,
  *  totalPages   - number of pages the document has,
  *  accessTime   - document access time, which is actually last modification time of the Okular history file.
  *
  */

namespace nowreading {

/** \class NowReadingEntry
    \brief Single entry which will be provided to presentation layer describing recently read document. */
struct NowReadingEntry {
    QString path;          /** Path to the document which is read */
    uint currentPage;      /** Current page the user was at, when last closing the Okular session */
    uint totalPages;       /** Total pages of the document */
    QDateTime accessTime;  /** Document access time, which is actually last modification time of the Okular history file. */

    bool operator < (const NowReadingEntry &other) const
    {
        // From the most recent to the oldest
        return accessTime > other.accessTime;
    }
};

class NowReadingEngine : public Plasma::DataEngine
{
    Q_OBJECT
public:
    NowReadingEngine(QObject *parent, const QVariantList& args);
    void init();
public slots:
    void update();
signals:

private:
    /** Map of known now reading entries, indexed by the Okular history filename. */
    typedef QMap<QString, NowReadingEntry> DataModel;
    DataModel _dataModel;
    enum {
        MaxDataModelSize = 10,
        PollingInterval = 2000
    };

};

} /* namespace nowreading */

#endif /* NOWREADINGENGINE_H */
