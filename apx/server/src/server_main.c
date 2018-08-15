#define CLEANUP_TEST 0                   //0=no cleanup test (default), 1=enable cleanup test
//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#ifdef _MSC_VER
#if CLEANUP_TEST
#   define _CRTDBG_MAP_ALLOC
#   include <stdlib.h>
#   include <crtdbg.h>
#endif
#include <Windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif
#include "apx_server.h"
#include "apx_types.h"
#include "apx_logging.h"
#include "apx_eventListener.h"
#include "apx_eventRecorderSrvTxt.h"
#include "apx_eventRecorderSrvRmfMgr.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//////////////////////////////////////////////////////////////////////////////
// CONSTANTS AND DATA TYPES
//////////////////////////////////////////////////////////////////////////////
#define DEFAULT_PORT 5000
#define CLEANUP_TEST_DURATION_SEC 30     //number of seconds before server shutdown is triggered in a cleanup test

//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static int parse_args(int argc, char **argv);
static void signal_handler_setup(void);
void signal_handler(int signum);
static void printUsage(char *name);

//////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
//////////////////////////////////////////////////////////////////////////////
int8_t g_debug; // Global so apx_logging can use it from everywhere
int m_runFlag = 1;

//////////////////////////////////////////////////////////////////////////////
// LOCAL VARIABLES
//////////////////////////////////////////////////////////////////////////////
static uint16_t m_port;
static apx_server_t m_server;
#if CLEANUP_TEST
static int32_t m_count;
#endif

static const char *SW_VERSION_STR = SW_VERSION_LITERAL;
//////////////////////////////////////////////////////////////////////////////
// GLOBAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
   apx_eventRecorderSrvTxt_t *eventRecorderSrvTxt;
   apx_eventRecorderSrvRmfMgr_t *apx_eventRecorderSrvRmfMgr;
#ifdef _WIN32
   WORD wVersionRequested;
   WSADATA wsaData;
   int err;
#endif
#if CLEANUP_TEST
   m_count = CLEANUP_TEST_DURATION_SEC;
#endif
   g_debug = 0;
   m_port = DEFAULT_PORT;
   m_runFlag = 1;
   printf("APX Server %s\n", SW_VERSION_STR);
   if(argc>1)
   {
      int result = parse_args(argc, argv);
      if (result != 0)
      {
         return 0;
      }
   }
   if(m_port==0)
   {
      printUsage(argv[0]);
      return 0;
   }
   APX_LOG_INFO("Listening on port %d\n", (int)m_port);
#ifdef _WIN32   
   wVersionRequested = MAKEWORD(2, 2);
   err = WSAStartup(wVersionRequested, &wsaData);
   if (err != 0) {
      /* Tell the user that we could not find a usable Winsock DLL*/
      APX_LOG_ERROR("WSAStartup failed with error: %d\n", err);
      return 1;
   }
#endif
   signal_handler_setup();
   apx_server_create(&m_server, m_port);
   apx_eventRecorderSrvRmfMgr = apx_eventRecorderSrvRmfMgr_new(APX_EVENT_RECORDER_RMF_DEFAULT_UPDATE_TIME);
   if (apx_eventRecorderSrvRmfMgr != 0)
   {
      apx_server_registerConnectionEventListener(&m_server, (apx_connectionEventListener_t*) apx_eventRecorderSrvRmfMgr);
   }
   eventRecorderSrvTxt = apx_eventRecorderSrvTxt_new();
   if (eventRecorderSrvTxt != 0)
   {
      apx_eventRecorderSrvTxt_open(eventRecorderSrvTxt, "server.apxlog");
      apx_server_registerConnectionEventListener(&m_server, (apx_connectionEventListener_t*) eventRecorderSrvTxt);
   }
   apx_server_setDebugMode(&m_server, g_debug);
   apx_server_start(&m_server);
   while(m_runFlag != 0)
   {
      SLEEP(1000); //main thread is sleeping while child threads do all the work
#if CLEANUP_TEST
    if (--m_count==0) //this counter is used during a cleanup test to verify that all resources are properly cleaned up
    {
       break;
    }
    else
    {
       printf("Shutdown in %d\n", m_count);
    }
#endif
   }
   printf("cleaning up\n");
   apx_server_destroy(&m_server);
   apx_eventRecorderSrvTxt_delete(eventRecorderSrvTxt);
#ifdef _WIN32
   WSACleanup();
#endif
#if defined(_MSC_VER) && (CLEANUP_TEST != 0)
   _CrtDumpMemoryLeaks();
#endif
   return 0;
}


//////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTIONS
//////////////////////////////////////////////////////////////////////////////

static void signal_handler_setup(void)
{
   if(signal (SIGINT, signal_handler) == SIG_IGN) {
      signal (SIGINT, SIG_IGN);
   }
   if(signal (SIGTERM, signal_handler) == SIG_IGN) {
      signal (SIGTERM, SIG_IGN);
   }
}

void signal_handler(int signum)
{
   (void)signum;
   m_runFlag = false;
}

static int parse_args(int argc, char **argv)
{
   int i;
   for(i=1;i<argc;i++)
   {
      if (strncmp(argv[i],"-p=",3)==0)
      {
         char *endptr=0;
         long num = strtol(&argv[i][3],&endptr,10);
         if (endptr > &argv[i][3])
         {
            m_port=(int)num;
         }
      }
      else if (strncmp(argv[i],"-p",2)==0)
      {
         char *endptr=0;
         long num = strtol(&argv[i][2],&endptr,10);
         if (endptr > &argv[i][2])
         {
            m_port=(int)num;
         }
      }      
      else if (strncmp(argv[i], "-h", 2) == 0)
      {
         printUsage(argv[0]);
         return -1;
      }
      else if (strncmp(argv[i], "--debug=", 8) == 0)
      {
         char *endptr=0;
         long num = strtol(&argv[i][8],&endptr,10);
         if (endptr > &argv[i][8])
         {
            g_debug=(int8_t) num;
            if (g_debug == APX_DEBUG_1_PROFILE)
            {
               // Disable stdout buffer to get accurate log timestamps
               setvbuf(stdout, NULL, _IONBF, 0);
            }
         }
      }
      else
      {
         printf("Unknown argument %s\n", argv[i]);
         printUsage(argv[0]);
         return -1;
      }
   }
   return 0;
}

static void printUsage(char *name)
{   
   printf("%s -p<port> [--debug=<level 1-4>]\n",name);
}
