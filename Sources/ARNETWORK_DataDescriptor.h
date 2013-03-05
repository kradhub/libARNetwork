/**
 *  @file ARNETWORK_DataDescriptor.h
 *  @brief ARNETWORK_DataDescriptor_t used by ARNETWORK_IOBuffer_t
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_DATADESCRIPTOR_PRIVATE_H_
#define _ARNETWORK_DATADESCRIPTOR_PRIVATE_H_

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_Manager.h>

/*****************************************
 * 
 *             define:
 *
******************************************/

/*****************************************
 * 
 *             header:
 *
******************************************/

/**
 *  @brief data sent by the network manager
**/
typedef struct  
{
    uint8_t *dataPtr; /**< pointer on the data to send*/
    int dataSize; /**< size of the data */
    void *customData; /**< custom data */
    ARNETWORK_Manager_Callback_t callback; /**< call back use when the data are sent or timeout occurred */
    int isUsingDataCopy; /**< Indicator of using copy of data */
    
}ARNETWORK_DataDescriptor_t;

#endif /** _ARNETWORK_DATADESCRIPTOR_PRIVATE_H_ */
