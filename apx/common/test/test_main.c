#include <stdio.h>
#include "CuTest.h"
#ifdef MEM_LEAK_CHECK
#include "CMemLeak.h"
#endif

//apx
CuSuite* testsuite_apx_dataSignature(void);
CuSuite* testsuite_apx_port(void);
CuSuite* testSuite_apx_node(void);
CuSuite* testSuite_apx_parser(void);
CuSuite* testSuite_apx_portDataMap(void);
CuSuite* testSuite_apx_allocator(void);
CuSuite* testSuite_apx_fileMap(void);
CuSuite* testSuite_apx_nodeData(void);
CuSuite* testsuite_apx_attributesParser(void);
CuSuite* testSuite_apx_dataElement(void);
CuSuite* testSuite_remotefile(void);
//CuSuite* testSuite_apx_server(void);
CuSuite* testSuite_apx_serverSocketConnection(void);
CuSuite* testSuite_apx_fileManagerShared(void);
CuSuite* testSuite_apx_fileManagerRemote(void);
CuSuite* testSuite_apx_fileManagerLocal(void);
CuSuite* testSuite_apx_fileManager(void);
CuSuite* testSuite_apx_clientSocketConnection(void);
//CuSuite* testSuite_apx_client(void);
//CuSuite* testSuite_apx_nodeManager(void);
//CuSuite* testSuite_apx_eventRecorderSrvRmf(void);
CuSuite* testSuite_apx_file2(void);
CuSuite* testSuite_apx_nodeDataManager(void);
CuSuite* testsuite_apx_datatype(void);
CuSuite* testSuite_apx_bytePortMap(void);
CuSuite* testSuite_apx_eventLoop(void);
CuSuite* testSuite_apx_globalPortMap(void);
CuSuite* testSuite_apx_portTriggerList(void);
CuSuite* testSuite_apx_port_routing(void);
CuSuite* testSuite_apx_compiler(void);
CuSuite* testSuite_apx_nodeInfo(void);

//util
CuSuite* testsuite_pack(void);
CuSuite* testsuite_sha256(void);

void RunAllTests(void)
{
   CuString *output = CuStringNew();
   CuSuite* suite = CuSuiteNew();

   CuSuiteAddSuite(suite, testsuite_sha256());
   CuSuiteAddSuite(suite, testsuite_pack());

   CuSuiteAddSuite(suite, testSuite_apx_dataElement());
   CuSuiteAddSuite(suite, testsuite_apx_dataSignature());
   CuSuiteAddSuite(suite, testsuite_apx_port());
   CuSuiteAddSuite(suite, testsuite_apx_datatype());
   CuSuiteAddSuite(suite, testSuite_apx_node());
   CuSuiteAddSuite(suite, testSuite_apx_parser());
   CuSuiteAddSuite(suite, testSuite_apx_portDataMap());


   CuSuiteAddSuite(suite, testSuite_apx_file2());
   CuSuiteAddSuite(suite, testSuite_apx_fileMap());
   CuSuiteAddSuite(suite, testSuite_apx_nodeData());
   CuSuiteAddSuite(suite, testSuite_apx_allocator());
   CuSuiteAddSuite(suite, testSuite_remotefile());

   CuSuiteAddSuite(suite, testsuite_apx_attributesParser());


   CuSuiteAddSuite(suite, testSuite_apx_serverSocketConnection());
   //CuSuiteAddSuite(suite, testSuite_apx_server());
   //CuSuiteAddSuite(suite, testSuite_apx_clientSession());
   //CuSuiteAddSuite(suite, testSuite_apx_sessionCmd());

   CuSuiteAddSuite(suite, testSuite_apx_fileManagerShared());
   CuSuiteAddSuite(suite, testSuite_apx_fileManagerRemote());
   CuSuiteAddSuite(suite, testSuite_apx_fileManagerLocal());
   CuSuiteAddSuite(suite, testSuite_apx_fileManager());

   CuSuiteAddSuite(suite, testSuite_apx_nodeDataManager());

   CuSuiteAddSuite(suite, testSuite_apx_bytePortMap());
   CuSuiteAddSuite(suite, testSuite_apx_eventLoop());
   CuSuiteAddSuite(suite, testSuite_apx_portTriggerList());

   CuSuiteAddSuite(suite, testSuite_apx_clientSocketConnection());
   //CuSuiteAddSuite(suite, testSuite_apx_port_routing());
   CuSuiteAddSuite(suite, testSuite_apx_compiler());
   CuSuiteAddSuite(suite, testSuite_apx_nodeInfo());







   CuSuiteRun(suite);
   CuSuiteSummary(suite, output);
   CuSuiteDetails(suite, output);
   printf("%s\n", output->buffer);
   CuSuiteDelete(suite);
   CuStringDelete(output);
   
}

int main(void)
{
   RunAllTests();
   return 0;
}
