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

/**
 *  @brief data sent to the callbak 
**/
typedef struct network_JNI_callbackData_t
{
    jobject jManager; /**< manager sent the data */
    jobject jSALData; /**< java native data*/
}network_JNI_callbackData_t;

/*****************************************
 * 
 *             private header:
 *
******************************************/

/**
 *  @brief call back use when the data are sent or have a timeout 
 *  @param[in] IoBufferId identifier of the IoBuffer is calling back
 *  @param[in] pData pointer on the data
 *  @param[in] pCustomData pointer on a custom data
 *  @param[in] status indicating the reason of the callback. eNETWORK_CALLBACK_STATUS type
 *  @return eNETWORK_CALLBACK_RETURN
 *  @see eNETWORK_CALLBACK_STATUS
**/
eNETWORK_CALLBACK_RETURN JNI_network_deportDatacallback(int IoBufferId, 
                                                            uint8_t* pData, 
                                                            void* pCustomData, 
                                                            eNETWORK_CALLBACK_STATUS status);

/**
 *  @brief free the global references used by the callback
 *  @param[in] env java environment
 *  @param[in] ppCallbackData address of the pointer on the callbackData storing the global references
 *  @warning this funtion free the callbackData and set ppCallbackData to NULL
**/
void freeCallbackData (JNIEnv* env, network_JNI_callbackData_t** ppCallbackData);

/*****************************************
 * 
 *             implementation :
 *
******************************************/

static const char* TAG = "APP"; /** tag used by the print of the file */
JavaVM* g_vm = NULL; /** reference to the java virtual machine */

/**
 *  @brief save the reference to the java virtual machine
 *  @note this function is automatically call on the JNI startup
 *  @param[in] vm reference to the java virtual machine
 *  @param[in] reserved data reserved
 *  @return JNI version
**/
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
    SAL_PRINT(PRINT_DEBUG, TAG, "Library has been loaded");

	/** Saving the reference to the java virtual machine */
	g_vm = vm;

	/** Return the JNI version */
	return JNI_VERSION_1_6;
}


/**
 *  @brief Create a new manager
 *  @warning This function allocate memory
 *  @post NETWORK_ManagerSocketsInit() must be called to initialize the sockets, indicate on which address send the data, the sending port the receiving  port and the timeout. 
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param[in] recvBufferSize size in byte of the receiving buffer. ideally must be equal to the sum of the sizes of one data of all output buffers
 *  @param[in] sendBufferSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 *  @param[in] numberOfInput Number of input buffer
 *  @param[in] inputArray array of the parameters of creation of the inputs. The table must contain as many parameters as the number of input buffer.
 *  @param[in] numberOfOutput Number of output buffer
 *  @param[in] outputArray array of the parameters of creation of the outputs. The table must contain as many parameters as the number of output buffer.
 *  @return Pointer on the network_manager_t.
 *  @note This creator adds for all output, one other inOutBuffer for storing the acknowledgment to return.
 * These new buffers are added in the input and output buffer tables.
 *  @warning The identifiers of the IoBuffer should not exceed the value 128.
 *  @see Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeDeleteManager()
 * 
**/
JNIEXPORT jlong JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeNewManager ( JNIEnv *env, jobject obj, 
                                                                  jint recvBufferSize,
                                                                  jint sendBufferSize,
                                                                  jint numberOfInput, 
                                                                  jobjectArray inputArray, 
                                                                  jint numberOfOutput,
                                                                  jobjectArray outputArray)
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
    
    /** print error */
    if(error == NETWORK_OK)
    {
        SAL_PRINT(PRINT_ERROR, TAG," error: %d occurred \n", error );
    }
    
    return (long) pManager;
}

/**
 *  @brief Delete the Manager
 *  @warning This function free memory
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @see Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeNewManager()
**/
JNIEXPORT void JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeDeleteManager (JNIEnv *env, jobject obj, jlong jpManager)
{
    /** -- Delete the Manager -- */
    
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    NETWORK_DeleteManager(&pManager);
}

