package com.parrot.arsdk.libnetwork;

import android.util.SparseArray;

public enum eNETWORK_Error
{
    NETWORK_OK (0), /**< no error */
    NETWORK_ERROR ( -1000), /**< error unknown */ 
    NETWORK_ERROR_ALLOC (-999), /**< allocation error */
    NETWORK_ERROR_BAD_PARAMETER (-998), /**< parameter incorrect */ 
    NETWORK_ERROR_ID_UNKNOWN (-997), /**< IoBuffer identifier unknown */ 
    NETWORK_ERROR_BUFFER_SIZE (-996), /**< free space of the buffer is insufficient */
    NETWORK_ERROR_BUFFER_EMPTY (-995), /**< try to read a buffer empty */
    NETWORK_ERROR_SEMAPHORE (-994), /**< error during the using of a semaphore */
    NETWORK_MANAGER_ERROR (-2000), /**< manager error unknown */ 
    NETWORK_MANAGER_ERROR_NEW_IOBUFFER (-1999), /**< error during the IoBuffer creation */ 
    NETWORK_MANAGER_ERROR_NEW_SENDER (-1998), /**< error during the Sender creation */ 
    NETWORK_MANAGER_ERROR_NEW_RECEIVER (-1997), /**< error during the Receiver creation */
    NETWORK_ERROR_NEW_BUFFER (-1996), /**< error during the buffer creation */
    NETWORK_ERROR_NEW_RINGBUFFER (-1995), /**< error during the RingBuffer creation */
    NETWORK_IOBUFFER_ERROR (-3000), /**< IoBuffer error unknown */ 
    NETWORK_IOBUFFER_ERROR_BAD_ACK (-2999),  /**< the sequence number isn't same as that waiting */
    NETWORK_SCOCKET_ERROR (-4000),  /**< socket error unknown */
    NETWORK_SCOCKET_ERROR_PERMISSION_DENIED (-3999); /**< Permission denied */
    
    private final int m_val;
    
    static SparseArray<eNETWORK_Error> errList;
    
    eNETWORK_Error(int val)
    {
        m_val = val;
    }
    
    public int getValue()
    {
        return m_val;
    }
    
    public boolean Compare(int val)
    {
    	return m_val == val;
    }
    
    public static eNETWORK_Error getErrorName(int val)
    {
    	if (null == errList)
    	{
        	eNETWORK_Error[] errorList = eNETWORK_Error.values();
    		errList = new SparseArray<eNETWORK_Error> (errorList.length);
    		for (eNETWORK_Error err : errorList) {
    			errList.append(err.getValue(), err);
    		}
    	}
    	return errList.get(val);
        
    	/*
    	eNETWORK_Error[] errorList = eNETWORK_Error.values();
        for(int i = 0; i < errorList.length; i++)
        {
            if(errorList[i].Compare(val))
                return errorList[i];
        }
        return eNETWORK_Error.NETWORK_ERROR;
        */
    }
    
}
