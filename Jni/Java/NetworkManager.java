package com.parrot.arsdk.libnetwork;

import android.util.Log;


public class NetworkManager 
{
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
    
    private int NETWORK_ERROR_BAD_PARAMETER =-998; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
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
                                                        byte[] jData, int dataSize );
                                                                
    private native int nativeManagerReadDeportedData( long jpManager, int outputBufferId, NetworkDataRecv data);
    
    private long mp_manager;
    private boolean m_initOk;
    
    public RunSending runSending;
    public RunReceiving runReceiving;
    
    public NetworkManager(int recvBufferSize ,int sendBufferSize, 
                             NetworkParamNewIoBuffer inputArray[], 
                             NetworkParamNewIoBuffer outputArray[]) 
    {
        int error = 0;
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
    
    public int socketsInit (String addr, int sendingPort, int recvPort, int recvTimeoutSec)
    {
        int ret = -1; // !!!!
        if(addr != null)
        {
            ret = nativeManagerSocketsInit( mp_manager, addr, sendingPort, recvPort, recvTimeoutSec);  
        }
        
        return ret;
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
    
    public int SendData( int inputBufferId, byte[] data)
    {
        int error = 0;
        if(m_initOk == true)
        {
            error = nativeManagerSendData(mp_manager, inputBufferId, data);
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    
    public int readData( int outputBufferId, byte[] data)
    {
        int error = 0;
        if(m_initOk == true)
        {
            error = nativeManagerReadData(mp_manager, outputBufferId, data);
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    public int sendDataDeported( int inputBufferId, byte[] data)
    {
        int error = 0;
        if(m_initOk == true)
        {
            error = nativeManagerSendDeportedData( mp_manager, inputBufferId, data, data.length );
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    
    public int readDataDeported( int outputBufferId, NetworkDataRecv data)
    {
        int error = 0;
        if(m_initOk == true)
        {
            error = nativeManagerReadDeportedData( mp_manager, outputBufferId, data);
        }
        else
        {
            error = NETWORK_ERROR_BAD_PARAMETER;
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
    
    public int callback (int IoBuffer, byte[] data, int status) 
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
                retry = 0;
            break;
            
            case NETWORK_CALLBACK_STATUS_TIMEOUT :
                retry = 1;
            break;
            
            default:

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
