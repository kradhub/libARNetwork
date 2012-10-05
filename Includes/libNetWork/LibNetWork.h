/**
 *	@file LibNetWork.h
 *  @brief AR commands processing, manage video stream reception,
 *	 photo reception and essential navdata demux
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _LIB_NET_WORK_H_
#define _LIB_NET_WORK_H_

// static :
#define BUFFER_CMD_SIZE 7
#define BUFFER_CMD_SQUARE_SIZE 10

//Enumerations :

/**
 *  @brief type of the acknowledge command
**/
typedef enum eCMD_ACK eCMD_ACK;
enum eCMD_ACK
{
    CMD_ACK_TAKE_OFF, 
	CMD_ACK_LANDING,
	CMD_ACK_EMERGENCY,
	CMD_ACK_LOOPING,
	CMD_ACK_STAR_STOP_RECORD,
	CMD_ACK_TAKE_SNAPSHOT,
	CMD_ACK_STAR_STOP_USERBOX
};

//Structures :

/**
 *  @brief structure send for the acknowledge command
**/
typedef struct AR_CMD_ACK AR_CMD_ACK;
struct AR_CMD_ACK
{
    eCMD_ACK CMDType;//!!
    int int_val;//!!
};

/**
 *  @brief structure
**/
typedef struct netWork_buffSend_t netWork_buffSend_t;
struct netWork_buffSend_t
{
    AR_CMD_ACK* buffCmdAck;
    int buffCmdAckNbData;
    int buffIndexInput;
    int buffIndexOutput;
    sal_mutex_t mutex;
    //buffCmdSize;
	//buffCmdSquareSize;
};

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
 *  @brief structure
**/
typedef struct netWork_buffPilotCmd_t netWork_buffPilotCmd_t;
struct netWork_buffPilotCmd_t
{
    AR_PILOT_CMD pilotCmd;
    int isUpDated;
    sal_mutex_t mutex;
};

/**
 *  @brief Init the buffer of acknowledged command
 *	@pre	call bufCmdAckDelete
**/
void buffCmdAckInit(netWork_buffSend_t* pBuffsend);

/**
 *  @brief Delete the buffer of acknowledged command
 *	@see	buffCmdAckInit
**/
void buffCmdAckDelete(netWork_buffSend_t* pBuffsend);

/**
 *  @brief Add acknowledged command in the sending buffer
**/
int addAckCmd(netWork_buffSend_t* pBuffsend, AR_CMD_ACK* cmd);


/**
 *  @brief send the oldest acknowledged command
**/
void sendAckCmd(netWork_buffSend_t* pBuffsend);


/**
 *  @brief Init the buffer of acknowledged command
 *	@pre	call bufCmdAckDelete
**/
void buffCmdAckInit();


/**
 *  @brief Init the buffer of piloting command
**/
void buffPilotCmdInit(netWork_buffPilotCmd_t* buffPilotCmd);

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


#endif // _LIB_NET_WORK_H_

