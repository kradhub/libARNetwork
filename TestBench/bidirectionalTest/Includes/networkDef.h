/**
 *  @file common.h
 *  @brief simul common file
 *  @date 05/18/2012
 *  @author maxime.maitre@parrot.com
**/

#ifndef _NETWORK_DEF_H_
#define _NETWORK_DEF_H_

#include <inttypes.h>

// static :

//Enumerations :

#define RING_BUFFER_SIZE 256
#define RING_BUFFER_CELL_SIZE 10

#define ID_SEND_RING_BUFF 1
#define ID_SEND_SING_BUFF 2

typedef enum eID_BUFF
{
    ID_CHAR_DATA = 5, //manager ->
    ID_INT_DATA_WITH_ACK, //manager ->
    ID_DEPORT_DATA_ACK,//manager ->
    ID_DEPORT_DATA,//manager ->
    ID_KEEP_ALIVE, //manager ->
    ID_CHAR_DATA_2, //manager <-
    ID_INT_DATA_WITH_ACK_2, //manager <-
    ID_DEPORT_DATA_ACK_2,//manager <-
    ID_DEPORT_DATA_2,//manager <-
}eID_BUFF;

#endif // _NETWORK_DEF_H_

