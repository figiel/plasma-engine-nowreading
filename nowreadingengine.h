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
  *  currentPage  - number of page the reader is currently on
  *  totalPages   - number of pages the document has
  *
  */
class NowReadingEngine : public Plasma::DataEngine
{
    Q_OBJECT
public:
    NowReadingEngine(QObject *parent, const QVariantList& args);
    void init();
signals:
    
public slots:
    
};

#endif // NOWREADINGENGINE_H
