#include <syslog.h>

#include "nowreadingengine.h"


NowReadingEngine::NowReadingEngine(QObject *parent, const QVariantList& args) :
    Plasma::DataEngine(parent)
{
    Q_UNUSED(args);

}

void NowReadingEngine::init()
{
    syslog(LOG_ALERT, "I'm started! :)");
}

K_EXPORT_PLASMA_DATAENGINE(nowreading, NowReadingEngine)

