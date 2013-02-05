package com.parrot.arsdk.libnetwork;

import android.util.Log;
import com.parrot.arsdk.libsal.SALNativeData;

/**
 * Network manager allow to send and receive data acknowledged or not.
**/
public class NetworkManager 
{
    private static final String tag = "NetworkManager";
    
    protected enum eNETWORK_CALLBACK_STATUS
    {
        NETWORK_CALLBACK_STATUS_SENT, /**< data sent  */
        NETWORK_CALLBACK_STATUS_SENT_WITH_ACK, /**< data acknowledged sent  */
        NETWORK_CALLBACK_STATUS_TIMEOUT, /**< timeout occurred, data not received */
        NETWORK_CALLBACK_STATUS_FREE; /**< free the data not sent*/
    }
    
    protected enum eNETWORK_CALLBACK_RETURN
    {
        NETWORK_CALLBACK_RETURN_DEFECT, /**<   */
        NETWORK_CALLBACK_RETURN_RETRY, /**< reset the number of retry  */
        NETWORK_CALLBACK_RETURN_DATA_POP, /**< pop the data */
        NETWORK_CALLBACK_RETURN_FLUSH; /**< FLUSH all input buffers*/
        
    }
    
    private native long nativeNewManager( int recvBufferSize, int sendBufferSize,
                                            int numberOfInput, Object[] inputArray, 
                                            int numberOfOutput, Object[] outputArray,
                                            int jerror );
                                                    
    private native int nativeDeleteManager( long jpManager);
    private native int nativeManagerSocketsInit( long jpManager, String jaddr, int sendingPort,
                                                   int recvPort, int recvTimeoutSec );
    
    private native void nativeManagerStop( long jpManager );
    private native int nativeManagerFlush( long jpManager );
    
    private native int nativeManagerSendData( long jpManager, int inputBufferId, byte[] data );
    private native int nativeManagerReadData( long jpManager, int outputBufferId, byte[] data );
    private native int nativeManagerReadDataWithTimeout( long jpManager, int outputBufferId,
                                                           byte[] data, int timeoutMs );
    
    private native int nativeManagerSendDeportedData( long jpManager,int inputBufferId, 
                                                        SALNativeData SALData, long pData, 
                                                        int dataSize);
    private native int nativeManagerReadDeportedData( long jpManager, int outputBufferId,
                                                        NetworkDataRecv data);
    private native int nativeManagerReadDeportedDataWithTimeout( long jpManager, 
                                                                   int outputBufferId, 
                                                                   NetworkDataRecv data, 
                                                                   int timeoutMs);
    
    private long mp_manager;
    private boolean m_initOk;
    
    public RunSending runSending;
    public RunReceiving runReceiving;
    
