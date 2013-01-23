package com.parrot.arsdk.libnetwork;

public class NetworkParamNewIoBuffer 
{
    static public enum eNETWORK_Frame_Type
    {
        UNINITIALIZED, /**< not known type*/
        ACK, /**< acknowledgment type*/
        DATA, /**< data type*/
        DATA_WITH_ACK, /**< data type with a waiting acknowledgment*/
        KEEP_ALIVE; /**< keep alive type*/
    }
    
    //private static final String TAG = new ParamNewIoBuffer().getClass().getName();
    public static final int INFINITE_NUMBER = -1;
    
    private native long nativeNewParamNewIoBuffer();
    private native void nativeDeleteNewParamNewIoBuffer(long jpParamIoBuffer);
    private native int nativeNewParamSetting(long jpParamIoBuffer, int id, int dataType, int sendingWaitTimeMs, int ackTimeoutMs, int nbOfRetry, int numberOfCell, int cellSize, int isOverwriting, long deportedData);
    
    private long mp_paramNewIoBuffer;
  

    private int m_id;   /**< Identifier used to find the InOutBuffer in a list*/
    private int m_dataType; /**< Type of the data stored in the buffer*/
    private int m_sendingWaitTimeMs;    /**< Time in millisecond between 2 send when the InOutBuffer is used with a libNetwork/sender*/
    private int m_ackTimeoutMs; /**< Timeout in millisecond before retry to send the data waiting an acknowledgment when the InOutBuffer is used with a libNetwork/sender*/
    private int m_nbOfRetry;    /**< Maximum number of retry of sending before to consider a failure when the InOutBuffer is used with a libNetwork/sender*/
    
    private int m_numberOfCell; /**< Maximum number of data stored*/
    private int m_cellSize; /**< Size of one data in byte*/
    private int m_isOverwriting;    /**< Indicator of overwriting possibility (1 = true | 0 = false)*/
    private int m_deportedData;   /**< Indicator of using data deported */
    
    public NetworkParamNewIoBuffer( int id, eNETWORK_Frame_Type dataType, int numberOfCell )
    {
        mp_paramNewIoBuffer = nativeNewParamNewIoBuffer();
        
        m_id = id;
        m_dataType = dataType.ordinal();
        m_numberOfCell = numberOfCell;
        m_isOverwriting = 0;
        
        m_sendingWaitTimeMs = 1;
        m_ackTimeoutMs = -1;
        m_nbOfRetry = -1;
        m_cellSize = 0;
        m_deportedData = 0;
        
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs,
                               m_nbOfRetry, m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData );
    }
    
    public NetworkParamNewIoBuffer( int id, eNETWORK_Frame_Type dataType, int numberOfCell, int isOverwriting )
    {
        mp_paramNewIoBuffer = nativeNewParamNewIoBuffer();
        
        m_id = id;
        m_dataType = dataType.ordinal();
        m_numberOfCell = numberOfCell;
        m_isOverwriting = isOverwriting;
        
        m_sendingWaitTimeMs = 1;
        m_ackTimeoutMs = -1;
        m_nbOfRetry = -1;
        m_cellSize = 0;
        m_deportedData = 0;
        
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs,
                               m_nbOfRetry, m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData );
    }
    
    public void finalize () 
    {
        nativeDeleteNewParamNewIoBuffer(mp_paramNewIoBuffer);
        mp_paramNewIoBuffer = 0;
    }
    
    public void settingForInput (int sendingWaitTimeMs, int ackTimeoutMs, int nbOfRetry, int cellSize ) 
    {
        m_sendingWaitTimeMs = sendingWaitTimeMs;
        m_ackTimeoutMs = ackTimeoutMs;
        m_nbOfRetry = nbOfRetry;
        m_cellSize = cellSize;
        
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData);
    }
    
    public void settingForInput (int sendingWaitTimeMs, int cellSize ) 
    {
        m_sendingWaitTimeMs = sendingWaitTimeMs;
        m_cellSize = cellSize;
        
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData);
    }
    
    public void settingForInputDeported (int sendingWaitTimeMs, int ackTimeoutMs, int nbOfRetry, int deportedData ) 
    {
        m_sendingWaitTimeMs = sendingWaitTimeMs;
        m_ackTimeoutMs = ackTimeoutMs;
        m_nbOfRetry = nbOfRetry;
        m_deportedData = deportedData;
        
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData );
    }
    
    public void settingForInputDeported (int sendingWaitTimeMs, int deportedData ) 
    {
        m_sendingWaitTimeMs = sendingWaitTimeMs;
        m_deportedData = deportedData;
        
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData );
    }
    
    public void settingForOutput ( int cellSize ) 
    {

        m_cellSize = cellSize;
        
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData );
    }
    
    public void settingForOutputDeported( int deportedData ) 
    {
        m_deportedData = deportedData;
        
        nativeNewParamSetting( mp_paramNewIoBuffer, m_id, m_dataType, m_sendingWaitTimeMs, m_ackTimeoutMs, m_nbOfRetry,
                               m_numberOfCell, m_cellSize, m_isOverwriting, m_deportedData);
    }
   
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

