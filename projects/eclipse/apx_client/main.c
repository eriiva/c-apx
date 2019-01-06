#include <stdio.h>
#include "ApxNode_test_client.h"
#include "apx_client.h"
#include "unistd.h"
#include "osmacro.h"

static apx_client_t m_client;
static apx_nodeData_t *m_nodeData;
int8_t g_debug;

static const char *m_apxDefinition=
"APX/1.2\n"
"N\"test_client\"\n"
"T\"OffOn_T\"C(0,3):VT(\"OffOn_Off\",\"OffOn_On\",\"OffOn_Error\",\"OffOn_NotAvailable\")\n"
"T\"Percent_T\"C\n"
"T\"VehicleSpeed_T\"S\n"
"R\"EngineRunningStatus\"T[0]:=3\n"
"R\"FuelLevelPercent\"T[1]:=255\n"
"R\"VehicleSpeed\"T[2]:=0xFFFF\n"
"\n";


static void print_values(void)
{
   OffOn_T EngineRunningStatus;
   Percent_T FuelLevelPercent;
   VehicleSpeed_T  VehicleSpeed;
   ApxNode_Read_test_client_EngineRunningStatus(&EngineRunningStatus);
   ApxNode_Read_test_client_FuelLevelPercent(&FuelLevelPercent);
   ApxNode_Read_test_client_VehicleSpeed(&VehicleSpeed);
   printf("EngineRunningStatus=%d FuelLevelPercent=%.1f (%d)", (int) EngineRunningStatus, ((double) FuelLevelPercent)*0.4, (int) FuelLevelPercent);
   printf(" VehicleSpeed=%.1f (%d)\n",((double)VehicleSpeed)/64.0, (int) VehicleSpeed );
}

static void run(void)
{
   print_values();
}

int main(int argc, char **argv)
{
   apx_error_t result;
   ApxNode_Init_test_client();
   m_nodeData = ApxNode_GetNodeData_test_client();
   apx_client_create(&m_client);
   //apx_client_attachLocalNode(&m_client, m_nodeData);
   apx_client_attachLocalNodeFromString(&m_client, m_apxDefinition);
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
   return 0;
}
