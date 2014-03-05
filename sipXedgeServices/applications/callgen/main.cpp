#include <boost/program_options.hpp>
#include "CallManager.h"

namespace opt = boost::program_options;


int main(int argc, char** argv)
{
  CallManager manager(argc, argv);
  
  manager.start();
  manager.waitForTerminationRequest();
  manager.stop();

  return 0;
}
