/**
 *	@file main.h
 *  @brief libNetWork TestBench automatic
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdio.h> 
#include <stdlib.h> 
#include <libSAL/print.h>
#include <libSAL/thread.h>

#include <string.h>

#include <libNetwork/frame.h>
#include <libNetwork/manager.h>
#include <libNetwork/paramNewIoBuffer.h>

#include <unistd.h>

/*****************************************
 * 
 * 			define :
 *
******************************************/

#define NUMBER_DATA_SENT 5
#define SENDING_SLEEP_TIME_US 50000
#define RECEIVER_TIMEOUT_SEC 5
#define FIRST_CHAR_SENT 'A'
#define FIRST_INT_ACK_SENT 100
#define BASE_DEPORTED_DATA 0xffffffffffffffffLL

#define RECV_TIMEOUT_MS 10
#define PORT1 5551
#define PORT2 5552
#define ADRR_IP "127.0.0.1"

#define SEND_BUFF_SIZE 256
#define RECV_BUFF_SIZE 256

#define NB_OF_INPUT_NET1 3
#define NB_OF_OUTPUT_NET1 1
#define NB_OF_INPUT_NET2 1
#define NB_OF_OUTPUT_NET2 3

#define SENDDATA 1
#define SENDDATADEPORT 1

/** define of the ioBuffer identifiers */
typedef enum eID_BUFF
{
	ID_CHAR_DATA = 5,
	ID_INT_DATA_WITH_ACK,
	ID_INT_DATA,
    ID_DEPORT_DATA
}eID_BUFF;

int callbackDepotData(int OutBufferId, void* pData, int status);

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

