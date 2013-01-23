/**
 *  @file manager_wrapper.c
 *  @brief JNI into between the manager.h and NetworkManager.java
 *  @date 01/18/2013
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 *             include file :
 *
******************************************/

#include <libNetwork/status.h>
#include <libNetwork/frame.h>
#include <libNetwork/manager.h>
#include <jni.h>
#include <stdlib.h>

#include <libSAL/print.h>

/*****************************************
 * 
 *             implementation :
 *
******************************************/

/**
 *  @brief data sent to the callbak 
**/
typedef struct network_JNI_callbackData_t
{
    jobject jManager; /**< manager sent the data */
    jobject jSALData; /**< java native data*/
}network_JNI_callbackData_t;

int JNI_network_deportDatacallback (int IoBufferId, uint8_t* pData, void* pCustomData, int status);

void freeCallbackData(JNIEnv* env, network_JNI_callbackData_t* pCallbackData);


static const char* TAG = "APP";
JavaVM* g_vm = NULL;

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    SAL_PRINT(PRINT_DEBUG, TAG,"Library has been loaded");

	/** Saving the reference to the java virtual machine */
	g_vm = vm;

	/** Return the JNI version */
	return JNI_VERSION_1_6;
}

/**
 *  @brief Create a new manager
**/
JNIEXPORT jlong JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeNewManager ( JNIEnv *env, jobject obj, 
                                                                  jint recvBufferSize,
                                                                  jint sendBufferSize,
                                                                  jint numberOfInput, 
                                                                  jobjectArray inputArray, 
                                                                  jint numberOfOutput,
                                                                  jobjectArray outputArray,
                                                                  jint jError )
{
    /** -- Create a new manager -- */
    
    /** local declarations */
    network_manager_t* pManager = NULL;
    network_paramNewIoBuffer_t* ptabParamInput = NULL;
    network_paramNewIoBuffer_t* ptabParamOutput = NULL;
    int ii = 0;
    eNETWORK_Error error = NETWORK_OK;
    
    jfieldID fid;
    jclass ParamNewIoBuffer_cls;
    jobject jIoBuff ;
    
    /** get the class ParamNewIoBuffer*/
    ParamNewIoBuffer_cls = (*env)->FindClass(env, "com/parrot/arsdk/libnetwork/NetworkParamNewIoBuffer");
    if (NULL == ParamNewIoBuffer_cls)
    {
        error = NETWORK_ERROR_ALLOC ;
    }
    
    if(error == NETWORK_OK)
    {
        /** allocate the input parameters C */
        
        ptabParamInput = malloc ( sizeof(network_paramNewIoBuffer_t) * numberOfInput );
        if(ptabParamInput == NULL && numberOfInput != 0 )
        {
            error = NETWORK_ERROR_ALLOC;
        }
    }
  
    if(error == NETWORK_OK)
    {
        /** copy the parameters java to the input parameters C*/
        
        for(ii = 0; ii < numberOfInput; ++ii)
        {
            /** get obj */
            jIoBuff = (*env)->GetObjectArrayElement(env, inputArray, ii);
            fid = (*env)->GetFieldID(env, ParamNewIoBuffer_cls, "mp_paramNewIoBuffer", "J" );
            
            ptabParamInput[ii] = *( (network_paramNewIoBuffer_t*) (intptr_t) (*env)->GetLongField(env, jIoBuff, fid) );
        }
    }
   
    if(error == NETWORK_OK)
    {
        /** allocate the output parameters C */
        
        ptabParamOutput = malloc ( sizeof(network_paramNewIoBuffer_t) * numberOfOutput );
        if(ptabParamOutput == NULL && numberOfInput != 0 )
        {
            error = NETWORK_ERROR_ALLOC;
        }
    }
  
    if(error == NETWORK_OK)
    {
        /** copy the parameters java to the output parameters C*/
        
        for(ii = 0; ii<  numberOfOutput; ++ii)
        {
            /** get obj */
            jIoBuff = (*env)->GetObjectArrayElement(env, outputArray, ii);
            fid = (*env)->GetFieldID(env, ParamNewIoBuffer_cls, "mp_paramNewIoBuffer", "J" );

            ptabParamOutput[ii] = *( (network_paramNewIoBuffer_t*) (intptr_t) (*env)->GetLongField(env, jIoBuff, fid) );
        }
    }
    
    if(error == NETWORK_OK)
    {
        /** create the manager */
        
        pManager = NETWORK_NewManager( recvBufferSize,sendBufferSize,
                                       numberOfInput, ptabParamInput, 
                                       numberOfOutput, ptabParamOutput, &error); 
    }
    
    /** return error */
    jError = error;
    
    return (long) pManager;
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeDeleteManager (JNIEnv *env, jobject obj, jlong jpManager)
{
    /** -- Delete the Manager -- */
    
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    NETWORK_DeleteManager(&pManager);
}

JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerSocketsInit ( JNIEnv *env, jobject obj,
                                            jlong jpManager, jstring jaddr, jint sendingPort,
                                            jint recvPort,jint recvTimeoutSec )
{
    /** -- initialize UDP sockets of sending and receiving the data. -- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    const char *nativeString = (*env)->GetStringUTFChars(env, jaddr, 0);
    eNETWORK_Error error = NETWORK_OK;
    
    error = NETWORK_ManagerSocketsInit( pManager, nativeString, sendingPort,
                                        recvPort, recvTimeoutSec);
    (*env)->ReleaseStringUTFChars( env, jaddr, nativeString );
    
    return error;
}

JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_libnetwork_RunSending_nativeManagerRunSendingThread( JNIEnv *env, jobject obj, jlong jpManager )
{
    /** -- Manage the sending of the data -- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    return (int) NETWORK_ManagerRunSendingThread( pManager );
}

JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_libnetwork_RunReceiving_nativeManagerRunReceivingThread(JNIEnv *env, jobject obj, jlong jpManager)
{
    /** -- Manage the reception of the data -- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    return (int) NETWORK_ManagerRunReceivingThread(pManager);
}

JNIEXPORT void JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerStop(JNIEnv *env, jobject obj, jlong jpManager)
{
    /** -- stop the threads of sending and reception -- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    NETWORK_ManagerStop(pManager);
}

JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerSendData( JNIEnv *env, jobject obj, 
                                                                  jlong jpManager, jint inputBufferId, 
                                                                  jbyteArray jData )
{
    /** -- Add data to send -- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    eNETWORK_Error error = NETWORK_OK;
    uint8_t* pData = (*env)->GetByteArrayElements (env, jData, NULL);
    
    if (NULL != jData)
    {
        error = NETWORK_ManagerSendData(pManager, inputBufferId, pData);
        
        (*env)->ReleaseByteArrayElements (env, jData, pData, JNI_ABORT); // Use JNI_ABORT because we NEVER want to modify jData
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}

JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerSendDeportedData( JNIEnv *env, jobject obj, jlong jpManager,
                                                                          jint inputBufferId, jobject SALData, 
                                                                          jlong jpData, jint dataSize)
{
    /** -- Add data deported to send -- */
    
    /** local declarations */
    jobject dataObj = NULL;
    uint8_t* pData = (uint8_t*) (intptr_t) jpData;
    network_JNI_callbackData_t* pCallbackData = NULL;
    eNETWORK_Error error = NETWORK_OK;
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    /** allocate the data send to the callback */
    pCallbackData = malloc( sizeof(network_JNI_callbackData_t) );
    if(pCallbackData == NULL)
    {
        error = NETWORK_ERROR_ALLOC;
    }
    
    if(error == NETWORK_OK)
    {
        /** create a global reference of the java manager object delete by the callback */
        pCallbackData->jManager = (*env)->NewGlobalRef(env, obj);
        
        /** create a global reference of the java SALnativeData object delete by the callback */
        pCallbackData->jSALData = (*env)->NewGlobalRef(env, SALData);
        
        /** send the data */
        error = NETWORK_ManagerSendDeportedData( pManager, inputBufferId, pData, dataSize, pCallbackData,
                                                 &(JNI_network_deportDatacallback) );
    }
                                             
    return error;
}

JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerReadData(JNIEnv *env, jobject obj, jlong jpManager, jint outputBufferId, jbyteArray jData)
{
    /** -- read data received-- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    eNETWORK_Error error = NETWORK_OK;
    uint8_t* pData = (*env)->GetByteArrayElements (env, jData, NULL);
    
    if (NULL != jData)
    {
        error = NETWORK_ManagerReadData(pManager, outputBufferId, pData);
        
        (*env)->ReleaseByteArrayElements(env, jData, pData, 0);
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}

JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerReadDeportedData( JNIEnv *env, jobject obj, jlong jpManager, jint outputBufferId, jobject jData)
{
    /** -- read data received-- */
    
    /** local declarations */
    jbyteArray jbyteArrayData = NULL;
    jfieldID fid = 0;
    int limitSize = 0;
    uint8_t* pData = NULL;
    int readSize = 0;
    eNETWORK_Error error = NETWORK_OK;
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    /** get the dataRecv class ref */
    jclass dataRecv_cls = (*env)->GetObjectClass(env, jData);
    
    if(dataRecv_cls != NULL)
    {
        /** get m_data */
        fid = (*env)->GetFieldID( env, dataRecv_cls, "m_data", "[B" );
        jbyteArrayData = (*env)->GetObjectField(env, jData, fid);
        
        if(jbyteArrayData == NULL)
        {
            SAL_PRINT(PRINT_ERROR, TAG, "error jbyteArrayData" );
            error = NETWORK_ERROR_BAD_PARAMETER; 
        }
    }

    if(error == NETWORK_OK)
    {
        fid = (*env)->GetFieldID(env, dataRecv_cls, "m_limitSize", "I" );
        limitSize = (*env)->GetIntField(env, jData, fid);
   
        pData = (*env)->GetByteArrayElements (env, jbyteArrayData, NULL);
        
        NETWORK_ManagerReadDeportedData( pManager, outputBufferId, pData, limitSize, &readSize );
        
        fid = (*env)->GetFieldID(env, dataRecv_cls, "m_readSize", "I" );
        (*env)->SetIntField(env, jData, fid, readSize);
        
        (*env)->ReleaseByteArrayElements( env, jbyteArrayData, pData, 0 );
    }
    
    /** delete class ref */
    (*env)->DeleteLocalRef (env, dataRecv_cls);
    
    return error;
}

JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerReadDataWithTimeout( JNIEnv *env, jobject obj, jlong jpManager, 
                                                                                 jint outputBufferId, 
                                                                                 jbyteArray jData,
                                                                                 jint timeoutMs )
{
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    return 0 ;
}


JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerReadDeportedDataWithTimeout( JNIEnv *env, jobject obj, jlong jpManager, 
                                                                                         jint outputBufferId,
                                                                                         jbyteArray jData, 
                                                                                         jint dataLimitSize, 
                                                                                         jint* pReadSize,
                                                                                         jint timeoutMs )
{
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    return 0 ;
}

int JNI_network_deportDatacallback (int IoBufferId, uint8_t* pData, void* pCustomData, int status)
{
    /** callback */
    
    /** local declarations */
    JNIEnv* env = NULL;
    eNETWORK_Error error = NETWORK_OK;
    eNETWORK_CALLBACK_RETURN callbackReturn = 0;
    int retry = 0;
    jclass manager_cls =  NULL;
    jmethodID jmCallback = 0;
    network_JNI_callbackData_t* pCallbackData = (network_JNI_callbackData_t*) pCustomData;

    /** get the environment */
	if (g_vm != NULL)
	{
		(*g_vm)->GetEnv(g_vm, (void **)&env, JNI_VERSION_1_6);
	}

	if (env != NULL && 
        pCallbackData != NULL &&
        pCallbackData->jManager != NULL ) 
    {
        /** send the callback java of the java manager */
        
        /** get the manager class reference */
        manager_cls = (*env)->GetObjectClass(env, pCallbackData->jManager);
        jmCallback = (*env)->GetMethodID(env, manager_cls, "callback", "(ILcom/parrot/arsdk/libsal/SALNativeData;I)I" );
        
        /** free the local reference on the class manager */
        (*env)->DeleteLocalRef(env, manager_cls);
        
        callbackReturn = (*env)->CallIntMethod(env, pCallbackData->jManager, jmCallback, IoBufferId, pCallbackData->jSALData, status); 

        switch(status)
        {
            case NETWORK_CALLBACK_STATUS_SENT :
            case NETWORK_CALLBACK_STATUS_SENT_WITH_ACK :
            case NETWORK_CALLBACK_STATUS_FREE :
            
                freeCallbackData (env, pCallbackData);
                
            break;
            
            
            case NETWORK_CALLBACK_STATUS_TIMEOUT :
            
                
            break;
            
            default:
            
            break;
        }
	}
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER; 
    }
    
    return callbackReturn;
}

void freeCallbackData (JNIEnv* env, network_JNI_callbackData_t* pCallbackData/*, uint8_t* pData*/)
{
    /** -- free the global references of the callback -- */
    
    /** Release the data[] */
    //(*env)->ReleaseByteArrayElements(env, pCallbackData->jDataObj, pData, 0);
    
    /** delete the data object reference */
    //(*env)->DeleteGlobalRef( env, pCallbackData->jDataObj );
        
    /** delete the java SALnativeData object reference */
    (*env)->DeleteGlobalRef( env, pCallbackData->jSALData );
    
    /** delete the java manager object reference */
    (*env)->DeleteGlobalRef( env, pCallbackData->jManager );
    
    /** the callback data */
    free(pCallbackData);
}
