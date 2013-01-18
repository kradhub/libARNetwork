/**
 *  @file paramNewIoBuffer_wrapper.c
 *  @brief JNI into between the paramNewIoBuffer.h and NetworkParamNewIoBuffer.java
 *  @date 01/18/2013
 *  @author maxime.maitre@parrot.com
**/

#include <libNetwork/status.h>
#include <libNetwork/frame.h>
#include <libNetwork/paramNewIoBuffer.h>
#include <jni.h>
#include <stdlib.h>

JNIEXPORT jlong JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkParamNewIoBuffer_nativeNewParamNewIoBuffer(JNIEnv *env, jobject obj)
{
    /** local declarations */
    network_paramNewIoBuffer_t* pParamNewIoBuffer = malloc( sizeof(network_paramNewIoBuffer_t) );
    
    NETWORK_ParamNewIoBufferDefaultInit(pParamNewIoBuffer);
    
    return (long) pParamNewIoBuffer;
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkParamNewIoBuffer_nativeDeleteNewParamNewIoBuffer(JNIEnv *env, jobject obj, jlong jpParamIoBuffer)
{
    /** local declarations */
    network_paramNewIoBuffer_t* pParamIoBuffer = (network_paramNewIoBuffer_t*) (intptr_t) jpParamIoBuffer;
    
    free(pParamIoBuffer);
    pParamIoBuffer = NULL;
}

JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkParamNewIoBuffer_nativeNewParamSetting(JNIEnv *env, jobject obj, jlong jpParamIoBuffer, jint id, jint dataType,jint sendingWaitTimeMs, jint ackTimeoutMs, jint nbOfRetry, jint numberOfCell, jint cellSize, jint isOverwriting, jint deportedData)
{
    /** local declarations */
    network_paramNewIoBuffer_t* pParamIoBuffer = (network_paramNewIoBuffer_t*) (intptr_t) jpParamIoBuffer;
    int error = NETWORK_OK;
    
    if(pParamIoBuffer != NULL )
    {
        pParamIoBuffer->id = id;
        pParamIoBuffer->dataType = dataType;
        pParamIoBuffer->ackTimeoutMs = ackTimeoutMs;
        pParamIoBuffer->sendingWaitTimeMs = sendingWaitTimeMs;
        pParamIoBuffer->nbOfRetry = nbOfRetry;
        pParamIoBuffer->numberOfCell = numberOfCell;
        pParamIoBuffer->cellSize = cellSize;
        pParamIoBuffer->isOverwriting = isOverwriting;
        pParamIoBuffer->deportedData = deportedData;
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}
