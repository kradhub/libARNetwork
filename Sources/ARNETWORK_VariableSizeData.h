/**
 *  @file ARNETWORK_VariableSizeData.h
 *  @brief ARNETWORK_VariableSizeData_t used by ARNETWORK_IOBuffer_t
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_VARIABLESIZEDATA_PRIVATE_H_
#define _ARNETWORK_VARIABLESIZEDATA_PRIVATE_H_

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_Manager.h>

/*****************************************
 * 
 *             header:
 *
******************************************/

/**
 *  @brief Variable Size Data used to send large data or variable size data
**/
typedef struct  
{
    uint8_t *dataPtr; /**< Pointer on the data */
    int dataSize; /**< size of the data */
    void *customData; /**< custom data */
    ARNETWORK_Manger_Callback_t callback; /**< call back use when the data are sent or have a timeout */
    
}ARNETWORK_VariableSizeData_t;

#endif /** _ARNETWORK_VARIABLESIZEDATA_PRIVATE_H_ */
