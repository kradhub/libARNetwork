/**
 *	@file LibNetWork.h
 *  @brief AR commands processing, manage video stream reception,
 *	 photo reception and essential navdata demux
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/


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
    eCMD_ACK CMDType;
    int int_val;
};

/**
 *  @brief init the buffer of acknowledge commande
*/
void bufCmdAckInit();

/**
 *  @brief send acknowledge command
*/
int sendCmdWithAck(AR_CMD_ACK* cmd);

/**
 *  @brief send piloting command
*/
void sendCmd(AR_CMD_ACK* cmd);

/**
 *  @brief puch a commande acknowledged
*/
void bufferPush();

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
