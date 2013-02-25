/**
 *  @file ARNETWORK_IOBufferParam.h
 *  @brief prameters used to set the parameters of a new IOBuffer
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _ARNETWORK_IOBUFFERPARAM_H_
#define _ARNETWORK_IOBUFFERPARAM_H_

#include <libARNetwork/ARNETWORK_Frame.h>
#include <libARNetwork/ARNETWORK_Error.h>

/*****************************************
 * 
 *             define :
 *
******************************************/

#define ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER -1

/*****************************************
 * 
 *             IOBufferParam header:
 *
******************************************/

/**
 *  @brief used to set the parameters of a new In Out Buffer
**/
typedef struct  
{
    int ID; /**< Identifier used to find the IOBuffer in a list*/
    eARNETWORK_FRAME_TYPE dataType; /**< Type of the data stored in the buffer*/
    int sendingWaitTimeMs; /**< Time in millisecond between 2 send when the IOBuffer is used with a ARNetwork_Sender */
    int ackTimeoutMs; /**< Timeout in millisecond before retry to send the data waiting an acknowledgement when the InOutBuffer is used with a ARNetwork_Sender*/
    int numberOfRetry; /**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a ARNetwork_Sender*/
    
    unsigned int numberOfCell; /**< Maximum number of data stored*/
    unsigned int cellSize; /**< Size of one data in byte*/
    int isOverwriting; /**< Indicator of overwriting possibility (1 = true | 0 = false)*/
    int isUsingVariableSizeData; /**< Indicator of using variable size data */

}ARNETWORK_IOBufferParam_t;

/**
 *  @brief initialization of the IOBufferParam with default parameters
 *  @pre before to use the IOBufferParam the paramaters useful must be set.
 *  @param[in,out] IOBufferParamPtr Pointer on the parameters for the new input or output buffer
 *  @return error of eARNETWORK_ERROR type
**/
eARNETWORK_ERROR ARNETWORK_IOBufferParam_DefaultInit(ARNETWORK_IOBufferParam_t *IOBufferParamPtr); 

/**
 *  @brief check the values of the IOBufferParam
 *  @param[in] IOBufferParamPtr Pointer on the parameters for the new input or output buffer
 *  @return 1 if the IOBufferParam is usable for create a new ioBuffer else 0
**/
int ARNETWORK_IOBufferParam_Check( const ARNETWORK_IOBufferParam_t *IOBufferParamPtr );

#endif /** _ARNETWORK_IOBUFFERPARAM_H_ */

