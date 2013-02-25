package com.parrot.arsdk.arnetwork;

import java.util.HashMap;

/**
 *  libARNetwork errors known.
**/

public enum eARNETWORK_ERROR
{
    ARNETWORK_OK (0), /**< no error */
    ARNETWORK_ERROR ( -1000), /**< error unknown */ 
    ARNETWORK_ERROR_ALLOC (-999), /**< allocation error */
    ARNETWORK_ERROR_BAD_PARAMETER (-998), /**< parameter incorrect */ 
    ARNETWORK_ERROR_ID_UNKNOWN (-997), /**< IoBuffer identifier unknown */ 
    ARNETWORK_ERROR_BUFFER_SIZE (-996), /**< free space of the buffer is insufficient */
    ARNETWORK_ERROR_BUFFER_EMPTY (-995), /**< try to read a buffer empty */
    ARNETWORK_ERROR_SEMAPHORE (-994), /**< error during the using of a semaphore */
    ARNETWORK_ERROR_MANAGER (-2000), /**< manager error unknown */ 
    ARNETWORK_ERROR_MANAGER_NEW_IOBUFFER (-1999), /**< error during the IoBuffer creation */ 
    ARNETWORK_ERROR_MANAGER_NEW_SENDER (-1998), /**< error during the Sender creation */ 
    ARNETWORK_ERROR_MANAGER_NEW_RECEIVER (-1997), /**< error during the Receiver creation */
    ARNETWORK_ERROR_NEW_BUFFER (-1996), /**< error during the buffer creation */
    ARNETWORK_ERROR_NEW_RINGBUFFER (-1995), /**< error during the RingBuffer creation */
    ARNETWORK_ERROR_IOBUFFER (-3000), /**< IoBuffer error unknown */ 
    ARNETWORK_ERROR_IOBUFFER_BAD_ACK (-2999),  /**< the sequence number isn't same as that waiting */
    ARNETWORK_ERROR_SOCKET (-4000),  /**< socket error unknown */
    ARNETWORK_ERROR_SOCKET_PERMISSION_DENIED (-3999); /**< Permission denied */
    
    private final int m_val;
    static HashMap<Integer, eARNETWORK_ERROR> errList;
    
    /**
     *  Constructor
     *  @param value of the enumeration
    **/
    eARNETWORK_ERROR(int val)
    {
        m_val = val;
    }
    
    /**
     *  Get the value.
     *  @return value of the enumeration
    **/
    public int getValue()
    {
        return m_val;
    }
    
    /**
     *  Get error name from value.
     *  @param val value of the enumeration
     *  @return name of the enumeration equal to the value
    **/
    public static eARNETWORK_ERROR getErrorName(int val)
    {
        /** if the errList not exist, create it */
        if (null == errList)
        {
            eARNETWORK_ERROR[] errorList = eARNETWORK_ERROR.values();
            errList = new HashMap<Integer, eARNETWORK_ERROR> (errorList.length);
            for (eARNETWORK_ERROR err : errorList) {
                errList.put(err.getValue(), err);
            }
        }
        return errList.get(val);
    }
    
}
