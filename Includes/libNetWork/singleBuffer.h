/**
 *	@file singleBuffer.h
 *  @brief single buffer
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _SINGLE_BUFFER_H_
#define _SINGLE_BUFFER_H_

// static :

//Enumerations :


//Structures :

/**
 *  @brief piloting command
**/
typedef struct AR_PILOT_CMD AR_PILOT_CMD;
struct AR_PILOT_CMD
{
    int x;//!!
    int y;//!!
    int z;//!!
};

/**
 *  @brief used for buffering the piloting commands 
**/
typedef struct netWork_buffPilotCmd_t netWork_buffPilotCmd_t;
struct netWork_buffPilotCmd_t
{
    AR_PILOT_CMD pilotCmd;
    int isUpDated;
    sal_mutex_t mutex;
};


/**
 *  @brief Create a buffer of piloting command
 *	@post Call deleteBuffPilotCmd
 * 	@return Pointer on the new buffer of piloting commands
**/
netWork_buffPilotCmd_t* newBuffPilotCmd();

/**
 *  @brief Delete the buffer of piloting command
 * 	@param ppBuffPilotCmd address of the pointer on the buffer of piloting commands
 * 	@see newBuffPilotCmd()
**/
void deleteBuffPilotCmd(netWork_buffPilotCmd_t** ppBuffPilotCmd);

/**
 *  @brief update the piloting command
**/
void updatePilotingCmd(netWork_buffPilotCmd_t* buffPilotCmd, AR_PILOT_CMD* cmd);

/**
 *  @brief send the piloting command
**/
void sendPilotingCmd(netWork_buffPilotCmd_t* buffPilotCmd);

/*
getNavData( struct*  ) : send the NavData respecting the protocol TBD 
addressFrame*  getLastFrame() : (circular buffer freeze the last buffer sent to the application )
sendCmdWithAck() :
sendCmd() : (push the cmd in a fifo … a thread send the fifo to the ar.drone)

Thread Recv
startThreadRecv() :
stopThreadRecv() :

Thread Send
startThreadSend() :
stopThreadSend() :
*/


#endif // _SINGLE_BUFFER_H_

