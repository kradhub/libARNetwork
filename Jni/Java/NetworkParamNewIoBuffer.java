package com.parrot.arsdk.libnetwork;

/**
 *  Used to set the parameters of a new ioBuffer
**/
public class NetworkParamNewIoBuffer 
{
    /**
     *  Type of frame send by the NETWORK_Manager
    **/
    static public enum eNETWORK_Frame_Type
    {
        UNINITIALIZED, /**< not known type*/
        ACK, /**< acknowledgment type*/
        DATA, /**< data type*/
        DATA_WITH_ACK, /**< data type with a waiting acknowledgment*/
        KEEP_ALIVE; /**< keep alive type*/
    }

    public static final int INFINITE_NUMBER = -1;
    
    private native long nativeNewParamNewIoBuffer();
    private native void nativeDeleteNewParamNewIoBuffer(long jpParamIoBuffer);
    private native int nativeNewParamSetting(long jpParamIoBuffer, int id, int dataType, int sendingWaitTimeMs, int ackTimeoutMs, int nbOfRetry, int numberOfCell, int cellSize, int isOverwriting, long deportedData);
    
    private long mp_paramNewIoBuffer;/**< C pointer on the paramNewIoBuffer */

    private int m_id;   /**< Identifier used to find the InOutBuffer in a list*/
    private int m_dataType; /**< Type of the data stored in the buffer */
    private int m_sendingWaitTimeMs;    /**< Time in millisecond between 2 send */
    private int m_ackTimeoutMs; /**< Timeout in millisecond before retry to send the data waiting an acknowledgment */
    private int m_nbOfRetry;    /**< Maximum number of retry of sending before to consider a failure */
    
    private int m_numberOfCell; /**< Maximum number of data stored */
    private int m_cellSize; /**< Size of one data in byte */
    private int m_isOverwriting;    /**< Indicator of overwriting possibility (1 = true | 0 = false)*/
    private int m_deportedData;   /**< Indicator of using deported data */
    
    /**
     *  Constructor
     *  @param id Identifier of the ioBuffer
     *  @param dataType Type of the data stored in the buffer
     *  @param numberOfCell Maximum number of data can be stored in the buffer
    **/
    public NetworkParamNewIoBuffer( int id, eNETWORK_Frame_Type dataType, int numberOfCell )
    {
        /** allocate a C  ParamNewIoBuffer*/
        mp_paramNewIoBuffer = nativeNewParamNewIoBuffer();
        
        /** initialization */
        m_id = id;
        m_dataType = dataType.ordinal();
        m_numberOfCell = numberOfCell;
        m_isOverwriting = 0;
        
        m_sendingWaitTimeMs = 1;
        m_ackTimeoutMs = -1;
        m_nbOfRetry = -1;
        m_cellSize = 0;
        m_deportedData = 0;
        
        /** set the C ParamNewIoBuffer*/ 
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs,
                               m_nbOfRetry, m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData );
    }
    
    /**
     *  Dispose NetworkParamNewIoBuffer
     *  @post after this function the NetworkParamNewIoBuffer must be not used more
    **/
    public void dispose() 
    {
        if(mp_paramNewIoBuffer != 0)
        {
            /** deallocate the C ParamNewIoBuffer*/
            nativeDeleteNewParamNewIoBuffer(mp_paramNewIoBuffer);
            mp_paramNewIoBuffer = 0;
        }
    }
    
    /**
     *  Destructor
    **/
    public void finalize () 
    {
        dispose();
    }
    
    /**
     *  Set the sending time 
     *  @param sendingWaitTimeMs Time in millisecond between 2 send
    **/
    public void setSendingWaitTimeMs(int sendingWaitTimeMs) 
    {
        m_sendingWaitTimeMs = sendingWaitTimeMs;
        
        /** update the C ParamNewIoBuffer*/ 
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData);
    }
    
    /**
     *  Set the Timeout 
     *  @param ackTimeoutMs Timeout in millisecond before retry to send the data waiting an acknowledgment
    **/
    public void setAckTimeoutMs(int ackTimeoutMs) 
    {
        m_ackTimeoutMs = ackTimeoutMs;
        
        /** update the C ParamNewIoBuffer*/ 
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData);
    }
    
    /**
     *  Set the maximum number of retry 
     *  @param nbOfRetry Maximum number of retry of sending before to consider a failure
    **/
    public void setNbOfRetry(int nbOfRetry) 
    {
        m_nbOfRetry = nbOfRetry;
        
        /** update the C ParamNewIoBuffer*/ 
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData);
    }
    
    /**
     *  @brief set the size of one data
     *  @param cellSize Size of one data in byte
    **/
    public void setCellSize(int cellSize) 
    {
        m_cellSize = cellSize;
        
        /** update the C ParamNewIoBuffer*/ 
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData);
    }
    
    /**
     *  Set using deported data
     *  @param useDeportedData Indicator of using deported data
    **/
    public void setDeportedData(int useDeportedData) 
    {
        m_deportedData = useDeportedData;
        
        /** update the C ParamNewIoBuffer*/ 
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData);
    }
    
    /**
     *  Set overwriting
     *  @param isOverwriting Indicator of overwriting possibility (1 = true | 0 = false)
    **/
    public void setIsOverwriting( int isOverwriting)
    {
        m_isOverwriting = isOverwriting;
        
        /** update the C ParamNewIoBuffer*/ 
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData);
    }
   
     /**
     *  Convert to string
     *  @return string from the NetworkParamNewIoBuffer
    **/
    public String toString()
    {
        return "{ " + 
                m_id + ", " + 
                m_dataType + ", " + 
                m_numberOfCell + ", " + 
                m_isOverwriting + ", " + 
                m_sendingWaitTimeMs + ", " + 
                m_ackTimeoutMs + ", " + 
                m_nbOfRetry + ", " + 
                m_cellSize + ", " + 
                m_deportedData + 
                "}";
    }
}

