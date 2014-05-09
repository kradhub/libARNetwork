package com.parrot.arsdk.arnetwork;

import com.parrot.arsdk.arsal.ARSALPrint;
import com.parrot.arsdk.arsal.ARNativeData;
import com.parrot.arsdk.arnetworkal.ARNetworkALManager;

/**
 * Network manager allow to send and receive data acknowledged or not.
 */
public abstract class ARNetworkManager
{
    private static final String TAG = "NetworkManager";

    private static native void nativeStaticInit ();

    private native long nativeNew(long jOSSpecificManagerPtr, int numberOfInput, Object[] inputParamArray, int numberOfOutput, Object[] outputParamArray, int timeBetweenPingsMs, int jerror );

    private native int nativeDelete( long nativeManager);

    private native void nativeStop(long nativeManager);
    private native int nativeFlush(long nativeManager);

    private native int nativeSendData( long nativeManager,int inputBufferID, ARNativeData ARData, long dataPtr, int dataSize, Object customData, int doDataCopy);
    private native int nativeReadData( long nativeManager, int outputBufferID, long dataPointer, int capacity, ARNativeData data);
    private native int nativeTryReadData( long nativeManager, int outputBufferID, long dataPointer, int capacity, ARNativeData data);
    private native int nativeReadDataWithTimeout( long nativeManager, int outputBufferID, long dataPointer, int capacity, ARNativeData data, int timeoutMs);

    private long nativeManager;

    private boolean m_initOk;
    private ARNetworkALManager alManager;

    public SendingRunnable m_sendingRunnable;
    public ReceivingRunnable m_receivingRunnable;
    
    
    static
    {
        nativeStaticInit();
    }

    /**
     * Constructor
     * @param osSpecificManager The ARNetworkALManager to use. Must be initialized and valid
     * @param inputParamArray array of the parameters of the input buffers
     * @param outputParamArray array of the parameters of the output buffers
     * @param timeBetweenPingsMs Minimum time between two pings. A negative value means "no pings", Zero means "use default value"
     */
    public ARNetworkManager(ARNetworkALManager osSpecificManager, ARNetworkIOBufferParam inputParamArray[], ARNetworkIOBufferParam outputParamArray[], int timeBetweenPingsMs)
    {
        int error = ARNETWORK_ERROR_ENUM.ARNETWORK_OK.getValue();
        m_initOk = false;
        nativeManager = nativeNew(osSpecificManager.getManager(), inputParamArray.length, inputParamArray, outputParamArray.length, outputParamArray, timeBetweenPingsMs, error);

        ARSALPrint.d (TAG, "Error:" + error );

        if( nativeManager != 0 )
        {
            m_initOk = true;
            m_sendingRunnable = new SendingRunnable(nativeManager);
            m_receivingRunnable = new ReceivingRunnable(nativeManager);
            alManager = osSpecificManager;
        }
    }

    /**
     * Dispose
     */
    public void dispose()
    {
        if(m_initOk == true)
        {
            nativeDelete(nativeManager);
            nativeManager = 0;
            m_initOk = false;
        }
    }

    /**
     * Destructor
     */
    public void finalize () throws Throwable
    {
        try
        {
            dispose ();
        }
        finally
        {
            super.finalize ();
        }
    }

    /**
     * Stop the threads of sending and reception<br>
     * Used to kill the threads calling ARNETWORK_Manager_SendingThreadRun() and ARNETWORK_Manager_ReceivingThreadRun().
     */
    public void stop()
    {
        if(m_initOk == true)
        {
            nativeStop(nativeManager);
        }
    }

    /**
     * Flush all buffers of the network manager
     * @return error ARNETWORK_ERROR_ENUM
     */
    public ARNETWORK_ERROR_ENUM Flush()
    {
        ARNETWORK_ERROR_ENUM error = ARNETWORK_ERROR_ENUM.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeFlush( nativeManager );
            error = ARNETWORK_ERROR_ENUM.getFromValue(intError);
        }
        else
        {
            error = ARNETWORK_ERROR_ENUM.ARNETWORK_ERROR_BAD_PARAMETER;
        }

