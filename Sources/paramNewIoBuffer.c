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

#include <libNetwork/status.h>

#include <libNetwork/paramNewIoBuffer.h>


/*****************************************
 * 
 * 			define :
 *
******************************************/

#define NETWORK_IOBUFFER_ID_DEFAULT NETWORK_IOBUFFER_INFINITE_NUMBER
#define NETWORK_IOBUFFER_DATATYPE_DEFAULT NETWORK_FRAME_TYPE_UNINITIALIZED
#define NETWORK_IOBUFFER_SENDING_WAIT_TIME_DEFAULT 1
#define NETWORK_IOBUFFER_ACKTILEOUTMS_DEFAULT -1
#define NETWORK_IOBUFFER_NBOFRETRY_DEFAULT -1
#define NETWORK_IOBUFFER_NUMBER_OF_CELL_DEFAULT 0
#define NETWORK_IOBUFFER_CELL_SIZE_DEFAULT 0
#define NETWORK_IOBUFFER_OVERWRITING_DEFAULT 0
#define NETWORK_IOBUFFER_deportedData_DEFAULT 0

/*****************************************
 * 
 * 			implementation :
 *
******************************************/

eNETWORK_Error NETWORK_ParamNewIoBufferDefaultInit(network_paramNewIoBuffer_t *pParam)
{
	/** -- initialization of the paramNewIoBuffer with default parameters -- */
    
    /** local declarations */
    eNETWORK_Error error = NETWORK_OK;
    
    if(pParam != NULL)
    {
        pParam->id = NETWORK_IOBUFFER_ID_DEFAULT;
        pParam->dataType = NETWORK_IOBUFFER_DATATYPE_DEFAULT;	
        pParam->sendingWaitTimeMs = NETWORK_IOBUFFER_SENDING_WAIT_TIME_DEFAULT;
        pParam->ackTimeoutMs = NETWORK_IOBUFFER_ACKTILEOUTMS_DEFAULT;
        pParam->nbOfRetry = NETWORK_IOBUFFER_NBOFRETRY_DEFAULT;
        
        pParam->numberOfCell = NETWORK_IOBUFFER_NUMBER_OF_CELL_DEFAULT;	
        pParam->cellSize = NETWORK_IOBUFFER_CELL_SIZE_DEFAULT;
        pParam->isOverwriting = NETWORK_IOBUFFER_OVERWRITING_DEFAULT;
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
        pParam->dataType != NETWORK_FRAME_TYPE_UNINITIALIZED &&
        pParam->sendingWaitTimeMs > 0 &&
        pParam->ackTimeoutMs >= -1 &&
        pParam->nbOfRetry >= -1 &&
        pParam->numberOfCell > 0 &&
        pParam->cellSize > 0)
    {
        ok = 1;
    }
    else
    {
        SAL_PRINT(PRINT_ERROR," parameters for new IoBuffer are not correct. \n \
values expected: \n \
    - id > -1 (value set: %d)\n\
    - dataType != NETWORK_FRAME_TYPE_UNINITIALIZED (value set: %d)\n\
    - sendingWaitTimeMs > 0 (value set: %d)\n\
    - ackTimeoutMs > 0 or -1 if not used (value set: %d)\n\
    - nbOfRetry > 0 or -1 if not used  (value set: %d)\n\
    - numberOfCell > 0 (value set: %d)\n\
    - cellSize > 0 (value set: %d)\n\
    - isOverwriting = 0 or 1 (value set: %d)\n\
    - deportedData = 0 or 1 (value set: %d)\n", 
        pParam->id, 
        pParam->dataType, 
        pParam->sendingWaitTimeMs,
        pParam->ackTimeoutMs,
        pParam->nbOfRetry,
        pParam->numberOfCell,
        pParam->cellSize,
        pParam->isOverwriting,
        pParam->deportedData);
    }
    
   return ok; 
}