int main(int argc, char *argv[])
{
	/** local declarations */
	network_manager_t* pManager1= NULL;
	network_manager_t* pManager2= NULL;
    sal_thread_t thread_send1 = NULL;
	sal_thread_t thread_recv1 = NULL;
	sal_thread_t thread_send2 = NULL;
	sal_thread_t thread_recv2 = NULL;
	int ii = 0;
	int error = 0;

#if SENDDATA
	char chData = 0;
	int intData = 0;
#endif
#if SENDDATADEPORT
    void* pDataDeported = NULL;
    uint64_t orgDataDeported = BASE_DEPORTED_DATA;
    uint64_t dataDeportedRead = 0;
    int dataDeportSize = 0 ;
#endif
	
	network_paramNewIoBuffer_t paramInputNetwork1[NB_OF_INPUT_NET1];
	network_paramNewIoBuffer_t paramOutputNetwork1[NB_OF_OUTPUT_NET1];
	
	network_paramNewIoBuffer_t paramInputNetwork2[NB_OF_INPUT_NET2];
	network_paramNewIoBuffer_t paramOutputNetwork2[NB_OF_OUTPUT_NET2];
	
    /** initialization of the buffer parameters */
	/** --- network 1 --- */
	
	/** input ID_CHAR_DATA char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramInputNetwork1[0]) );
	paramInputNetwork1[0].id = ID_CHAR_DATA;
	paramInputNetwork1[0].dataType = network_frame_t_TYPE_DATA;
	paramInputNetwork1[0].buffSize = 1;
	paramInputNetwork1[0].buffCellSize = sizeof(char);
	paramInputNetwork1[0].overwriting = 1;
	
	/** input ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramInputNetwork1[1]) );
	paramInputNetwork1[1].id = ID_INT_DATA_WITH_ACK;
	paramInputNetwork1[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramInputNetwork1[1].sendingWaitTime = 2;
	paramInputNetwork1[1].ackTimeoutMs = 5;
	paramInputNetwork1[1].nbOfRetry = -1/*20*/;
	paramInputNetwork1[1].buffSize = 5;
	paramInputNetwork1[1].buffCellSize = sizeof(int);
    
    /** input ID_DEPORT_DATA int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramInputNetwork1[2]) );
	paramInputNetwork1[2].id = ID_DEPORT_DATA;
	paramInputNetwork1[2].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramInputNetwork1[2].sendingWaitTime = 2;
	paramInputNetwork1[2].ackTimeoutMs = 5;
	paramInputNetwork1[2].nbOfRetry = -1/*20*/;
	paramInputNetwork1[2].buffSize = 5;
	paramInputNetwork1[2].deportedData = 1;
	
	/** output ID_INT_DATA int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramOutputNetwork1[0]) );
	paramOutputNetwork1[0].id = ID_INT_DATA;
	paramOutputNetwork1[0].dataType = network_frame_t_TYPE_DATA;
	paramOutputNetwork1[0].buffSize = 5;
	paramOutputNetwork1[0].buffCellSize = sizeof(int);
	paramOutputNetwork1[0].overwriting = 1;
	
	/** ----------------------------- */
	
	/**--- network 2 --- */
	
	/** input ID_INT_DATA char */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramInputNetwork2[0]) );
	paramInputNetwork2[0].id = ID_INT_DATA;
	paramInputNetwork2[0].dataType = network_frame_t_TYPE_DATA;
	paramInputNetwork2[0].sendingWaitTime = 2;
	paramInputNetwork2[0].buffSize = 2;
	paramInputNetwork2[0].buffCellSize = sizeof(int);
	paramInputNetwork2[0].overwriting = 1;
	
	/**  output ID_CHAR_DATA int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramOutputNetwork2[0]) );
	paramOutputNetwork2[0].id = ID_CHAR_DATA;
	paramOutputNetwork2[0].dataType = network_frame_t_TYPE_DATA;
	paramOutputNetwork2[0].sendingWaitTime = 3;
	paramOutputNetwork2[0].buffSize = 1;
	paramOutputNetwork2[0].buffCellSize = sizeof(char);
	paramOutputNetwork2[0].overwriting = 1;
	
	/** output ID_INT_DATA_WITH_ACK int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramOutputNetwork2[1]) );
	paramOutputNetwork2[1].id = ID_INT_DATA_WITH_ACK;
    paramOutputNetwork2[1].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramOutputNetwork2[1].buffSize = 5;
	paramOutputNetwork2[1].buffCellSize = sizeof(int);
    
    /** output ID_DEPORT_DATA int */
    NETWORK_ParamNewIoBufferDefaultInit( &(paramOutputNetwork2[2]) );
	paramOutputNetwork2[2].id = ID_DEPORT_DATA;
    paramOutputNetwork2[2].dataType = network_frame_t_TYPE_DATA_WITH_ACK;
	paramOutputNetwork2[2].buffSize = 5;
    paramOutputNetwork2[2].deportedData = 1;
	
	/** ----------------------------- */
	
    printf(" -- libNetWork TestBench auto -- \n");
    
    /** create the Manger1 */
    
	pManager1 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE,
                                    NB_OF_INPUT_NET1, paramInputNetwork1,
                                    NB_OF_OUTPUT_NET1, paramOutputNetwork1);
    
    error = NETWORK_ManagerSocketsInit(pManager1, ADRR_IP, PORT1, PORT2, RECEIVER_TIMEOUT_SEC);
    printf("pManager1 error initScoket = %d \n", error);

    /** create the Manger2 */
	pManager2 = NETWORK_NewManager( RECV_BUFF_SIZE, SEND_BUFF_SIZE,
                                    NB_OF_INPUT_NET2, paramInputNetwork2,
                                    NB_OF_OUTPUT_NET2, paramOutputNetwork2);          
    
    error = NETWORK_ManagerSocketsInit(pManager2, ADRR_IP, PORT2, PORT1, RECEIVER_TIMEOUT_SEC);
    printf("pManager2 error initScoket = %d \n ", error);
	
	printf("main start \n");
	
    /** create the threads */
	sal_thread_create(&thread_recv2, (sal_thread_routine) NETWORK_ManagerRunReceivingThread, pManager2);
	sal_thread_create(&thread_recv1, (sal_thread_routine) NETWORK_ManagerRunReceivingThread, pManager1);
	
	sal_thread_create(&thread_send1, (sal_thread_routine) NETWORK_ManagerRunSendingThread, pManager1);
	sal_thread_create(&thread_send2, (sal_thread_routine) NETWORK_ManagerRunSendingThread, pManager2);
    

    /** loop sending data */
    for(ii = 0; ii < NUMBER_DATA_SENT; ii++)
    {
        
#if SENDDATA
		chData = FIRST_CHAR_SENT + ii;
		printf(" send char: %c \n",chData);
		error = NETWORK_ManagerSendData(pManager1, ID_CHAR_DATA, &chData);
		
		if( error )
		{
			printf(" error send char :%d \n", error);
		}

		intData = FIRST_INT_ACK_SENT + ii;
		printf(" send int: %d \n",intData);		
		error = NETWORK_ManagerSendData(pManager1, ID_INT_DATA_WITH_ACK, &intData);
		
		if( error  )
		{
			printf(" error send int ack :%d \n", error);
		}
#endif		
        
#if SENDDATADEPORT
      
        /** create DataDeported */
        dataDeportSize = ii+1;
        
        pDataDeported = malloc(dataDeportSize);
        memcpy(pDataDeported, &orgDataDeported, dataDeportSize);
        
        error = NETWORK_ManagerSendDeportedData( pManager1, ID_DEPORT_DATA,
                                                 pDataDeported, dataDeportSize,
                                                 &(callbackDepotData) );
                                                 
        printf(" send %d byte deportedData\n",dataDeportSize);
        if( error  )
		{
			printf(" error send deported data ack :%d \n", error);
		}
#endif
        
        usleep(SENDING_SLEEP_TIME_US);
    }
	
	printf(" -- stop-- \n");
	
	/** stop all therad */
    NETWORK_ManagerStop(pManager1);
	NETWORK_ManagerStop(pManager2);
    
    printf("wait ... \n");
	
	/** kill all threads */
	if(thread_send1 != NULL)
	{
		sal_thread_join(&(thread_send1), NULL);
	}
	if(thread_send2 != NULL)
	{
		sal_thread_join(&(thread_send2), NULL);
	}
	
	if(thread_recv1 != NULL)
	{
		sal_thread_join(&(thread_recv1), NULL);
	}
	
	if(thread_recv2 != NULL)
	{
		sal_thread_join(&(thread_recv2), NULL);
	}
    
    /** checking */