    /**
     *  Constructor
     *  @param recvBufferSize Size of the reception buffer
     *  @param sendBufferSize Size of the sending buffer
     *  @param inputArray array of the parameters of the input buffers
     *  @param outputArray array of the parameters of the output buffers
    **/
    public NetworkManager(int recvBufferSize ,int sendBufferSize, 
                             NetworkParamNewIoBuffer inputArray[], 
                             NetworkParamNewIoBuffer outputArray[]) 
    {
        int error = eNETWORK_Error.NETWORK_OK.ordinal();
        m_initOk = false;
        mp_manager = nativeNewManager( recvBufferSize, sendBufferSize, inputArray.length,
                                        inputArray, outputArray.length, outputArray, error );
        
        Log.d ("LOG_TAG", "Error:" + error );
        
        if( mp_manager != 0 )
        {
            m_initOk = true; 
            runSending = new RunSending(mp_manager);
            runReceiving = new RunReceiving(mp_manager);
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
            nativeDeleteManager(mp_manager);
            mp_manager = 0;
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
     *  @param recvPort port on which the data will be received.
     *  @param recvTimeoutSec timeout in seconds set on the socket to limit the time of blocking of the function NETWORK_ReceiverRead().
     *  @return error equal to NETWORK_OK if the initialization if successful otherwise see eNETWORK_Manager_Error.
    **/
    public eNETWORK_Error socketsInit (String addr, int sendingPort, int recvPort, int recvTimeoutSec)
    {
        eNETWORK_Error error = eNETWORK_Error.NETWORK_OK;
        
        if(addr != null)
        {
            int intError = nativeManagerSocketsInit( mp_manager, addr, sendingPort, recvPort, recvTimeoutSec);
            error =  eNETWORK_Error.getErrorName(intError); 
        }
        
        return error;
    }
    
    /**
     *  Stop the threads of sending and reception
     *  @details Used to kill the threads calling NETWORK_ManagerRunSendingThread() and NETWORK_ManagerRunReceivingThread().
     *  @see NETWORK_ManagerRunSendingThread()
     *  @see NETWORK_ManagerRunReceivingThread()
    **/
    public void stop()
    {
        if(m_initOk == true)
        {
            nativeManagerStop(mp_manager);
        }
    }
    
    /**
     *  Flush all buffers of the network manager
     *  @return error eNETWORK_Error
    **/
    public eNETWORK_Error Flush()
    {
        eNETWORK_Error error = eNETWORK_Error.NETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeManagerFlush( mp_manager );
            error =  eNETWORK_Error.getErrorName(intError);
        }
        else
        {
            error = eNETWORK_Error.NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Add data to send
     *  @param inputBufferId identifier of the input buffer in which the data must be stored
     *  @param data pointer on the data to send
     *  @return error eNETWORK_Error
    **/
    public eNETWORK_Error SendData( int inputBufferId, byte[] data)
    {
        eNETWORK_Error error = eNETWORK_Error.NETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeManagerSendData(mp_manager, inputBufferId, data);
            error =  eNETWORK_Error.getErrorName(intError); 
        }
        else
        {
            error = eNETWORK_Error.NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Add deported data to send
     *  @param inputBufferId identifier of the input buffer in which the data must be stored
     *  @param salData data to send
     *  @return error eNETWORK_Error
    **/
    public eNETWORK_Error sendDeportedData( int inputBufferId, SALNativeData salData)
    {
        eNETWORK_Error error = eNETWORK_Error.NETWORK_OK;
        if(m_initOk == true)
        {
            long pData =  salData.getData();
            int dataSize =  salData.getDataSize();
            int intError = nativeManagerSendDeportedData( mp_manager, inputBufferId, salData, pData, dataSize );
            error =  eNETWORK_Error.getErrorName(intError);  
        }
        else
        {
            error = eNETWORK_Error.NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Read data received
     *  @warning the outputBuffer should not be deportedData type
     *  @param outputBufferId identifier of the output buffer in which the data must be read
     *  @param pData pointer on the data read
     *  @return error eNETWORK_Error
    **/
    public eNETWORK_Error readData( int outputBufferId, byte[] data)
    {
        eNETWORK_Error error = eNETWORK_Error.NETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeManagerReadData(mp_manager, outputBufferId, data);
            error =  eNETWORK_Error.getErrorName(intError);  
        }
        else
        {
            error = eNETWORK_Error.NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Read data received with timeout
     *  @details This function is blocking
     *  @warning the outputBuffer should not be deportedData type
     *  @param outputBufferId identifier of the output buffer in which the data must be read
     *  @param data pointer on the data read
     *  @param timeoutMs maximum time in millisecond to wait if there is no data to read
     *  @return error eNETWORK_Error
    **/
    public eNETWORK_Error readDataWithTimeout( int outputBufferId, byte[] data, int timeoutMs)
    {
        eNETWORK_Error error = eNETWORK_Error.NETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeManagerReadDataWithTimeout(mp_manager, outputBufferId, data, timeoutMs);
            error =  eNETWORK_Error.getErrorName(intError);  
        }
        else
        {
            error = eNETWORK_Error.NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Read deported data received
     *  @warning the outputBuffer must be deportedData type
     *  @param outputBufferId identifier of the output buffer in which the data must be read
     *  @param data Data where store the reading
     *  @return error eNETWORK_Error type
    **/
    public eNETWORK_Error readDeportedData( int outputBufferId, NetworkDataRecv data)
    {
        eNETWORK_Error error = eNETWORK_Error.NETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeManagerReadDeportedData( mp_manager, outputBufferId, data);
            error =  eNETWORK_Error.getErrorName(intError);  
        }
        else
        {
            error = eNETWORK_Error.NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Read deported data received with timeout
     *  @warning the outputBuffer must be deportedData type
     *  @param outputBufferId identifier of the output buffer in which the data must be read
     *  @param data Data where store the reading
     *  @param timeoutMs maximum time in millisecond to wait if there is no data to read
     *  @return error eNETWORK_Error type
    **/
    public eNETWORK_Error readDeportedDataWithTimeout( int outputBufferId, NetworkDataRecv data, int timeoutMs)
    {
        eNETWORK_Error error = eNETWORK_Error.NETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeManagerReadDeportedDataWithTimeout( mp_manager, outputBufferId, data, timeoutMs);
            error =  eNETWORK_Error.getErrorName(intError);  
        }
        else
        {
            error = eNETWORK_Error.NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  Get the pointer C on the network manager 
     *  @return  Pointer C on the network manager 
    **/
    public long getManager () 
    {
        return mp_manager;
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
     *  @return eNETWORK_CALLBACK_RETURN what do in timeout case
    **/
    public int callback (int IoBuffer, SALNativeData data, int status) 
    {
        /** -- callback -- */
        
        /** local declarations */
        int retry = 0;
        
        eNETWORK_CALLBACK_STATUS jStatus = eNETWORK_CALLBACK_STATUS.values()[status];
        
        switch( jStatus )
        {
            case NETWORK_CALLBACK_STATUS_SENT :
            case NETWORK_CALLBACK_STATUS_SENT_WITH_ACK :
            case NETWORK_CALLBACK_STATUS_FREE :
                retry = eNETWORK_CALLBACK_RETURN.NETWORK_CALLBACK_RETURN_DATA_POP.ordinal();
            break;
            
            case NETWORK_CALLBACK_STATUS_TIMEOUT :
                retry = eNETWORK_CALLBACK_RETURN.NETWORK_CALLBACK_RETURN_RETRY.ordinal();
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
class RunSending implements Runnable
{
    private static native int nativeManagerRunSendingThread( long jpManager);
    
    long mp_manager;

    /**
     *  Constructor
     *  @param pManager Pointer C on the network manager
    **/
    RunSending(long pManager)
    {
      mp_manager = pManager;
    }
    
    /**
     *  Manage the sending of the data
    **/
    public void run()
    {
       nativeManagerRunSendingThread(mp_manager);
    }
}

/**
 *  Reception Runnable
**/
class RunReceiving implements Runnable
{
    private static native int nativeManagerRunReceivingThread( long jpManager);
    
    long mp_manager;
    
    /**
     *  Constructor
     *  @param pManager Pointer C on the network manager
    **/
    RunReceiving(long pManager)
    {
      mp_manager = pManager;
    }
    
    /**
     *  Manage the reception of the data.
    **/
    public void run()
    {
       nativeManagerRunReceivingThread(mp_manager);
    }
}
