package com.parrot.arsdk.arnetwork;

import android.util.Log;
import com.parrot.arsdk.arsal.ARNativeData;
import com.parrot.arsdk.arnetworkal.ARNetworkALManager;

/**
 * Network manager allow to send and receive data acknowledged or not.
**/
public abstract class ARNetworkManager 
{
    private static final String TAG = "NetworkManager";
    
    protected enum eARNETWORK_MANAGER_CALLBACK_STATUS
    {
        ARNETWORK_MANAGER_CALLBACK_STATUS_SENT, /**< data sent  */
        ARNETWORK_MANAGER_CALLBACK_STATUS_ACK_RECEIVED, /**< acknowledged received */
        ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT, /**< timeout occurred, data not received; the callback must return what the network manager must do with the data. */
        ARNETWORK_MANAGER_CALLBACK_STATUS_CANCEL, /**< data will not be sent */
        ARNETWORK_MANAGER_CALLBACK_STATUS_FREE /**< free the data not sent; in case of variable size data, this data can be reused or freed. */
    }
    
    protected enum eARNETWORK_MANAGER_CALLBACK_RETURN
    {
        ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT, /**< default value must be returned when the status callback differ of ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT */
        ARNETWORK_MANAGER_CALLBACK_RETURN_RETRY, /**< reset the number of retry  */
        ARNETWORK_MANAGER_CALLBACK_RETURN_DATA_POP, /**< pop the data */
        ARNETWORK_MANAGER_CALLBACK_RETURN_FLUSH; /**< FLUSH all input buffers*/
        
    }
    
    private native long nativeNew(long jOSSpecificManagerPtr, int numberOfInput, Object[] inputParamArray, int numberOfOutput, Object[] outputParamArray, int jerror );
    
    private native int nativeDelete( long jManagerPtr);
        
    private native void nativeStop(long jManagerPtr);
    private native int nativeFlush(long jManagerPtr);
    
    private native int nativeSendData( long jManagerPtr,int inputBufferID, ARNativeData ARData, long dataPtr, int dataSize, int doDataCopy);
    private native int nativeReadData( long jManagerPtr, int outputBufferID, ARNetworkDataRecv data);
    private native int nativeTryReadData( long jManagerPtr, int outputBufferID, ARNetworkDataRecv data);
    private native int nativeReadDataWithTimeout( long jManagerPtr, int outputBufferID, ARNetworkDataRecv data, int timeoutMs);
    
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
    public ARNetworkManager(ARNetworkALManager osSpecificManager, ARNetworkIOBufferParam inputParamArray[], ARNetworkIOBufferParam outputParamArray[]) 
    {
        int error = eARNETWORK_ERROR.ARNETWORK_OK.ordinal();
        m_initOk = false;
        m_managerPtr = nativeNew(osSpecificManager.getManager(), inputParamArray.length, inputParamArray, outputParamArray.length, outputParamArray, error);
        
        Log.d (TAG, "Error:" + error );
        
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
     *  @param arData data to send
     *  @param doDataCopy indocator to copy the data in the ARNETWORK_Manager
     *  @return error eARNETWORK_ERROR
    **/
    public eARNETWORK_ERROR sendData(int inputBufferID, ARNativeData arData, boolean doDataCopy)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        
        int doDataCopyInt = (doDataCopy) ? 1 : 0;
        
        if(m_initOk == true)
        {
            long dataPtr =  arData.getData();
            int dataSize =  arData.getDataSize();
            int intError = nativeSendData( m_managerPtr, inputBufferID, arData, dataPtr, dataSize, doDataCopyInt );
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
     *  @warning blocking function
     *  @param outputBufferID identifier of the output buffer in which the data must be read
     *  @param data Data where store the reading
     *  @return error eARNETWORK_ERROR type
    **/
    public eARNETWORK_ERROR readData(int outputBufferID, ARNetworkDataRecv data)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeReadData( m_managerPtr, outputBufferID, data);
            error =  eARNETWORK_ERROR.getErrorName(intError);  
        }
        else
        {
            error = eARNETWORK_ERROR.ARNETWORK_ERROR_BAD_PARAMETER;
        }
        
        return error;
    }
    
    /**
     *  try read data received (non-blocking function)
     *  @param outputBufferID identifier of the output buffer in which the data must be read
     *  @param data Data where store the reading
     *  @return error eARNETWORK_ERROR type
    **/
    public eARNETWORK_ERROR tryReadData(int outputBufferID, ARNetworkDataRecv data)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeTryReadData( m_managerPtr, outputBufferID, data);
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
     *  @param outputBufferID identifier of the output buffer in which the data must be read
     *  @param data Data where store the reading
     *  @param timeoutMs maximum time in millisecond to wait if there is no data to read
     *  @return error eARNETWORK_ERROR type
    **/
    public eARNETWORK_ERROR readDataWithTimeout(int outputBufferID, ARNetworkDataRecv data, int timeoutMs)
    {
        eARNETWORK_ERROR error = eARNETWORK_ERROR.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeReadDataWithTimeout( m_managerPtr, outputBufferID, data, timeoutMs);
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
    public abstract int callback (int IoBuffer, ARNativeData data, int status);
    
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
