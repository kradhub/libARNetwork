/**
 *  @file ARNETWORK_DeportedData.h
 *  @brief deported data used to send large data or data with scalable size
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_DEPORTEDDATA_H_
#define _ARNETWORK_DEPORTEDDATA_H_

#include <libARNetwork/ARNETWORK_Error.h>

/*****************************************
 * 
 *             variableSizeData header:
 *
******************************************/

/**
 *  @brief callback use when the data are sent or have a timeout 
 *  @param[in] IOBufferId identifier of the IoBuffer is calling back
 *  @param[in] dataPtr pointer on the data
 *  @param[in] customData custom data
 *  @param[in] status indicating the reason of the callback. eARNETWORK_MANAGER_CALLBACK_STATUS type
 *  @return what do in timeout case
 *  @see eARNETWORK_MANAGER_CALLBACK_STATUS
**/
typedef eARNETWORK_MANAGER_CALLBACK_RETURN (*ARNETWORK_Manger_Callback_t)(int IOBufferId, uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status);

#endif /** _ARNETWORK_DEPORTEDDATA_H_ */
