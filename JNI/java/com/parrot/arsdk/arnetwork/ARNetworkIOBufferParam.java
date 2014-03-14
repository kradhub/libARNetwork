package com.parrot.arsdk.arnetwork;

import com.parrot.arsdk.arnetworkal.ARNETWORKAL_FRAME_TYPE_ENUM;
import com.parrot.arsdk.arsal.ARSALPrint;

/**
 * Used to set the parameters of a new ioBuffer
 */
public class ARNetworkIOBufferParam
{
    private static final String TAG = ARNetworkIOBufferParam.class.getSimpleName ();

    /**
     * @brief Infinite value for the IOBufferParams
     */
    public static int ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER;

    /**
     * @brief Automatically use the maximum value for dataCopyMaxSize.
     */
    public static int ARNETWORK_IOBUFFERPARAM_DATACOPYMAXSIZE_USE_MAX;

    private native static int nativeStaticGetInfiniteNumber();
    private native static int nativeStaticGetDataCopyMaxSizeUseMax();
    
    private native long nativeNew();
    private native void nativeDelete(long jIOBufferParamPtr);
    private native void nativeSetId(long jIOBufferParamPtr, int ID);
    private native void nativeSetDataType(long jIOBufferParamPtr, int type);
    private native void nativeSetTimeBetweenSend(long jIOBufferParamPtr, int time);
    private native void nativeSetAckTimeoutMs(long jIOBufferParamPtr, int time);
    private native void nativeSetNumberOfRetry(long jIOBufferParamPtr, int nb);
    private native void nativeSetNumberOfCell(long jIOBufferParamPtr, int nb);
    private native void nativeSetDataCopyMaxSize(long jIOBufferParamPtr, int size);
    private native void nativeSetIsOverwriting(long jIOBufferParamPtr, boolean over);

    private long cIOBufferParam;
    
    private int id;
    private ARNETWORKAL_FRAME_TYPE_ENUM dataType;
    private int timeBetweenSend;
    private int ackTimeoutMs;
    private int numberOfRetry;
    private int numberOfCell;
    private int copyMaxSize;
    private boolean isOverwriting;
    
    static
    {
        ARNETWORK_IOBUFFERPARAM_INFINITE_NUMBER = nativeStaticGetInfiniteNumber ();
        ARNETWORK_IOBUFFERPARAM_DATACOPYMAXSIZE_USE_MAX = nativeStaticGetDataCopyMaxSizeUseMax ();
    }
    
    /**
     * Constructor
     * @param id Identifier of the buffer
     * @param dataType Type of the buffer
     * @param timeBetweenSendMs Minimum time (ms) between two send
     * @param ackTimeoutMs Timeout (ms) of acknowledges
     * @param numberOfRetry Maximum number or retries (only for acknowledged data)
     * @param numberOfCell Capacity of the buffer
     * @param dataCopyMaxSize Maximum size (bytes) of a data to copy
     * @param isOverwriting Overwriting flag
     */
    public ARNetworkIOBufferParam (int id, ARNETWORKAL_FRAME_TYPE_ENUM dataType, int timeBetweenSendMs, int ackTimeoutMs, int numberOfRetry, int numberOfCell, int dataCopyMaxSize, boolean isOverwriting) {
        /** allocate a C  IOBufferParam*/
        cIOBufferParam = nativeNew();

        nativeSetId (cIOBufferParam, id);
        nativeSetDataType (cIOBufferParam, dataType.ordinal ());
        nativeSetTimeBetweenSend (cIOBufferParam, timeBetweenSendMs);
        nativeSetAckTimeoutMs (cIOBufferParam, ackTimeoutMs);
        nativeSetNumberOfRetry (cIOBufferParam, numberOfRetry);
        nativeSetNumberOfCell (cIOBufferParam, numberOfCell);
        nativeSetDataCopyMaxSize (cIOBufferParam, dataCopyMaxSize);
        nativeSetIsOverwriting (cIOBufferParam, isOverwriting);
        
        this.id = id;
        this.dataType = dataType;
        this.timeBetweenSend = timeBetweenSend;
        this.ackTimeoutMs = ackTimeoutMs;
        this.numberOfRetry = numberOfRetry;
        this.numberOfCell = numberOfCell;
        this.copyMaxSize = copyMaxSize;
        this.isOverwriting = isOverwriting;
    }

    /**
     * Dispose NetworkIOBufferParam
     */
    public void dispose() {
        if(cIOBufferParam != 0) {
            nativeDelete(cIOBufferParam);
            cIOBufferParam = 0;
        }
    }

    /**
     * Destructor
     */
    public void finalize () throws Throwable {
        try {
            if (cIOBufferParam != 0) {
                ARSALPrint.e(TAG, "Object " + toString () + " was not disposed by the application !");
                dispose ();
            }
        } finally {
            super.finalize ();
        }
    }

    public long getNativePointer () {
        return cIOBufferParam;
    }
    
    public int getId ()
    {
        return this.id;
    }
    
    public ARNETWORKAL_FRAME_TYPE_ENUM getDataType ()
    {
        return this.dataType;
    }
    
    public int getTimeBetweenSend ()
    {
        return this.timeBetweenSend;
    }
    
    public int getAckTimeoutMs ()
    {
        return this.ackTimeoutMs;
    }
    
    public int getNumberOfRetry ()
    {
        return this.numberOfRetry;
    }
    
    public int getNumberOfCell ()
    {
        return this.numberOfCell;
    }
    
    public int getCopyMaxSize ()
    {
        return this.copyMaxSize;
    }
    
    public boolean getIsOverwriting ()
    {
        return this.isOverwriting;
    }
    
}
