/*
    Copyright (C) 2014 Parrot SA

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the 
      distribution.
    * Neither the name of Parrot nor the names
      of its contributors may be used to endorse or promote products
      derived from this software without specific prior written
      permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
    OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
    AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
    OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/
/**
 *	@file common.h
 * @brief simul common file
 * @date 05/18/2012
 * @author maxime.maitre@parrot.com
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

