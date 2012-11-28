/**
 *	@file manager.h
 *  @brief network manager allow to send data acknowledged or not.
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _PRIVATE_NETWORK_MANGER_H_
#define _PRIVATE_NETWORK_MANGER_H_

#include <libNetwork/manager.h>

/*****************************************
 * 
 * 			private header:
 *
******************************************/

/**
 *  @brief create manager's IoBuffers.
 *  @warning only call by NETWORK_NewManager()
 *  @pre pManager->ppTabOutput and pManager->ppTabInput must be allocated and set to NULL.
 *  @pre pManager->numOfOutputWithoutAck, pManager->numOfOutput, pManager->numOfInputWithoutAck and pManager->numOfInput must be accurate
 *  @param pManager pointer on the Manager
 *  @param[in] ptabParamInput Table of the parameters of creation of the inputs. The table must contain as many parameters as the number of input buffer.
 * 	@param[in] ptabParamOutput Table of the parameters of creation of the outputs. The table must contain as many parameters as the number of output buffer.
 *  @return error equal to NETWORK_OK if the IoBuffer are correctly created otherwise see eNETWORK_Manager_Error.
 *  @see NETWORK_NewManager()
**/
int NETWORK_ManagerCreateIoBuffer(network_manager_t* pManager,
                                    network_paramNewIoBuffer_t* ptabParamInput, 
                                    network_paramNewIoBuffer_t* ptabParamOutput);

#endif // _PRIVATE_NETWORK_MANGER_H_