        return error;
    }

    /**
     * Add data to send
     * @param inputBufferID identifier of the input buffer in which the data must be stored
     * @param arData data to send
     * @param doDataCopy indocator to copy the data in the ARNETWORK_Manager
     * @return error ARNETWORK_ERROR_ENUM
     */
    public ARNETWORK_ERROR_ENUM sendData(int inputBufferID, ARNativeData arData, Object customData,boolean doDataCopy)
    {
        ARNETWORK_ERROR_ENUM error = ARNETWORK_ERROR_ENUM.ARNETWORK_OK;

        int doDataCopyInt = (doDataCopy) ? 1 : 0;

        if(m_initOk == true)
        {
            long dataPtr =  arData.getData();
            int dataSize =  arData.getDataSize();
            int intError = nativeSendData (nativeManager, inputBufferID, arData, dataPtr, dataSize, customData, doDataCopyInt );
            error =  ARNETWORK_ERROR_ENUM.getFromValue(intError);
        }
        else
        {
            error = ARNETWORK_ERROR_ENUM.ARNETWORK_ERROR_BAD_PARAMETER;
        }

        return error;
    }

    /**
     * Read data received<br>
     * Warning: This is a blocking function
     * @param outputBufferID identifier of the output buffer in which the data must be read
     * @param data Data where store the reading
     * @return error ARNETWORK_ERROR_ENUM type
     */
    public ARNETWORK_ERROR_ENUM readData(int outputBufferID, ARNativeData data)
    {
        ARNETWORK_ERROR_ENUM error = ARNETWORK_ERROR_ENUM.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeReadData( nativeManager, outputBufferID, data.getData (), data.getCapacity (), data);
            error =  ARNETWORK_ERROR_ENUM.getFromValue(intError);
        }
        else
        {
            error = ARNETWORK_ERROR_ENUM.ARNETWORK_ERROR_BAD_PARAMETER;
        }

        return error;
    }

    /**
     * try read data received (non-blocking function)
     * @param outputBufferID identifier of the output buffer in which the data must be read
     * @param data Data where store the reading
     * @return error ARNETWORK_ERROR_ENUM type
     */
    public ARNETWORK_ERROR_ENUM tryReadData(int outputBufferID, ARNativeData data)
    {
        ARNETWORK_ERROR_ENUM error = ARNETWORK_ERROR_ENUM.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeTryReadData( nativeManager, outputBufferID, data.getData (), data.getCapacity (), data);
            error =  ARNETWORK_ERROR_ENUM.getFromValue(intError);
        }
        else
        {
            error = ARNETWORK_ERROR_ENUM.ARNETWORK_ERROR_BAD_PARAMETER;
        }

        return error;
    }

    /**
     * Read data received with timeout
     * @param outputBufferID identifier of the output buffer in which the data must be read
     * @param data Data where store the reading
     * @param timeoutMs maximum time in millisecond to wait if there is no data to read
     * @return error ARNETWORK_ERROR_ENUM type
     */
    public ARNETWORK_ERROR_ENUM readDataWithTimeout(int outputBufferID, ARNativeData data, int timeoutMs)
    {
        ARNETWORK_ERROR_ENUM error = ARNETWORK_ERROR_ENUM.ARNETWORK_OK;
        if(m_initOk == true)
        {
            int intError = nativeReadDataWithTimeout( nativeManager, outputBufferID, data.getData (), data.getCapacity (), data, timeoutMs);
            error =  ARNETWORK_ERROR_ENUM.getFromValue(intError);
        }
        else
        {
            error = ARNETWORK_ERROR_ENUM.ARNETWORK_ERROR_BAD_PARAMETER;
        }

        return error;
    }

    /**
     * Get the pointer C on the network manager
     * @return  Pointer C on the network manager
     */
    public long getManager ()
    {
        return nativeManager;
    }

    /**
     * Get is the Manager is correctly initialized and if it is usable
     * @return true is the Manager is usable
     */
    public boolean isCorrectlyInitialized ()
    {
        return m_initOk;
    }

    /**
     * CallBack for the status of the data sent or free
     * @param IoBufferId identifier of the IoBuffer is calling back
     * @param data data sent
     * @param status reason of the callback
     * @param customData custom data
     * @return ARNETWORK_MANAGER_CALLBACK_RETURN_ENUM what do in timeout case
     */
    public abstract ARNETWORK_MANAGER_CALLBACK_RETURN_ENUM onCallback (int IoBufferId, ARNativeData data, ARNETWORK_MANAGER_CALLBACK_STATUS_ENUM status, Object customData);
    
    /**
     * CallBack for the status of the data sent or free
     * @param IoBufferId identifier of the IoBuffer is calling back
     * @param data data sent
     * @param status reason of the callback
     * @param customData custom data
     * @return ARNETWORK_MANAGER_CALLBACK_RETURN_ENUM what do in timeout case
     */
    private int callback (int IoBufferId, ARNativeData data, int status, Object customData)
    {
        ARNETWORK_MANAGER_CALLBACK_STATUS_ENUM jStatus = ARNETWORK_MANAGER_CALLBACK_STATUS_ENUM.values()[status];
        
        ARNETWORK_MANAGER_CALLBACK_RETURN_ENUM retVal = onCallback (IoBufferId, data, jStatus, customData);
        
        return retVal.getValue();
    }
    
    /**
     * @brief function called on disconnect
     * @param alManager The ARNetworkAL manager
     */
    public abstract void onDisconnect (ARNetworkALManager alManager);
    
    /**
     * @brief function called on disconnect
     */
    private void disconnectCallback ()
    {
        onDisconnect (alManager);
    }
}

/**
 * Sending Runnable
 */
class SendingRunnable implements Runnable
{
    private static native int nativeSendingThreadRun( long nativeManager);

    long nativeManager;

    /**
     * Constructor
     * @param managerPtr Pointer C on the network manager
     */
    SendingRunnable(long managerPtr)
    {
        nativeManager = managerPtr;
    }

    /**
     * Manage the sending of the data
     */
    public void run()
    {
        nativeSendingThreadRun(nativeManager);
    }
}

/**
 * Reception Runnable
 */
class ReceivingRunnable implements Runnable
{
    private static native int nativeReceivingThreadRun(long nativeManager);

    long nativeManager;

    /**
     * Constructor
     * @param managerPtr Pointer C on the network manager
     */
    ReceivingRunnable(long managerPtr)
    {
        nativeManager = managerPtr;
    }

    /**
     * Manage the reception of the data.
     */
    public void run()
    {
        nativeReceivingThreadRun(nativeManager);
    }
}
