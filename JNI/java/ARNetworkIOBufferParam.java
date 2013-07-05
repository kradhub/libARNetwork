package com.parrot.arsdk.arnetwork;

import com.parrot.arsdk.arnetworkal.ARNETWORKAL_FRAME_TYPE_ENUM;
import com.parrot.arsdk.arsal.ARSALPrint;

/**
 * Used to set the parameters of a new ioBuffer
 */
public class ARNetworkIOBufferParam
{
    public static final int INFINITE_NUMBER = -1;

    private static final String TAG = ARNetworkIOBufferParam.class.getSimpleName ();

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
    }

    /**
     * Dispose NetworkIOBufferParam
     * @post after this function the NetworkIOBufferParam must be not used more
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
}
