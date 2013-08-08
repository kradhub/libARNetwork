/**
 * @file IOBufferParam_wrapper.c
 * @brief JNI into between the ARNETWORK_IOBufferParam.h and NetworkIOBufferParam.java
 * @date 01/18/2013
 * @author maxime.maitre@parrot.com
 **/

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_IOBufferParam.h>
#include <libARNetworkAL/ARNETWORKAL_Frame.h>
#include <jni.h>
#include <stdlib.h>

JNIEXPORT jlong JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeNew(JNIEnv *env, jobject obj)
{
    /** local declarations */
    ARNETWORK_IOBufferParam_t* ioBufferParam = malloc (sizeof (ARNETWORK_IOBufferParam_t));

    ARNETWORK_IOBufferParam_DefaultInit (ioBufferParam);

    return (long) ioBufferParam;
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeDelete(JNIEnv *env, jobject obj, jlong jIOBufferParamPtr)
{
    /** local declarations */
    ARNETWORK_IOBufferParam_t* ioBufferParamPtr = (ARNETWORK_IOBufferParam_t*) (intptr_t) jIOBufferParamPtr;

    free(ioBufferParamPtr);
}


JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeSetId(JNIEnv *env, jobject thizz, jlong jIOBufferParamPtr, jint ID)
{
    if (jIOBufferParamPtr != 0)
    {
        ARNETWORK_IOBufferParam_t *param = (ARNETWORK_IOBufferParam_t *) (intptr_t) jIOBufferParamPtr;
        param->ID = ID;
    }
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeSetDataType(JNIEnv *env, jobject thizz, jlong jIOBufferParamPtr, jint type)
{
    if (jIOBufferParamPtr != 0)
    {
        ARNETWORK_IOBufferParam_t *param = (ARNETWORK_IOBufferParam_t *) (intptr_t) jIOBufferParamPtr;
        param->dataType = type;
    }
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeSetTimeBetweenSend(JNIEnv *env, jobject thizz, jlong jIOBufferParamPtr, jint time)
{
    if (jIOBufferParamPtr != 0)
    {
        ARNETWORK_IOBufferParam_t *param = (ARNETWORK_IOBufferParam_t *) (intptr_t) jIOBufferParamPtr;
        param->sendingWaitTimeMs = time;
    }
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeSetAckTimeoutMs(JNIEnv *env, jobject thizz, jlong jIOBufferParamPtr, jint time)
{
    if (jIOBufferParamPtr != 0)
    {
        ARNETWORK_IOBufferParam_t *param = (ARNETWORK_IOBufferParam_t *) (intptr_t) jIOBufferParamPtr;
        param->ackTimeoutMs = time;
    }
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeSetNumberOfRetry(JNIEnv *env, jobject thizz, jlong jIOBufferParamPtr, jint nb)
{
    if (jIOBufferParamPtr != 0)
    {
        ARNETWORK_IOBufferParam_t *param = (ARNETWORK_IOBufferParam_t *) (intptr_t) jIOBufferParamPtr;
        param->numberOfRetry = nb;
    }
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeSetNumberOfCell(JNIEnv *env, jobject thizz, jlong jIOBufferParamPtr, jint nb)
{
    if (jIOBufferParamPtr != 0)
    {
        ARNETWORK_IOBufferParam_t *param = (ARNETWORK_IOBufferParam_t *) (intptr_t) jIOBufferParamPtr;
        param->numberOfCell = nb;
    }
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeSetDataCopyMaxSize(JNIEnv *env, jobject thizz, jlong jIOBufferParamPtr, jint size)
{
    if (jIOBufferParamPtr != 0)
    {
        ARNETWORK_IOBufferParam_t *param = (ARNETWORK_IOBufferParam_t *) (intptr_t) jIOBufferParamPtr;
        param->dataCopyMaxSize = size;
    }
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkIOBufferParam_nativeSetIsOverwriting(JNIEnv *env, jobject thizz, jlong jIOBufferParamPtr, jboolean over)
{
    if (jIOBufferParamPtr != 0)
    {
        ARNETWORK_IOBufferParam_t *param = (ARNETWORK_IOBufferParam_t *) (intptr_t) jIOBufferParamPtr;
        param->isOverwriting = (over == JNI_TRUE) ? 1 : 0;
    }
}
