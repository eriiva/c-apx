#include <stdio.h>

#include "apx_client.h"
#include "unistd.h"
#include "osmacro.h"
#include "apx_parser.h"
#include "apx_error.h"
#include "filestream.h"

static apx_client_t m_client;
static apx_nodeData_t *m_nodeData;
int8_t g_debug;






int main(int argc, char **argv)
{
   apx_error_t result;
   if (argc > 1)
   {
      adt_bytearray_t *apx_bytes;
      const char *filename = argv[1];

      apx_bytes = ifstream_util_readTextFile(filename);
      if (apx_bytes != 0)
      {
         apx_parser_t *parser;
         const char *apx_text  = (const char*) adt_bytearray_data(apx_bytes);
         parser = apx_parser_new();
         m_nodeData = apx_nodeData_makeFromString(parser, apx_text);
         apx_parser_delete(parser);
         if (m_nodeData != 0)
         {
            apx_client_create(&m_client);
            result = apx_client_attachLocalNode(&m_client, m_nodeData);
            if (result != APX_NO_ERROR)
            {
               printf("Attach error %d\n", result);
            }
            result = apx_client_connectTcp(&m_client, "127.0.0.1", 5000);
            if (result == APX_NO_ERROR)
            {
               apx_clientConnectionBase_t *connection = apx_client_getConnection(&m_client);
               for(;;)
               {
                  SLEEP(1000);
                  printf("RX: %d\tTX: %d\n", (int) apx_clientConnectionBase_getTotalBytesReceived(connection), (int) apx_clientConnectionBase_getTotalBytesSent(connection) );
               }
            }
            apx_client_destroy(&m_client);
         }
         adt_bytearray_delete(apx_bytes);
      }
      else
      {
         printf("APX error %d\n", apx_getLastError());
      }
   }
   else
   {
      printf("Not enough arguments\n");
   }

   return 0;
}
