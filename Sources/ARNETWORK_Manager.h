/**
 * @file ARNETWORK_Manager.h
 * @brief network manager allow to send data acknowledged or not.
 * @date 05/18/2012
 * @author maxime.maitre@parrot.com
 */

#ifndef _NETWORK_MANAGER_PRIVATE_H_
#define _NETWORK_MANAGER_PRIVATE_H_

#include "ARNETWORK_IOBuffer.h"
#include "ARNETWORK_Sender.h"
#include "ARNETWORK_Receiver.h"
#include <libARNetwork/ARNETWORK_Manager.h>
#include <libARNetworkAL/ARNETWORKAL_Manager.h>

typedef enum {
    ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PING = 0, /**< Ping buffer id - ping requests */
    ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_PONG, /**< Pong buffer id - ping reply */
    ARNETWORK_MANAGER_INTERNAL_BUFFER_ID_MAX, /**< Should always be kept less or equal to 10 */
} eARNETWORK_MANAGER_INTERNAL_BUFFER_ID;

/**
 * @brief get the identifier of the output buffer storing the acknowledgment for an output buffer storing data acknowledged.
 * @param[in] ID identifier of the output buffer waiting an acknowledgment.
 * @return identifier of the output buffer storing the acknowledgment.
 */
static inline int ARNETWORK_Manager_IDOutputToIDAck (ARNETWORKAL_Manager_t *alManager, int identifier)
{
    return identifier + (alManager->maxIds / 2);
}

/**
 * @brief get the identifier of the output buffer storing data acknowledged for an output buffer storing acknowledgments.
 * @param[in] ID identifier of the output buffer storing the acknowledgment.
 * @return identifier of the output buffer waiting an acknowledgment.
 */
static inline int ARNETWORK_Manager_IDAckToIDInput (ARNETWORKAL_Manager_t *alManager, int identifier)
{
    return identifier - (alManager->maxIds / 2);
}

/**
 * @brief network manager allow to send data acknowledged or not.
 */
struct ARNETWORK_Manager_t
{
    ARNETWORKAL_Manager_t *networkALManager; /**< Pointer on the OS specific manager */
    ARNETWORK_Sender_t *senderPtr; /**< Pointer on the sender */
    ARNETWORK_Receiver_t *receiverPtr; /**< Pointer on the receiver */
    ARNETWORK_IOBuffer_t **inputBufferPtrArr; /**< Address of the array storing the input buffer */
    ARNETWORK_IOBuffer_t **outputBufferPtrArr; /**< Address of the array storing the output buffer */
    ARNETWORK_IOBuffer_t **internalInputBufferPtrArr; /**< Address of the array storing the internal input buffers */
    int numberOfInput; /**< Number of input buffer */
    int numberOfOutput; /**< Number of output buffer */
    int numberOfInputWithoutAck; /**< Number of input buffer without the  buffers of acknowledgement */
    int numberOfOutputWithoutAck; /**< Number of output buffer without the  buffers of acknowledgement */
    int numberOfInternalInputs; /**< Number of internal input buffers */
    ARNETWORK_IOBuffer_t **inputBufferPtrMap; /**< array storing the inputBuffers by their identifier */
    ARNETWORK_IOBuffer_t **outputBufferPtrMap; /**< array storing the outputBuffers by their identifier */
};

#endif /** _NETWORK_MANAGER_PRIVATE_H_ */
