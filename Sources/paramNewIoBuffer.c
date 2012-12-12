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

#include <libNetwork/error.h>

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

int NETWORK_ParamNewIoBufferDefaultInit(network_paramNewIoBuffer_t *pParam)
{
	/** -- initialization of the paramNewIoBuffer with default parameters -- */
    
    /** local declarations */
    int error = NETWORK_OK;
    
    if(pParam != NULL)
    {
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
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}

int NETWORK_ParamNewIoBufferCheck( const network_paramNewIoBuffer_t* pParam )
{
    /** -- check the values of the paramNewIoBuffer -- */
    
    /** local declarations */
    int ok = 0;
    
    /** check the parameters values */
    if( pParam != NULL &&
        pParam->id > NETWORK_IOBUFFER_ID_DEFAULT && 
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
        SAL_PRINT(PRINT_ERROR," parameters for new IoBuffer are not correct. \n \
values expected: \n \
    - id > -1 (value set: %d)\n\
    - dataType != network_frame_t_TYPE_UNINITIALIZED (value set: %d)\n\
    - sendingWaitTime > 0 (value set: %d)\n\
    - ackTimeoutMs > 0 or -1 if not used (value set: %d)\n\
    - nbOfRetry > 0 or -1 if not used  (value set: %d)\n\
    - buffSize > 0 (value set: %d)\n\
    - buffCellSize > 0 (value set: %d)\n\
    - overwriting = 0 or 1 (value set: %d)\n\
    - deportedData = 0 or 1 (value set: %d)\n", 
    pParam->id, 
    pParam->dataType, 
    pParam->sendingWaitTime,
    pParam->ackTimeoutMs,
    pParam->nbOfRetry,
    pParam->buffSize,
    pParam->buffCellSize,
    pParam->overwriting,
    pParam->deportedData);
    }
    
   return ok; 
}
