/**
 *  @file IOBufferParam_wrapper.c
 *  @brief JNI into between the ARNETWORK_IOBufferParam.h and NetworkIOBufferParam.java
 *  @date 01/18/2013
 *  @author maxime.maitre@parrot.com
**/

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_Frame.h>
#include <libARNetwork/ARNETWORK_IOBufferParam.h>
#include <jni.h>
#include <stdlib.h>

JNIEXPORT jlong JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeNew(JNIEnv *env, jobject obj)
{
    /** local declarations */
    ARNETWORK_IOBufferParam_t* pIOBufferParam = malloc( sizeof(ARNETWORK_IOBufferParam_t) );
    
    ARNETWORK_IOBufferParam_DefaultInit(pIOBufferParam);
    
    return (long) pIOBufferParam;
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeDelete(JNIEnv *env, jobject obj, jlong jIOBufferParamPtr)
{
    /** local declarations */
    ARNETWORK_IOBufferParam_t* IOBufferParamPtr = (ARNETWORK_IOBufferParam_t*) (intptr_t) jIOBufferParamPtr;
    
    free(IOBufferParamPtr);
    IOBufferParamPtr = NULL;
}

JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeSet(JNIEnv *env, jobject obj, jlong jIOBufferParamPtr, jint ID, jint dataType, jint sendingWaitTimeMs, jint ackTimeoutMs, jint numberOfRetry, jint numberOfCell, jint dataCopyMaxSize, jint isOverwriting)
{
    /** local declarations */
    ARNETWORK_IOBufferParam_t* IOBufferParamPtr = (ARNETWORK_IOBufferParam_t*) (intptr_t) jIOBufferParamPtr;
    int error = ARNETWORK_OK;
    
    if(IOBufferParamPtr != NULL )
    {
        IOBufferParamPtr->ID = ID;
        IOBufferParamPtr->dataType = dataType;
        IOBufferParamPtr->ackTimeoutMs = ackTimeoutMs;
        IOBufferParamPtr->sendingWaitTimeMs = sendingWaitTimeMs;
        IOBufferParamPtr->numberOfRetry = numberOfRetry;
        IOBufferParamPtr->numberOfCell = numberOfCell;
        IOBufferParamPtr->dataCopyMaxSize = dataCopyMaxSize;
        IOBufferParamPtr->isOverwriting = isOverwriting;
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}
