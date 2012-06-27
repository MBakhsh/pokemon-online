#include "clientscripting.h"
#include "../Teambuilder/engineinterface.h"
#include "scriptwindow.h"
#include "scriptengine.h"

ClientPlugin* createPluginClass(MainEngineInterface*)
{
    return new ClientScripting();
}

ClientScripting::ClientScripting()
{
}

bool ClientScripting::hasConfigurationWidget() const
{
    return true;
}

QString ClientScripting::pluginName() const
{
    return QObject::tr("Script Window");
}

QWidget *ClientScripting::getConfigurationWidget()
{
    ScriptWindow *ret = new ScriptWindow();
    QObject::connect(ret, SIGNAL(scriptChanged(QString)), engine.data(), SLOT(changeScript(QString)));
    return ret;
}

OnlineClientPlugin* ClientScripting::getOnlinePlugin(ClientInterface *c)
{
    return engine = new ScriptEngine(c);
}