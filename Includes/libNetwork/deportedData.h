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
 *  @param[in] pData pointer on the data
 *  @param[in] pCustomData pointer on a custom data
 *  @param[in] status indicating the reason of the callback. eNETWORK_CALLBACK_STATUS type
 *  @return what do in timeout case
 *  @see eNETWORK_CALLBACK_STATUS
**/
typedef eNETWORK_CALLBACK_RETURN (*network_deportDatacallback)(int IoBufferId,
                                                                       uint8_t* pData, 
                                                                       void* customData,
                                                                       eNETWORK_CALLBACK_STATUS status);

/**
 *  @brief deported data used to send large data or data with scalable size
**/
typedef struct network_DeportedData_t  
{
    uint8_t* pData; /**< Pointer on the data */
    int dataSize; /**< size of the data */
    void* pCustomData; /**< Pointer on a custom data */
    network_deportDatacallback callback; /**< call back use when the data are sent or have a timeout */
                                      
}network_DeportedData_t;

#endif /** _NETWORK_DEPORTEDDATA_H_ */

