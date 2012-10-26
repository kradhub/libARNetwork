/**
 *	@file netWork.h
 *  @brief single buffer
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NET_WORK_H_
#define _NET_WORK_H_

// static :

//Enumerations :


//Structures :

/**
 *  @brief piloting command
**/
typedef struct netWork_t
{
    netWork_Sender_t* pSender;
    netWork_Receiver_t* pReceiver;
}netWork_t;


/**
 *  @brief Create a NetWork
 *	@post Call deleteNetWork
 * 	@return Pointer on the new NetWork
**/
netWork_t* newNetWork();

/**
 *  @brief Delete the NetWork
 * 	@param ppNetWork address of the pointer on NetWork
 * 	@see newNetWork()
**/
void deleteNetWork(netWork_t** ppNetWork);

#endif // _NET_WORK_H_

