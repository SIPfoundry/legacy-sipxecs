
#include "EventSubscriber.h"


int main(int argc, char** argv)
{
  using namespace resip;

  OsServiceOptions::daemonize(argc, argv);
  OsServiceOptions service(argc, argv, "sipxsub", "1.0", "Ezuce Inc. All Rights Reserved");
  service.addDaemonOptions();

  service.addOptionString("local-uri", ": URI to be used in the From header",  OsServiceOptions::ConfigOption);
  service.addOptionString("resource-uri", ": URI of the resource",  OsServiceOptions::ConfigOption);
  service.addOptionString("resource-file", ": Path for the resource-lists.xml file",  OsServiceOptions::ConfigOption);
  service.addOptionString("proxy-uri", ": URI of the proxy that will handle the subscription",  OsServiceOptions::ConfigOption);
  service.addOptionString("resource-type", ": The event ID for the resource.  Example: dialog",  OsServiceOptions::ConfigOption);
  service.addOptionString("auth-domain", ": Domain/Realm used to authenticate with sipXproxy",  OsServiceOptions::ConfigOption);
  service.addOptionString("auth-user", ": User used to authenticate with sipXproxy",  OsServiceOptions::ConfigOption);
  service.addOptionString("auth-pass", ": Password used to authenticate with sipXproxy",  OsServiceOptions::ConfigOption);
  service.addOptionInt("sip-port", "Port to be used by the SIP transport",  OsServiceOptions::ConfigOption);
  service.addOptionInt("refresh-interval", "The refresh intervals for subscriptions. (seconds)", OsServiceOptions::ConfigOption);

  if (!service.parseOptions())
    service.displayUsage(std::cerr);


  EventSubscriber sipxsub(service);
  sipxsub.run();
  sipxsub.subscribe();
  service.waitForTerminationRequest();
  sipxsub.stop();

  exit(0);
}