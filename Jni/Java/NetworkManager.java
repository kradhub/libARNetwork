package com.parrot.arsdk.libnetwork;

import android.util.Log;
import com.parrot.arsdk.libsal.SALNativeData;


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
    
    private native long nativeNewManager (int recvBufferSize, int sendBufferSize,
                                            int numberOfInput, Object[] inputArray, 
                                            int numberOfOutput, Object[] outputArray,
                                            int jerror);
                                                    
    private native int nativeDeleteManager( long jpManager);
    private native int nativeManagerSocketsInit( long jpManager, String jaddr, int sendingPort, int recvPort, int recvTimeoutSec);
    
    
    private native void nativeManagerStop( long jpManager);
    private native int nativeManagerSendData(long jpManager, int inputBufferId, byte[] data);
    private native int nativeManagerReadData(long jpManager, int outputBufferId, byte[] data);
    
    private native int nativeManagerSendDeportedData( long jpManager,int inputBufferId, 
                                                        SALNativeData SALData, long pData, int dataSize );
                                                                
    private native int nativeManagerReadDeportedData( long jpManager, int outputBufferId, NetworkDataRecv data);
    
    private long mp_manager;
    private boolean m_initOk;
    
    public RunSending runSending;
    public RunReceiving runReceiving;
    
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
    
    public void close () 
    {
        if(m_initOk == true)
        {
            nativeDeleteManager(mp_manager);
            mp_manager = 0;
            m_initOk = false;
        }
    }
    
    public void finalize () 
    {
        close();
    }
    
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
    
    /*
    public int runSendingThread()
    {
        return nativeManagerRunSendingThread(mp_manager);
    }
    */
    
    /*
    public int runReceivingThread()
    {
        return nativeManagerRunReceivingThread(mp_manager);
    }
    */
    
    public void stop()
    {
        if(m_initOk == true)
        {
            nativeManagerStop(mp_manager);
        }
    }
    
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
    
    public eNETWORK_Error sendDataDeported( int inputBufferId, SALNativeData salData)
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
    
    
    public eNETWORK_Error readDataDeported( int outputBufferId, NetworkDataRecv data)
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
    
    public long getManager () 
    {
        return mp_manager;
    }
    
    public boolean isCorrectlyInitialized () 
    {
        return m_initOk;
    }
    
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

class RunSending implements Runnable
{
    private static native int nativeManagerRunSendingThread( long jpManager);
    
    long mp_manager;

    RunSending(long pManager)
    {
      mp_manager = pManager;
    }
    
    public void run()
    {
       nativeManagerRunSendingThread(mp_manager);
    }
    
}

class RunReceiving implements Runnable
{
    private static native int nativeManagerRunReceivingThread( long jpManager);
    
    long mp_manager;

    RunReceiving(long pManager)
    {
      mp_manager = pManager;
    }
    
    public void run()
    {
       nativeManagerRunReceivingThread(mp_manager);
    }
    
}
