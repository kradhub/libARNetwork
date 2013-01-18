package com.parrot.arsdk.libnetwork;

public enum NETWORK_Frame_Type
{
    UNINITIALIZED, /**< not known type*/
    ACK, /**< acknowledgment type*/
    DATA, /**< data type*/
    DATA_WITH_ACK, /**< data type with a waiting acknowledgment*/
    KEEP_ALIVE; /**< keep alive type*/
}