/**
 *  @brief initialize UDP sockets of sending and receiving the data.
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @param[in] jaddr address of connection at which the data will be sent.
 *  @param[in] sendingPort port on which the data will be sent.
 *  @param[in] recvPort port on which the data will be received.
 *  @param[in] recvTimeoutSec timeout in seconds set on the socket to limit the time of blocking of the function NETWORK_ReceiverRead().
 *  @return error equal to NETWORK_OK if the Bind if successful otherwise see eNETWORK_Manager_Error.
**/
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

/**
 *  @brief Manage the sending of the data 
 *  @warning This function must be called by a specific thread.
 *  @pre The sockets must be initialized through nativeManagerSocketsInit().
 *  @post Before join the thread calling this function, nativeManagerStop() must be called.
 *  @note This function send the data stored in the input buffer through NETWORK_ManagerSendData().
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @return NULL
 *  @see Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerSocketsInit()
 *  @see Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerStop()
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_libnetwork_RunSending_nativeManagerRunSendingThread( JNIEnv *env, jobject obj, jlong jpManager )
{
    /** -- Manage the sending of the data -- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    return (int) NETWORK_ManagerRunSendingThread( pManager );
}

/**
 *  @brief Manage the reception of the data.
 *  @warning This function must be called by a specific thread.
 *  @pre The socket of the receiver must be initialized through nativeManagerSocketsInit().
 *  @post Before join the thread calling this function, nativeManagerStop() must be called.
 *  @note This function receives the data through NETWORK_ManagerReadData() and stores them in the output buffers according to their parameters.
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @return NULL
 *  @see Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerSocketsInit()
 *  @see Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerStop()
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_libnetwork_RunReceiving_nativeManagerRunReceivingThread(JNIEnv *env, jobject obj, jlong jpManager)
{
    /** -- Manage the reception of the data -- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    return (int) NETWORK_ManagerRunReceivingThread(pManager);
}

/**
 *  @brief stop the threads of sending and reception
 *  @details Used to kill the threads calling nativeManagerRunSendingThread() and nativeManagerRunReceivingThread().
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @see Java_com_parrot_arsdk_libnetwork_RunSending_nativeManagerRunSendingThread()
 *  @see Java_com_parrot_arsdk_libnetwork_RunReceiving_nativeManagerRunReceivingThread()
**/
JNIEXPORT void JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerStop(JNIEnv *env, jobject obj, jlong jpManager)
{
    /** -- stop the threads of sending and reception -- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    NETWORK_ManagerStop(pManager);
}


/**
 *  @brief Flush all buffers of the network manager
 *  @param pManager pointer on the Manager
 *  @return error eNETWORK_Error
**/
JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerFlush(JNIEnv *env, jobject obj, 
                                                                   jlong jpManager)
{
    /** -- Flush all buffers of the network manager -- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    return NETWORK_ManagerFlush(pManager);
}

/**
 *  @brief Add data to send
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @param[in] inputBufferId identifier of the input buffer in which the data must be stored
 *  @param[in] jData array of byte to send
 *  @return error eNETWORK_Error
**/
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

/**
 *  @brief Add deported data to send
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @param[in] inputBufferId identifier of the input buffer in which the data must be stored
 *  @param[in] SALData SALNativeData to send
 *  @param[in] jpData array of byte to send ( use salData.getData() )
 *  @param[in] dataSize size of the data to send ( use salData.getDataSize() )
 *  @return error eNETWORK_Error
**/
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

/**
 *  @brief Read data received
 *  @warning the outputBuffer should not be deportedData type
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @param[in] outputBufferId identifier of the output buffer in which the data must be read
 *  @param[out] jData array of byte to store the data received
 *  @return error eNETWORK_Error
**/
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

/**
 *  @brief Read deported data received
 *  @warning the outputBuffer must be deportedData type
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @param[in] outputBufferId identifier of the output buffer in which the data must be read
 *  @param[out] jData NetworkDataRecv to store the data received
 *  @return error eNETWORK_Error type
**/
JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerReadDeportedData( JNIEnv *env, jobject obj, jlong jpManager, jint outputBufferId, jobject jData)
{
    /** -- read deported data received -- */
    
    /** local declarations */
    jbyteArray jbyteArrayData = NULL;
    jfieldID fid = 0;
    int limitSize = 0;
    uint8_t* pData = NULL;
    int readSize = 0;
    eNETWORK_Error error = NETWORK_OK;
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    
    /** get the NetworkDataRecv class reference */
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

/**
 *  @brief Read data received with timeout
 *  @details This function is blocking
 *  @warning the outputBuffer should not be deportedData type
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @param[in] outputBufferId identifier of the output buffer in which the data must be read
 *  @param[out] jData array of byte to store the data received
 *  @param[in] timeoutMs maximum time in millisecond to wait if there is no data to read
 *  @return error eNETWORK_Error
**/
JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerReadDataWithTimeout(JNIEnv *env,
                                                                                 jobject obj,
                                                                                 jlong jpManager, 
                                                                                 jint outputBufferId, 
                                                                                 jbyteArray jData,
                                                                                 jint timeoutMs )
{
    /** -- read data received with timeout-- */
    
    /** local declarations */
    network_manager_t* pManager = (network_manager_t*) (intptr_t) jpManager;
    eNETWORK_Error error = NETWORK_OK;
    uint8_t* pData = (*env)->GetByteArrayElements (env, jData, NULL);
    
    if (NULL != jData)
    {
        error = NETWORK_ManagerReadDataWithTimeout( pManager, outputBufferId, pData, timeoutMs );
        
        (*env)->ReleaseByteArrayElements(env, jData, pData, 0);
    }
    else
    {
        error = NETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}

/**
 *  @brief Read deported data received with timeout
 *  @warning the outputBuffer must be deportedData type
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jpManager adress of the network_manager_t
 *  @param[in] outputBufferId identifier of the output buffer in which the data must be read
 *  @param[out] jData NetworkDataRecv to store the data received
 *  @param[in] timeoutMs maximum time in millisecond to wait if there is no data to read
 *  @return error eNETWORK_Error type
**/
JNIEXPORT int JNICALL
Java_com_parrot_arsdk_libnetwork_NetworkManager_nativeManagerReadDeportedDataWithTimeout( JNIEnv *env,
                                                                                          jobject obj,
                                                                                          jlong jpManager, 
                                                                                          jint outputBufferId,
                                                                                          jobject jData,
                                                                                          jint timeoutMs )
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
    
    /** get the dataRecv class reference */
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
        
        error = NETWORK_ManagerReadDeportedDataWithTimeout( pManager, outputBufferId, 
                                                            pData, limitSize, 
                                                            &readSize, timeoutMs );
        
        fid = (*env)->GetFieldID(env, dataRecv_cls, "m_readSize", "I" );
        (*env)->SetIntField(env, jData, fid, readSize);
        
        (*env)->ReleaseByteArrayElements( env, jbyteArrayData, pData, 0 );
    }
    
    /** delete class ref */
    (*env)->DeleteLocalRef (env, dataRecv_cls);
    
    return error;
}

eNETWORK_CALLBACK_RETURN JNI_network_deportDatacallback(int IoBufferId, 
                                                           uint8_t* pData, 
                                                           void* pCustomData,
                                                           eNETWORK_CALLBACK_STATUS status)
{
    /** callback */
    
    /** local declarations */
    JNIEnv* env = NULL;
    eNETWORK_Error error = NETWORK_OK;
    eNETWORK_CALLBACK_RETURN callbackReturn = NETWORK_CALLBACK_RETURN_DEFAULT;
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
            
                freeCallbackData (env, &pCallbackData);
                
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

void freeCallbackData (JNIEnv* env, network_JNI_callbackData_t** ppCallbackData)
{
    /** -- free the global references of the callback -- */
    
    /** local declarations */
    network_JNI_callbackData_t* pCallbackData = NULL;
    
    if(ppCallbackData != NULL)
    {
        pCallbackData = *ppCallbackData;
        
        if(pCallbackData != NULL)
        {
            /** delete the java SALnativeData object reference */
            (*env)->DeleteGlobalRef( env, pCallbackData->jSALData );
            
            /** delete the java manager object reference */
            (*env)->DeleteGlobalRef( env, pCallbackData->jManager );
            
            /** the callback data */
            free(pCallbackData);
            pCallbackData = NULL;
        }
        *ppCallbackData = NULL;
    }
}
