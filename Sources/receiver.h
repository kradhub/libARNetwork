/**
 *	@file receiver.h
 *  @brief manage the data received, used by libNetwork/network
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _PRIVATE_NETWORK_RECEIVER_H_
#define _PRIVATE_NETWORK_RECEIVER_H_

#include <libNetwork/receiver.h>

/*****************************************
 * 
 * 			private header:
 *
******************************************/

/**
 *  @brief get the next Frame of the receiving buffer
 * 	@param pReceiver the pointer on the receiver
 * 	@param prevFrame[in] pointer on the previous frame
 * 	@return nextFrame pointer on the next frame
 *	@pre only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
network_frame_t* NETWORK_ReceiverGetNextFrame( network_receiver_t* pReceiver,
                                                 network_frame_t* prevFrame );

/**
 *  @brief copy the data received to the output buffer
 * 	@param pReceiver the pointer on the receiver
 * 	@param pOutBuffer[in] pointer on the output buffer
 *  @param pFrame[in] pointer on the frame received
 * 	@return error equal to NETWORK_OK if the Bind if successful otherwise equal to error of eNETWORK_Error.
 *	@pre only call by NETWORK_RunSendingThread()
 * 	@see NETWORK_RunSendingThread()
**/
int NETWORK_ReceiverCopyDataRecv( network_receiver_t* pReceiver,
                                   network_ioBuffer_t* pOutBuffer,
                                   network_frame_t* pFrame );
                                   
/**
 *  @brief call back use to free deported data 
 *  @param[in] OutBufferId IoBuffer identifier of the IoBuffer is calling back
 *  @param[in] pData pointer on the data
 *  @param[in] status status indicating the reason of the callback. eNETWORK_DEPORTEDDATA_Callback_Status type
 *  @return   
 *  @see eNETWORK_DEPORTEDDATA_Callback_Status
**/
int NETWORK_freedeportedData(int OutBufferId, void* pData, int status);

#endif // _PRIVATE_NETWORK_RECEIVER_H_

