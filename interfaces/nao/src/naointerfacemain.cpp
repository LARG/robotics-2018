/**
 * \mainpage
 * \section Author
 * @author Michael Quinlan
 */

#include <boost/shared_ptr.hpp>
#include <alcommon/albroker.h>
#include <alcommon/almodule.h>
#include <alcommon/albrokermanager.h>
#include <alcommon/altoolsmain.h>

#include "naointerface.h"
#include "AudioWrapper.h"

#ifdef NAOINTERFACE_IS_REMOTE_OFF

# ifdef _WIN32
    #define ALCALL __declspec(dllexport)
# else
    #define ALCALL
# endif

#else
    #define ALCALL
#endif

#ifdef __cplusplus
extern "C"
{
#endif


ALCALL int _createModule(boost::shared_ptr<AL::ALBroker> pBroker) {
  AL::ALBrokerManager::setInstance(pBroker->fBrokerManager.lock());
  AL::ALBrokerManager::getInstance()->addBroker(pBroker);
  AL::ALModule::createModule<naointerface>( pBroker, "naointerface" );
  AL::ALModule::createModule<AudioWrapper>(pBroker, "AudioWrapper");
  return 0;
}

ALCALL int _closeModule() {
  return 0;
}

# ifdef __cplusplus
}
# endif

#ifdef NAOINTERFACE_IS_REMOTE_ON

int main( int argc, char *argv[] )
{
  // pointer to createModule
  TMainType sig;
  sig = &_createModule;

  // call main
  ALTools::mainFunction("naointerface",argc, argv,sig);
}
#endif

