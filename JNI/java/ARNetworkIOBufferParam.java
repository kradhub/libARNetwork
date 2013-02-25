package com.parrot.arsdk.arnetwork;

/**
 *  Used to set the parameters of a new ioBuffer
**/
public class ARNetworkIOBufferParam 
{
    /**
     *  Type of frame send by the NETWORK_Manager
    **/
    static public enum eARNETWORK_FRAME_TYPE
    {
        UNINITIALIZED, /**< not known type*/
        ACK, /**< acknowledgment type*/
        DATA, /**< data type*/
        DATA_WITH_ACK, /**< data type with a waiting acknowledgment*/
        KEEP_ALIVE; /**< keep alive type*/
    }
    
    public static final int INFINITE_NUMBER = -1;
    
    private native long nativeNew();
    private native void nativeDelete(long jIOBufferParamPtr);
    private native int nativeSet(long jIOBufferParamPtr, int id, int dataType, int sendingWaitTimeMs, int ackTimeoutMs, int numberOfRetry, int numberOfCell, int cellSize, int isOverwriting, long isUsingVariableSizeData);
    
    private long m_IOBufferParamPtr;/**< C pointer on the IOBufferParam */

    private int m_ID;   /**< Identifier used to find the InOutBuffer in a list*/
    private int m_dataType; /**< Type of the data stored in the buffer */
    private int m_sendingWaitTimeMs;    /**< Time in millisecond between 2 send */
    private int m_ackTimeoutMs; /**< Timeout in millisecond before retry to send the data waiting an acknowledgment */
    private int m_numberOfRetry;    /**< Maximum number of retry of sending before to consider a failure */
    
    private int m_numberOfCell; /**< Maximum number of data stored */
    private int m_cellSize; /**< Size of one data in byte */
    private int m_isOverwriting;    /**< Indicator of overwriting possibility (1 = true | 0 = false)*/
    private int m_isUsingVariableSizeData;   /**< Indicator of using variable size data */
    
    /**
     *  Constructor
     *  @param ID Identifier of the ioBuffer
     *  @param dataType Type of the data stored in the buffer
     *  @param numberOfCell Maximum number of data can be stored in the buffer
    **/
    public ARNetworkIOBufferParam( int ID, eARNETWORK_FRAME_TYPE dataType, int numberOfCell )
    {
        /** allocate a C  IOBufferParam*/
        m_IOBufferParamPtr = nativeNew();
        
        /** initialization */
        m_ID = ID;
        m_dataType = dataType.ordinal();
        m_numberOfCell = numberOfCell;
        m_isOverwriting = 0;
        
        m_sendingWaitTimeMs = 1;
        m_ackTimeoutMs = -1;
        m_numberOfRetry = -1;
        m_cellSize = 0;
        m_isUsingVariableSizeData = 0;
        
        /** set the C IOBufferParam*/ 
        nativeSet(m_IOBufferParamPtr, m_ID, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_numberOfRetry, m_numberOfCell, m_cellSize, m_isOverwriting, m_isUsingVariableSizeData);
    }
    
    /**
     *  Dispose NetworkIOBufferParam
     *  @post after this function the NetworkIOBufferParam must be not used more
    **/
    public void dispose() 
    {
        if(m_IOBufferParamPtr != 0)
        {
            /** deallocate the C IOBufferParam*/
            nativeDelete(m_IOBufferParamPtr);
            m_IOBufferParamPtr = 0;
        }
    }
    
    /**
     *  Destructor
    **/
    public void finalize () throws Throwable
    {
        try {
            dispose ();
        } finally {
            super.finalize ();
        }
    }
    
    /**
     *  Set the sending time 
     *  @param sendingWaitTimeMs Time in millisecond between 2 send
    **/
    public void setSendingWaitTimeMs(int sendingWaitTimeMs) 
    {
        m_sendingWaitTimeMs = sendingWaitTimeMs;
        
        /** update the C IOBufferParam*/ 
        nativeSet( m_IOBufferParamPtr, m_ID, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_numberOfRetry, m_numberOfCell, m_cellSize, m_isOverwriting, m_isUsingVariableSizeData);
    }
    
    /**
     *  Set the Timeout 
     *  @param ackTimeoutMs Timeout in millisecond before retry to send the data waiting an acknowledgment
    **/
    public void setAckTimeoutMs(int ackTimeoutMs) 
    {
        m_ackTimeoutMs = ackTimeoutMs;
        
        /** update the C IOBufferParam*/ 
        nativeSet( m_IOBufferParamPtr, m_ID, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_numberOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_isUsingVariableSizeData);
    }
    
    /**
     *  Set the maximum number of retry 
     *  @param numberOfRetry Maximum number of retry of sending before to consider a failure
    **/
    public void setNumberOfRetry(int numberOfRetry) 
    {
        m_numberOfRetry = numberOfRetry;
        
        /** update the C IOBufferParam*/ 
        nativeSet(m_IOBufferParamPtr, m_ID, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_numberOfRetry, m_numberOfCell, m_cellSize, m_isOverwriting, m_isUsingVariableSizeData);
    }
    
    /**
     *  @brief set the size of one data
     *  @param cellSize Size of one data in byte
    **/
    public void setCellSize(int cellSize) 
    {
        m_cellSize = cellSize;
        
        /** update the C IOBufferParam*/ 
        nativeSet( m_IOBufferParamPtr, m_ID, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_numberOfRetry, m_numberOfCell, m_cellSize, m_isOverwriting, m_isUsingVariableSizeData);
    }
    
    /**
     *  Set using deported data
     *  @param useDeportedData Indicator of using deported data
    **/
    public void setDeportedData(int useDeportedData) 
    {
        m_isUsingVariableSizeData = useDeportedData;
        
        /** update the C IOBufferParam*/ 
        nativeSet(m_IOBufferParamPtr, m_ID, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_numberOfRetry, m_numberOfCell, m_cellSize, m_isOverwriting, m_isUsingVariableSizeData);
    }
    
    /**
     *  Set overwriting
     *  @param isOverwriting Indicator of overwriting possibility (1 = true | 0 = false)
    **/
    public void setIsOverwriting( int isOverwriting)
    {
        m_isOverwriting = isOverwriting;
        
        /** update the C IOBufferParam*/ 
        nativeSet(m_IOBufferParamPtr, m_ID, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_numberOfRetry, m_numberOfCell, m_cellSize, m_isOverwriting, m_isUsingVariableSizeData);
    }
   
     /**
     *  Convert to string
     *  @return string from the NetworkIOBufferParam
    **/
    public String toString()
    {
        return "{ " + 
                m_ID + ", " + 
                m_dataType + ", " + 
                m_numberOfCell + ", " + 
                m_isOverwriting + ", " + 
                m_sendingWaitTimeMs + ", " + 
                m_ackTimeoutMs + ", " + 
                m_numberOfRetry + ", " + 
                m_cellSize + ", " + 
                m_isUsingVariableSizeData + 
                "}";
    }
}

