/**
 *	@file circularBuffer.h
 *  @brief circular buffer for the commands to send
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _CIRCULAR_BUFFER_H_
#define _CIRCULAR_BUFFER_H_

// static :
#define BUFFER_CMD_SIZE 7
#define BUFFER_CMD_SQUARE_SIZE 10

//Enumerations :


// !!! put in common.h ?????
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
 *  @brief Acknowledged command
**/
typedef struct AR_CMD_ACK AR_CMD_ACK;
struct AR_CMD_ACK
{
    eCMD_ACK CMDType;//!!
    int int_val;//!!
};

/**
 *  @brief used for buffering the acknowledge commands 
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
 *  @brief Create a buffer of acknowledged commands
 *	@post Call deleteBuffCmdAck()
 * 	@return Pointer on the new buffer of acknowledged commands
**/
netWork_buffSend_t* newBuffCmdAck(); 

/**
 *  @brief Delete the buffer of acknowledged command
 * 	@param ppBuffsend address of the pointer on the buffer of acknowledged command
 *	@see newBuffCmdAck()
**/
void deleteBuffCmdAck(netWork_buffSend_t** ppBuffsend);

/**
 *  @brief Add acknowledged command in the sending buffer
**/
int addAckCmd(netWork_buffSend_t* pBuffsend, AR_CMD_ACK* cmd);

/**
 *  @brief send the oldest acknowledged command
 * 	not used directly, called by runSendingThread()
 * 	@see runSendingThread()
**/
void sendAckCmd(netWork_buffSend_t* pBuffsend);

#endif // _CIRCULAR_BUFFER_H_

