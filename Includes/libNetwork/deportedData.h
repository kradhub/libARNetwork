/**
 *  @file deportedData.h
 *  @brief deported data used to send large data or data with scalable size
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_DEPORTEDDATA_H_
#define _NETWORK_DEPORTEDDATA_H_

#include <libNetwork/status.h>

/*****************************************
 * 
 *             deportedData header:
 *
******************************************/

/**
 *  @brief call back use when the data are sent or have a timeout 
 *  @param[in] IoBufferId identifier of the IoBuffer is calling back
 *  @param[in] pointer on the data
 *  @param[in] status indicating the reason of the callback. eNETWORK_CALLBACK_STATUS type
 *  @return 1 to reset the retry counter or 0 to stop the InputBuffer 
 *  @see eNETWORK_CALLBACK_STATUS
**/
typedef int (*network_deportDatacallback)(int IoBufferId, void* pData, int status);

/**
 *  @brief deported data used to send large data or data with scalable size
**/
typedef struct network_DeportedData_t  
{
    void* pData; /**< Pointer on the data */
    int dataSize; /**< size of the data */
    network_deportDatacallback callback; /**< call back use when the data are sent or have a timeout */
                                      
}network_DeportedData_t;

#endif // _NETWORK_DEPORTEDDATA_H_