#if SENDDATA
    ii = 0;
	printf("\n the last char transmitted:\n");
    while( ! NETWORK_ManagerReadData(pManager2, ID_CHAR_DATA, &chData) )
    {
        ++ii;
        printf("- %c \n", chData);
        /** check values */
        error = error || ( chData != ( FIRST_CHAR_SENT + (NUMBER_DATA_SENT - 1) ) );
    }
    /** check nb data */
	error = error || ( ii != 1) ;
    
	printf("\n the integers transmitted:\n");
    ii = 0;
    while( ! NETWORK_ManagerReadData(pManager2, ID_INT_DATA_WITH_ACK, &intData) )
    {
        printf("- %d \n", intData);
        /** check values */
        error = error || ( intData != FIRST_INT_ACK_SENT + ii);
        ++ii;
    }
    /** check nb data */
    error = error || ( ii != 5) ;
#endif

#if SENDDATADEPORT

    printf("\n the deported data transmitted:\n");
    ii = 0;
    dataDeportedRead = 0;
    
    while( ! NETWORK_ManagerReaddeportedData(pManager2, ID_DEPORT_DATA, &dataDeportedRead, ii+1, NULL) )
    {
        dataDeportSize = ii+1;
        
        printf("- %08x %08x \n",  (uint32_t) (dataDeportedRead >> 32), (uint32_t) dataDeportedRead );
        
        /** create DataDeported */
        pDataDeported = malloc(dataDeportSize);
        memcpy(pDataDeported, &orgDataDeported, dataDeportSize);
        
        /** check values */
        error = error || memcmp(&dataDeportedRead, pDataDeported ,dataDeportSize);
        ++ii;
        free(pDataDeported);
        dataDeportedRead = 0;
    }
    /** check nb data */
    error = error || ( ii != 5) ;

#endif

	printf("\n");

    if(error)
    {
        printf("Bad result of the test bench \n");
    }
    else
    {
        printf("Good result of the test bench \n");
    }

    printf("\n");
	printf("end \n");
    
    /** delete */
    sal_thread_destroy( &thread_send1 );
    sal_thread_destroy( &thread_send2 );
    sal_thread_destroy( &thread_recv1 );
    sal_thread_destroy(&thread_recv2);
    NETWORK_DeleteManager( &pManager1 );
	NETWORK_DeleteManager( &pManager2 );

	return 0;
}

int callbackDepotData(int OutBufferId, void* pData, int status)
{
    /** local declarations */
    int error = 0;
    
    printf(" -- callbackDepotData -- \n");
    
    free(pData);
    
    return error;
}

