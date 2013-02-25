package com.parrot.arsdk.arnetwork;

import android.util.Log;
import com.parrot.arsdk.arsal.ARNativeData;

/**
 * Network manager allow to send and receive data acknowledged or not.
**/
public class ARNetworkManager 
{
    private static final String tag = "NetworkManager";
    
    protected enum eARNETWORK_MANAGER_CALLBACK_STATUS
    {
        ARNETWORK_MANAGER_CALLBACK_STATUS_SENT, /**< data sent  */
        ARNETWORK_MANAGER_CALLBACK_STATUS_SENT_WITH_ACK, /**< data acknowledged sent  */
        ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT, /**< timeout occurred, data not received */
        ARNETWORK_MANAGER_CALLBACK_STATUS_FREE; /**< free the data not sent*/
    }
    
    protected enum eARNETWORK_MANAGER_CALLBACK_RETURN
    {
        ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT, /**< default value must be returned when the status callback differ of ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT */
        ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY, /**< reset the number of retry  */
        ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP, /**< pop the data */
        ARNETWORK_MANAGER_CALLBACK_RETURN_FLUSH; /**< FLUSH all input buffers*/
        
    }
    
    private native long nativeNew( int recvBufferSize, int sendBufferSize, int numberOfInput, Object[] inputParamArray, int numberOfOutput, Object[] outputParamArray, int jerror );
    
    private native int nativeDelete( long jManagerPtr);
    private native int nativeSocketsInit( long jManagerPtr, String jaddr, int sendingPort, int receivingPort, int recvTimeoutSec );
    
    private native void nativeStop(long jManagerPtr);
    private native int nativeFlush(long jManagerPtr);
    
    private native int nativeSendFixedSizeData( long jManagerPtr, int inputBufferID, byte[] data );
    private native int nativeReadFixedSizeData( long jManagerPtr, int outputBufferID, byte[] data );
    private native int nativeReadFixedSizeDataWithTimeout( long jManagerPtr, int outputBufferID, byte[] data, int timeoutMs );
    
    private native int nativeSendVariableSizeData( long jManagerPtr,int inputBufferID, ARNativeData ARData, long dataPtr, int dataSize);
    private native int nativeReadVarableSizeData( long jManagerPtr, int outputBufferID, ARNetworkDataRecv data);
    private native int nativeReadVarableSizeDataWithTimeout( long jManagerPtr, int outputBufferID, ARNetworkDataRecv data, int timeoutMs);
    
    private long m_managerPtr;
    private boolean m_initOk;
    
    public SendingRunnable m_sendingRunnable;
    public ReceivingRunnable m_receivingRunnable;
    
    /**
     *  Constructor
     *  @param recvBufferSize Size of the reception buffer
     *  @param sendBufferSize Size of the sending buffer
     *  @param inputParamArray array of the parameters of the input buffers
     *  @param outputParamArray array of the parameters of the output buffers
    **/
    public ARNetworkManager(int recvBufferSize ,int sendBufferSize, ARNetworkIOBufferParam inputParamArray[], ARNetworkIOBufferParam outputParamArray[]) 
    {
        int error = eARNETWORK_ERROR.ARNETWORK_OK.ordinal();
        m_initOk = false;
        m_managerPtr = nativeNew(recvBufferSize, sendBufferSize, inputParamArray.length, inputParamArray, outputParamArray.length, outputParamArray, error);
        
        Log.d ("LOG_TAG", "Error:" + error );
        
        if( m_managerPtr != 0 )
        {
            m_initOk = true; 
            m_sendingRunnable = new SendingRunnable(m_managerPtr);
            m_receivingRunnable = new ReceivingRunnable(m_managerPtr);
        }

    }
    
