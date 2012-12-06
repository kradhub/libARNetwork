/**
 *	@file paramNewIoBuffer.c
 *  @brief prameters used to set the parameters of a new inOutBuffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 * 			include file :
 *
******************************************/

#include <stdlib.h>
#include <libSAL/print.h>

#include <libNetwork/paramNewIoBuffer.h>


/*****************************************
 * 
 * 			define :
 *
******************************************/

#define NETWORK_IOBUFFER_ID_DEFAULT -1
#define NETWORK_IOBUFFER_DATATYPE_DEFAULT network_frame_t_TYPE_UNINITIALIZED
#define NETWORK_IOBUFFER_SENDINGWAITTIME_DEFAULT 1
#define NETWORK_IOBUFFER_ACKTILEOUTMS_DEFAULT -1
#define NETWORK_IOBUFFER_NBOFRETRY_DEFAULT -1
#define NETWORK_IOBUFFER_BUFFSIZE_DEFAULT 0
#define NETWORK_IOBUFFER_BUFFCELLSIZE_DEFAULT 0
#define NETWORK_IOBUFFER_OVERWRITING_DEFAULT 0
#define NETWORK_IOBUFFER_deportedData_DEFAULT 0

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

void NETWORK_ParamNewIoBufferDefaultInit(network_paramNewIoBuffer_t *pParam)
{
	/** -- initialization of the paramNewIoBuffer with default parameters -- */
	
	pParam->id = NETWORK_IOBUFFER_ID_DEFAULT;
    pParam->dataType = NETWORK_IOBUFFER_DATATYPE_DEFAULT;	
    pParam->sendingWaitTime = NETWORK_IOBUFFER_SENDINGWAITTIME_DEFAULT;
    pParam->ackTimeoutMs = NETWORK_IOBUFFER_ACKTILEOUTMS_DEFAULT;
    pParam->nbOfRetry = NETWORK_IOBUFFER_NBOFRETRY_DEFAULT;
    
    pParam->buffSize = NETWORK_IOBUFFER_BUFFSIZE_DEFAULT;	
    pParam->buffCellSize = NETWORK_IOBUFFER_BUFFCELLSIZE_DEFAULT;
    pParam->overwriting = NETWORK_IOBUFFER_OVERWRITING_DEFAULT;
    pParam->deportedData = NETWORK_IOBUFFER_deportedData_DEFAULT;
}

int NETWORK_ParamNewIoBufferCheck( const network_paramNewIoBuffer_t* pParam )
{
    /** -- check the values of the paramNewIoBuffer -- */
    
    /** local declarations */
    int ok = 0;
    
    if( pParam->id > NETWORK_IOBUFFER_ID_DEFAULT && 
        pParam->dataType != network_frame_t_TYPE_UNINITIALIZED &&
        pParam->sendingWaitTime > 0 &&
        pParam->ackTimeoutMs >= -1 &&
        pParam->nbOfRetry >= -1 &&
        pParam->buffSize > 0 &&
        pParam->buffCellSize > 0)
    {
        ok = 1;
    }
    else
    {
        sal_print(PRINT_ERROR," parameters for new IoBuffer are not correct. \n \
values expected: \n \
    - id > -1 \n \
    - dataType != network_frame_t_TYPE_UNINITIALIZED \n \
    - sendingWaitTime > 0 \n \
    - ackTimeoutMs > 0 or -1 if not used \n \
    - nbOfRetry > 0 or -1 if not used  \n \
    - buffSize > 0 \n \
    - buffCellSize > 0 \n \
    - overwriting = 0 or 1 \n");
    }
    
   return ok; 
}
