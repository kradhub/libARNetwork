/**
 *	@file deportedData.h
 *  @brief deported data used to send large data or data with scalable size
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_DEPORTEDDATA_H_
#define _NETWORK_DEPORTEDDATA_H_

#include <libNetwork/error.h>

/*****************************************
 * 
 * 			deportedData header:
 *
******************************************/

/**
 *  @brief status sent by callback.
**/
typedef enum
{
	NETWORK_DEPORTEDDATA_CALLBACK_SENT = 0, /**< data sent  */
    NETWORK_DEPORTEDDATA_CALLBACK_SENT_WITH_ACK, /**< data acknowledged sent  */
    NETWORK_DEPORTEDDATA_CALLBACK_TIMEOUT, /**< timeout occurred, data not received */
    NETWORK_DEPORTEDDATA_CALLBACK_FREE /**< free the data not sent*/
	
} eNETWORK_DEPORTEDDATA_Callback_Status;

/**
 *  @brief deported data used to send large data or data with scalable size
**/
typedef struct network_DeportedData_t  
{
    void* pData; /**< Pointer on the data */
    int dataSize; /**< size of the data */
    int (*callBack)(int IoBuffer , void* pData, int status); /**
                                         *<     @brief call back use when the data are sent or have a timeout 
                                         *      @param[in] IoBuffer identifier of the IoBuffer is calling back
                                         *      @param[in] pointer on the data
                                         *      @param[in] status indicating the reason of the callback. eNETWORK_DEPORTEDDATA_Callback_Status type
                                         *      @return equal 1  
                                         *      @see eNETWORK_DEPORTEDDATA_Callback_Status
                                        **/
}network_DeportedData_t;

#endif // _NETWORK_DEPORTEDDATA_H_