    /**
     *  Dispose
     *  @post after this function the object must be not used more
    **/
    public void dispose() 
    {
        if(m_initOk == true)
        {
            nativeDelete(m_managerPtr);
            m_managerPtr = 0;
            m_initOk = false;
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
     *  Initialize UDP sockets of sending and receiving the data.
     *  @param addr address of connection at which the data will be sent.
     *  @param sendingPort port on which the data will be sent.
     *  @param receivingPort port on which the data will be received.
     *  @param recvTimeoutSec timeout in seconds set on the socket to limit the time of blocking of the function ARNETWORK_Receiver_Read().
     *  @return error equal to ARNETWORK_OK if the initialization if successful otherwise see eARNETWORK_ERROR.
    **/
    public eARNETWORK_ERROR socketsInit (String addr, int sendingPort, int receivingPort, int recvTimeoutSec)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        
        if(addr != null)
        {
            int intError = nativeSocketsInit( m_managerPtr, addr, sendingPort, receivingPort, recvTimeoutSec);
            error =  eARNETWORK_ERROR.getErrorName(intError); 
        }
        
        return error;
    }
    
    /**
     *  Stop the threads of sending and reception
     *  @details Used to kill the threads calling ARNETWORK_Manager_SendingThreadRun() and ARNETWORK_Manager_ReceivingThreadRun().
     *  @see ARNETWORK_Manager_SendingThreadRun()
     *  @see ARNETWORK_Manager_ReceivingThreadRun()
    **/
    public void stop()
    {
        if(m_initOk == true)
        {
            nativeStop(m_managerPtr);
        }
    }
    
    /**
     *  Flush all buffers of the network manager
     *  @return error eARNETWORK_ERROR
    **/
    public eARNETWORK_ERROR Flush()
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeFlush( m_managerPtr );
            error = eARNETWORK_ERROR.getErrorName(intError);
        }
        else
        {
            error = eARNETWORK_ERROR.ARNETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Add data to send
     *  @param inputBufferID identifier of the input buffer in which the data must be stored
     *  @param data pointer on the data to send
     *  @return error eARNETWORK_ERROR
    **/
    public eARNETWORK_ERROR SendData( int inputBufferID, byte[] data)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeSendFixedSizeData(m_managerPtr, inputBufferID, data);
            error =  eARNETWORK_ERROR.getErrorName(intError); 
        }
        else
        {
            error = eARNETWORK_ERROR.ARNETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Add deported data to send
     *  @param inputBufferID identifier of the input buffer in which the data must be stored
     *  @param arData data to send
     *  @return error eARNETWORK_ERROR
    **/
    public eARNETWORK_ERROR sendDeportedData( int inputBufferID, ARNativeData arData)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            long dataPtr =  arData.getData();
            int dataSize =  arData.getDataSize();
            int intError = nativeSendVariableSizeData( m_managerPtr, inputBufferID, arData, dataPtr, dataSize );
            error =  eARNETWORK_ERROR.getErrorName(intError);  
        }
        else
        {
            error = eARNETWORK_ERROR.ARNETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Read data received
     *  @warning the outputBuffer should not be using variable data 
     *  @param outputBufferID identifier of the output buffer in which the data must be read
     *  @param dataPtr pointer on the data read
     *  @return error eARNETWORK_ERROR
    **/
    public eARNETWORK_ERROR readData( int outputBufferID, byte[] data)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeReadFixedSizeData(m_managerPtr, outputBufferID, data);
            error =  eARNETWORK_ERROR.getErrorName(intError);  
        }
        else
        {
            error = eARNETWORK_ERROR.ARNETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Read data received with timeout
     *  @details This function is blocking
     *  @warning the outputBuffer should not be using variable data
     *  @param outputBufferID identifier of the output buffer in which the data must be read
     *  @param data pointer on the data read
     *  @param timeoutMs maximum time in millisecond to wait if there is no data to read
     *  @return error eARNETWORK_ERROR
    **/
    public eARNETWORK_ERROR readDataWithTimeout( int outputBufferID, byte[] data, int timeoutMs)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeReadFixedSizeDataWithTimeout(m_managerPtr, outputBufferID, data, timeoutMs);
            error =  eARNETWORK_ERROR.getErrorName(intError);  
        }
        else
        {
            error = eARNETWORK_ERROR.ARNETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Read deported data received
     *  @warning the outputBuffer must be using variable data
     *  @param outputBufferID identifier of the output buffer in which the data must be read
     *  @param data Data where store the reading
     *  @return error eARNETWORK_ERROR type
    **/
    public eARNETWORK_ERROR readDeportedData(int outputBufferID, ARNetworkDataRecv data)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeReadVarableSizeData( m_managerPtr, outputBufferID, data);
            error =  eARNETWORK_ERROR.getErrorName(intError);  
        }
        else
        {
            error = eARNETWORK_ERROR.ARNETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Read deported data received with timeout
     *  @warning the outputBuffer must be using variable data
     *  @param outputBufferID identifier of the output buffer in which the data must be read
     *  @param data Data where store the reading
     *  @param timeoutMs maximum time in millisecond to wait if there is no data to read
     *  @return error eARNETWORK_ERROR type
    **/
    public eARNETWORK_ERROR readDeportedDataWithTimeout(int outputBufferID, ARNetworkDataRecv data, int timeoutMs)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeReadVarableSizeDataWithTimeout( m_managerPtr, outputBufferID, data, timeoutMs);
            error =  eARNETWORK_ERROR.getErrorName(intError);  
        }
        else
        {
            error = eARNETWORK_ERROR.ARNETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Get the pointer C on the network manager 
     *  @return  Pointer C on the network manager 
    **/
    public long getManager () 
    {
        return m_managerPtr;
    }
    
    /**
     *  Get is the Manager is correctly initialized and if it is usable
     *  @return true is the Manager is usable
    **/
    public boolean isCorrectlyInitialized () 
    {
        return m_initOk;
    }
    
    /**
     *  CallBack for the status of the data sent or free
     *  @param IoBufferId identifier of the IoBuffer is calling back
     *  @param data data sent
     *  @param status reason of the callback
     *  @return eARNETWORK_MANAGER_CALLBACK_RETURN what do in timeout case
    **/
    public int callback (int IoBuffer, ARNativeData data, int status) 
    {
        /** -- callback -- */
        
        /** local declarations */
        int retry = 0;
        
        eARNETWORK_MANAGER_CALLBACK_STATUS jStatus = eARNETWORK_MANAGER_CALLBACK_STATUS.values()[status];
        
        switch( jStatus )
        {
            case ARNETWORK_MANAGER_CALLBACK_STATUS_SENT :
            case ARNETWORK_MANAGER_CALLBACK_STATUS_SENT_WITH_ACK :
            case ARNETWORK_MANAGER_CALLBACK_STATUS_FREE :
                retry = eARNETWORK_MANAGER_CALLBACK_RETURN.ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP.ordinal();
                break;
            
            case ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT :
                retry = eARNETWORK_MANAGER_CALLBACK_RETURN.ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY.ordinal();
                break;
            
            default:
                Log.e(tag, "default case status:" + jStatus);
                break;
        }
        
        return retry;

    }

}

/**
 *  Sending Runnable
**/
class SendingRunnable implements Runnable
{
    private static native int nativeSendingThreadRun( long jManagerPtr);
    
    long m_managerPtr;

    /**
     *  Constructor
     *  @param managerPtr Pointer C on the network manager
    **/
    SendingRunnable(long managerPtr)
    {
      m_managerPtr = managerPtr;
    }
    
    /**
     *  Manage the sending of the data
    **/
    public void run()
    {
       nativeSendingThreadRun(m_managerPtr);
    }
}

/**
 *  Reception Runnable
**/
class ReceivingRunnable implements Runnable
{
    private static native int nativeReceivingThreadRun(long jManagerPtr);
    
    long m_managerPtr;
    
    /**
     *  Constructor
     *  @param managerPtr Pointer C on the network manager
    **/
    ReceivingRunnable(long managerPtr)
    {
      m_managerPtr = managerPtr;
    }
    
    /**
     *  Manage the reception of the data.
    **/
    public void run()
    {
       nativeReceivingThreadRun(m_managerPtr);
    }
}
