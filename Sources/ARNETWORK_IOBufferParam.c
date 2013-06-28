/**
 *  @file ARNETWORK_IOBufferParam.c
 *  @brief prameters used to set the parameters of a new IOBuffer
 *  @date 28/09/2012
 *  @author maxime.maitre@parrot.com
 **/

/*****************************************
 *
 *             include file :
 *
 ******************************************/

#include <stdlib.h>

#include <libARSAL/ARSAL_Print.h>

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_IOBufferParam.h>

/*****************************************
 *
 *             define :
 *
 ******************************************/

#define ARNETWORK_IOBUFFER_PARAM_TAG "ARNETWORK_IOBufferParam"

#define ARNETWORK_IOBUFFER_ID_DEFAULT -1
#define ARNETWORK_IOBUFFER_ID_MIN 2
#define ARNETWORK_IOBUFFER_ID_MAX 127
#define ARNETWORK_IOBUFFER_DATA_TYPE_DEFAULT ARNETWORKAL_FRAME_TYPE_UNINITIALIZED
#define ARNETWORK_IOBUFFER_SENDING_WAIT_TIME_DEFAULT 1
#define ARNETWORK_IOBUFFER_ACK_TIMEOUT_MS_DEFAULT ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER
#define ARNETWORK_IOBUFFER_NUMBER_OF_RETRY_DEFAULT ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER
#define ARNETWORK_IOBUFFER_NUMBER_OF_CELL_DEFAULT 0
#define ARNETWORK_IOBUFFER_MAX_SIZE_OF_DATA_COPY_DEFAULT 0
#define ARNETWORK_IOBUFFER_OVERWRITING_DEFAULT 0

/*****************************************
 *
 *             implementation :
 *
 ******************************************/

eARNETWORK_ERROR ARNETWORK_IOBufferParam_DefaultInit (ARNETWORK_IOBufferParam_t *IOBufferParamPtr)
{
    /** -- initialization of the IOBufferParam with default parameters -- */

    /** local declarations */
    eARNETWORK_ERROR error = ARNETWORK_OK;

    if (IOBufferParamPtr != NULL)
    {
        IOBufferParamPtr->ID = ARNETWORK_IOBUFFER_ID_DEFAULT;
        IOBufferParamPtr->dataType = ARNETWORK_IOBUFFER_DATA_TYPE_DEFAULT;
        IOBufferParamPtr->sendingWaitTimeMs = ARNETWORK_IOBUFFER_SENDING_WAIT_TIME_DEFAULT;
        IOBufferParamPtr->ackTimeoutMs = ARNETWORK_IOBUFFER_ACK_TIMEOUT_MS_DEFAULT;
        IOBufferParamPtr->numberOfRetry = ARNETWORK_IOBUFFER_NUMBER_OF_RETRY_DEFAULT;

        IOBufferParamPtr->numberOfCell = ARNETWORK_IOBUFFER_NUMBER_OF_CELL_DEFAULT;
        IOBufferParamPtr->dataCopyMaxSize = ARNETWORK_IOBUFFER_MAX_SIZE_OF_DATA_COPY_DEFAULT;
        IOBufferParamPtr->isOverwriting = ARNETWORK_IOBUFFER_OVERWRITING_DEFAULT;
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }

    return error;
}

int ARNETWORK_IOBufferParam_Check (const ARNETWORK_IOBufferParam_t *IOBufferParamPtr)
{
    /** -- check the values of the IOBufferParam -- */

    /** local declarations */
    int ok = 0;

    /** check the parameters values */
    if ((IOBufferParamPtr != NULL) &&
        (IOBufferParamPtr->ID >= ARNETWORK_IOBUFFER_ID_MIN) &&
        (IOBufferParamPtr->ID <= ARNETWORK_IOBUFFER_ID_MAX) &&
        (IOBufferParamPtr->dataType != ARNETWORKAL_FRAME_TYPE_UNINITIALIZED) &&
        (IOBufferParamPtr->sendingWaitTimeMs >= 0) &&
        (IOBufferParamPtr->ackTimeoutMs >= -1) &&
        (IOBufferParamPtr->numberOfRetry >= -1))
    {
        ok = 1;
    }
    else
    {
        ARSAL_PRINT (ARSAL_PRINT_ERROR, ARNETWORK_IOBUFFER_PARAM_TAG," parameters for new IOBuffer are not correct. \n\
values expected: \n\
    - %d <= ID <= %d (value set: %d)\n\
    - dataType != %d (value set: %d)\n\
    - sendingWaitTimeMs >= 0 (value set: %d)\n\
    - ackTimeoutMs > 0 or -1 if not used (value set: %d)\n\
    - numberOfRetry > 0 or -1 if not used  (value set: %d)\n\
    - numberOfCell > 0 (value set: %d)\n\
    - dataCopyMaxSize >= 0 (value set: %d)\n\
    - isOverwriting = 0 or 1 (value set: %d)",
                     ARNETWORK_IOBUFFER_ID_MIN, ARNETWORK_IOBUFFER_ID_MAX, IOBufferParamPtr->ID,
                     ARNETWORKAL_FRAME_TYPE_UNINITIALIZED, IOBufferParamPtr->dataType,
                     IOBufferParamPtr->sendingWaitTimeMs,
                     IOBufferParamPtr->ackTimeoutMs,
                     IOBufferParamPtr->numberOfRetry,
                     IOBufferParamPtr->numberOfCell,
                     IOBufferParamPtr->dataCopyMaxSize,
                     IOBufferParamPtr->isOverwriting);
    }

    return ok;
}
