/**
 *  @file ARNETWORK_JNIManager.c
 *  @brief JNI between the ARNETWORK_Manager.h and ARNETWORK_Manager.java
 *  @date 01/18/2013
 *  @author maxime.maitre@parrot.com
**/

/*****************************************
 * 
 *             include file :
 *
******************************************/

#include <jni.h>
#include <stdlib.h>

#include <libARSAL/ARSAL_Print.h>

#include <libARNetwork/ARNETWORK_Error.h>
#include <libARNetwork/ARNETWORK_Frame.h>
#include <libARNetwork/ARNETWORK_Manager.h>

/**
 *  @brief data sent to the callbak 
**/
typedef struct
{
    jobject jManager; /**< manager sent the data */
    jobject jARData; /**< java native data*/
}ARNETWORK_JNIManager_CallbackData_t;

/*****************************************
 * 
 *             private header:
 *
******************************************/

#define ARNETWORK_JNIMANGER_TAG "JNIManager" /** tag used by the print of the file */

/**
 *  @brief call back use when the data are sent or have a timeout 
 *  @param[in] IOBufferID identifier of the IoBuffer is calling back
 *  @param[in] dataPtr pointer on the data
 *  @param[in] customData pointer on a custom data
 *  @param[in] status indicating the reason of the callback. eARNETWORK_MANAGER_CALLBACK_STATUS type
 *  @return eARNETWORK_MANAGER_CALLBACK_RETURN
 *  @see eARNETWORK_MANAGER_CALLBACK_STATUS
**/
eARNETWORK_MANAGER_CALLBACK_RETURN ARNETWORK_JNIManger_Callback_t(int IOBufferID,  uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status);

/**
 *  @brief free the global references used by the callback
 *  @param[in] env java environment
 *  @param[in] callbackDataPtrAddr address of the pointer on the callbackData storing the global references
 *  @warning this funtion free the callbackData and set callbackDataPtrAddr to NULL
**/
void ARNETWORK_JNIManager_FreeCallbackData(JNIEnv *env, ARNETWORK_JNIManager_CallbackData_t **callbackDataPtrAddr);

/*****************************************
 * 
 *             implementation :
 *
******************************************/


JavaVM* gARNETWORK_JNIManager_VM = NULL; /** reference to the java virtual machine */

/**
 *  @brief save the reference to the java virtual machine
 *  @note this function is automatically call on the JNI startup
 *  @param[in] VM reference to the java virtual machine
 *  @param[in] reserved data reserved
 *  @return JNI version
**/
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *VM, void *reserved)
{
    ARSAL_PRINT(ARSAL_PRINT_DEBUG, ARNETWORK_JNIMANGER_TAG, "Library has been loaded");
    
    /** Saving the reference to the java virtual machine */
    gARNETWORK_JNIManager_VM = VM;
    
    /** Return the JNI version */
    return JNI_VERSION_1_6;
}

/**
 *  @brief Create a new manager
 *  @warning This function allocate memory
 *  @post ARNETWORK_Manager_SocketsInit() must be called to initialize the sockets, indicate on which address send the data, the sending port the receiving  port and the timeout. 
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param[in] recvBufferSize size in byte of the receiving buffer. ideally must be equal to the sum of the sizes of one data of all output buffers
 *  @param[in] sendBufferSize size in byte of the sending buffer. ideally must be equal to the sum of the sizes of one data of all input buffers
 *  @param[in] numberOfInput Number of input buffer
 *  @param[in] inputParamArray array of the parameters of creation of the inputs. The array must contain as many parameters as the number of input buffer.
 *  @param[in] numberOfOutput Number of output buffer
 *  @param[in] outputParamArray array of the parameters of creation of the outputs. The array must contain as many parameters as the number of output buffer.
 *  @return Pointer on the ARNETWORK_Manager_t.
 *  @note This creator adds for all output, one other IOBuffer for storing the acknowledgment to return.
 * These new buffers are added in the input and output buffer arrays.
 *  @warning The identifiers of the IoBuffer should not exceed the value 128.
 *  @see Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeDelete()
 * 
**/
JNIEXPORT jlong JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeNew(JNIEnv *env, jobject obj, jint recvBufferSize, jint sendBufferSize, jint numberOfInput, jobjectArray inputParamArray, jint numberOfOutput, jobjectArray outputParamArray)
{
    /** -- Create a new manager -- */
    
    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = NULL;
    ARNETWORK_IOBufferParam_t* pArrParamInput = NULL;
    ARNETWORK_IOBufferParam_t* pArrParamOutput = NULL;
    int ii = 0;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    jfieldID fieldID;
    jclass IOBufferParam_cls;
    jobject jIOBuffer ;
    
    /** get the class IOBufferParam */
    IOBufferParam_cls = (*env)->FindClass(env, "com/parrot/arsdk/arnetwork/ARNetworkIOBufferParam");
    if (NULL == IOBufferParam_cls)
    {
        error = ARNETWORK_ERROR_ALLOC;
    }
    
    if(error == ARNETWORK_OK)
    {
        /** allocate the input parameters C */
        
        pArrParamInput = malloc ( sizeof(ARNETWORK_IOBufferParam_t) * numberOfInput );
        if(pArrParamInput == NULL && numberOfInput != 0 )
        {
            error = ARNETWORK_ERROR_ALLOC;
        }
    }
  
    if(error == ARNETWORK_OK)
    {
        /** copy the parameters java to the input parameters C*/
        
        for(ii = 0; ii < numberOfInput; ++ii)
        {
            /** get obj */
            jIOBuffer = (*env)->GetObjectArrayElement(env, inputParamArray, ii);
            fieldID = (*env)->GetFieldID(env, IOBufferParam_cls, "m_IOBufferParamPtr", "J" );
            
            pArrParamInput[ii] = *( (ARNETWORK_IOBufferParam_t*) (intptr_t) (*env)->GetLongField(env, jIOBuffer, fieldID) );
        }
    }
   
    if(error == ARNETWORK_OK)
    {
        /** allocate the output parameters C */
        
        pArrParamOutput = malloc ( sizeof(ARNETWORK_IOBufferParam_t) * numberOfOutput );
        if(pArrParamOutput == NULL && numberOfInput != 0 )
        {
            error = ARNETWORK_ERROR_ALLOC;
        }
    }
  
    if(error == ARNETWORK_OK)
    {
        /** copy the parameters java to the output parameters C*/
        
        for(ii = 0; ii<  numberOfOutput; ++ii)
        {
            /** get obj */
            jIOBuffer = (*env)->GetObjectArrayElement(env, outputParamArray, ii);
            fieldID = (*env)->GetFieldID(env, IOBufferParam_cls, "m_IOBufferParamPtr", "J");

            pArrParamOutput[ii] = *( (ARNETWORK_IOBufferParam_t*) (intptr_t) (*env)->GetLongField(env, jIOBuffer, fieldID) );
        }
    }
    
    if(error == ARNETWORK_OK)
    {
        /** create the manager */
        
        managerPtr = ARNETWORK_Manager_New(recvBufferSize, sendBufferSize, numberOfInput, pArrParamInput, numberOfOutput, pArrParamOutput, &error); 
    }
    
    /** print error */
    if(error == ARNETWORK_OK)
    {
        ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_JNIMANGER_TAG, " error: %d occurred \n", error);
    }
    
    return (long) managerPtr;
}

/**
 *  @brief Delete the Manager
 *  @warning This function free memory
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @see Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeNew()
**/
JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeDelete(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    /** -- Delete the Manager -- */
    
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    ARNETWORK_Manager_Delete(&managerPtr);
}

/**
 *  @brief initialize UDP sockets of sending and receiving the data.
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @param[in] jaddr address of connection at which the data will be sent.
 *  @param[in] sendingPort port on which the data will be sent.
 *  @param[in] recvPort port on which the data will be received.
 *  @param[in] recvTimeoutSec timeout in seconds set on the socket to limit the time of blocking of the function ARNETWORK_Receiver_Read().
 *  @return error equal to ARNETWORK_OK if the Bind if successful otherwise see eARNETWORK_ERROR.
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeSocketsInit(JNIEnv *env, jobject obj, jlong jManagerPtr, jstring jaddr, jint sendingPort, jint recvPort, jint recvTimeoutSec)
{
    /** -- initialize UDP sockets of sending and receiving the data. -- */
    
    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    const char *nativeString = (*env)->GetStringUTFChars(env, jaddr, 0);
    eARNETWORK_ERROR error = ARNETWORK_OK;
    
    error = ARNETWORK_Manager_SocketsInit(managerPtr, nativeString, sendingPort, recvPort, recvTimeoutSec);
    (*env)->ReleaseStringUTFChars( env, jaddr, nativeString );
    
    return error;
}

/**
 *  @brief Manage the sending of the data 
 *  @warning This function must be called by a specific thread.
 *  @pre The sockets must be initialized through nativeSocketsInit().
 *  @post Before join the thread calling this function, nativeStop() must be called.
 *  @note This function send the data stored in the input buffer through ARNETWORK_Manager_SendFixedSizeData().
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @return NULL
 *  @see Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeSocketsInit()
 *  @see Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeStop()
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_SendingRunnable_nativeSendingThreadRun(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    /** -- Manage the sending of the data -- */
    
    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    
    return (int) ARNETWORK_Manager_SendingThreadRun( managerPtr );
}

/**
 *  @brief Manage the reception of the data.
 *  @warning This function must be called by a specific thread.
 *  @pre The socket of the receiver must be initialized through nativeSocketsInit().
 *  @post Before join the thread calling this function, nativeStop() must be called.
 *  @note This function receives the data through ARNETWORK_Manager_ReadFixedSizeData() and stores them in the output buffers according to their parameters.
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @return NULL
 *  @see Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeSocketsInit()
 *  @see Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeStop()
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ReceivingRunnable_nativeReceivingThreadRun(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    /** -- Manage the reception of the data -- */
    
    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    
    return (int) ARNETWORK_Manager_ReceivingThreadRun(managerPtr);
}

/**
 *  @brief stop the threads of sending and reception
 *  @details Used to kill the threads calling nativeSendingThreadRun() and nativeReceivingThreadRun().
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @see Java_com_parrot_arsdk_arnetwork_SendingRunnable_nativeSendingThreadRun()
 *  @see Java_com_parrot_arsdk_arnetwork_ReceivingRunnable_nativeReceivingThreadRun()
**/
JNIEXPORT void JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeStop(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    /** -- stop the threads of sending and reception -- */
    
    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    
    ARNETWORK_Manager_Stop(managerPtr);
}

/**
 *  @brief Flush all buffers of the network manager
 *  @param managerPtr pointer on the Manager
 *  @return error eARNETWORK_ERROR
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeFlush(JNIEnv *env, jobject obj, jlong jManagerPtr)
{
    /** -- Flush all buffers of the network manager -- */
    
    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    
    return ARNETWORK_Manager_Flush(managerPtr);
}

/**
 *  @brief Add data to send in a IOBuffer using fixed size data
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @param[in] inputBufferID identifier of the input buffer in which the data must be stored
 *  @param[in] jData array of byte to send
 *  @return error eARNETWORK_ERROR
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeSendFixedSizeData( JNIEnv *env, jobject obj, jlong jManagerPtr, jint inputBufferID, jbyteArray jData )
{
    /** -- Add data to send in a IOBuffer using fixed size data -- */
    
    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    uint8_t *dataPtr = (*env)->GetByteArrayElements (env, jData, NULL);
    
    if (NULL != jData)
    {
        error = ARNETWORK_Manager_SendFixedSizeData(managerPtr, inputBufferID, dataPtr);
        
        (*env)->ReleaseByteArrayElements (env, jData, dataPtr, JNI_ABORT); // Use JNI_ABORT because we NEVER want to modify jData
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}

/**
 *  @brief Add data to send in a IOBuffer using variable size data
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @param[in] inputBufferID identifier of the input buffer in which the data must be stored
 *  @param[in] ARData ARNativeData to send
 *  @param[in] jdataPtr array of byte to send ( use arData.getData() )
 *  @param[in] dataSize size of the data to send ( use arData.getDataSize() )
 *  @return error eARNETWORK_ERROR
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeSendVariableSizeData(JNIEnv *env, jobject obj, jlong jManagerPtr, jint inputBufferID, jobject ARData, jlong jdataPtr, jint dataSize)
{
    /** -- Add data to send in a IOBuffer using variable size data -- */
    
    /** local declarations */
    jobject dataObj = NULL;
    uint8_t *dataPtr = (uint8_t*) (intptr_t) jdataPtr;
    ARNETWORK_JNIManager_CallbackData_t *callbackDataPtr = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    
    /** allocate the data send to the callback */
    callbackDataPtr = malloc( sizeof(ARNETWORK_JNIManager_CallbackData_t) );
    if(callbackDataPtr == NULL)
    {
        error = ARNETWORK_ERROR_ALLOC;
    }
    
    if(error == ARNETWORK_OK)
    {
        /** create a global reference of the java manager object delete by the callback */
        callbackDataPtr->jManager = (*env)->NewGlobalRef(env, obj);
        
        /** create a global reference of the java ARnativeData object delete by the callback */
        callbackDataPtr->jARData = (*env)->NewGlobalRef(env, ARData);
        
        /** send the data */
        error = ARNETWORK_Manager_SendVariableSizeData( managerPtr, inputBufferID, dataPtr, dataSize, callbackDataPtr, &(ARNETWORK_JNIManger_Callback_t) );
    }
    
    return error;
}

/**
 *  @brief Read data received in a IOBuffer using fixed size data
 *  @warning the outputBuffer should not be using of variable Size Data type
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @param[in] outputBufferID identifier of the output buffer in which the data must be read
 *  @param[out] jData array of byte to store the data received
 *  @return error eARNETWORK_ERROR
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeReadFixedSizeData(JNIEnv *env, jobject obj, jlong jManagerPtr, jint outputBufferID, jbyteArray jData)
{
    /** -- Read data received in a IOBuffer using fixed size data -- */
    
    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    uint8_t *dataPtr = (*env)->GetByteArrayElements (env, jData, NULL);
    
    if (NULL != jData)
    {
        error = ARNETWORK_Manager_ReadFixedSizeData(managerPtr, outputBufferID, dataPtr);
        
        (*env)->ReleaseByteArrayElements(env, jData, dataPtr, 0);
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}

/**
 *  @brief Read data received in a IOBuffer using variable size data
 *  @warning the outputBuffer must be using of variable Size Data type
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @param[in] outputBufferID identifier of the output buffer in which the data must be read
 *  @param[out] jData ARNetworkDataRecv to store the data received
 *  @return error eARNETWORK_ERROR type
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeReadVarableSizeData(JNIEnv *env, jobject obj, jlong jManagerPtr, jint outputBufferID, jobject jData)
{
    /** -- Read data received in a IOBuffer using variable size data -- */
    
    /** local declarations */
    jbyteArray jbyteArrayData = NULL;
    jfieldID fieldID = 0;
    int limitSize = 0;
    uint8_t *dataPtr = NULL;
    int readSize = 0;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    
    /** get the ARNetworkDataRecv class reference */
    jclass dataRecv_cls = (*env)->GetObjectClass(env, jData);
    
    if(dataRecv_cls != NULL)
    {
        /** get m_data */
        fieldID = (*env)->GetFieldID( env, dataRecv_cls, "m_data", "[B" );
        jbyteArrayData = (*env)->GetObjectField(env, jData, fieldID);
        
        if(jbyteArrayData == NULL)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_JNIMANGER_TAG, "error jbyteArrayData" );
            error = ARNETWORK_ERROR_BAD_PARAMETER; 
        }
    }
    
    if(error == ARNETWORK_OK)
    {
        fieldID = (*env)->GetFieldID(env, dataRecv_cls, "m_limitSize", "I" );
        limitSize = (*env)->GetIntField(env, jData, fieldID);
   
        dataPtr = (*env)->GetByteArrayElements (env, jbyteArrayData, NULL);
        
        ARNETWORK_Manager_ReadVariableSizeData( managerPtr, outputBufferID, dataPtr, limitSize, &readSize );
        
        fieldID = (*env)->GetFieldID(env, dataRecv_cls, "m_readSize", "I" );
        (*env)->SetIntField(env, jData, fieldID, readSize);
        
        (*env)->ReleaseByteArrayElements( env, jbyteArrayData, dataPtr, 0 );
    }
    
    /** delete class ref */
    (*env)->DeleteLocalRef (env, dataRecv_cls);
    
    return error;
}

/**
 *  @brief Read, with timeout, a data received in IOBuffer using fixed size data
 *  @details This function is blocking
 *  @warning the outputBuffer should not be using of variable Size Data type
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @param[in] outputBufferID identifier of the output buffer in which the data must be read
 *  @param[out] jData array of byte to store the data received
 *  @param[in] timeoutMs maximum time in millisecond to wait if there is no data to read
 *  @return error eARNETWORK_ERROR
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeReadFixedSizeDataWithTimeout(JNIEnv *env, jobject obj, jlong jManagerPtr, jint outputBufferID, jbyteArray jData, jint timeoutMs)
{
    /** -- Read, with timeout, a data received in IOBuffer using fixed size data -- */
    
    /** local declarations */
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    uint8_t *dataPtr = (*env)->GetByteArrayElements (env, jData, NULL);
    
    if (NULL != jData)
    {
        error = ARNETWORK_Manager_ReadFixedSizeDataWithTimeout( managerPtr, outputBufferID, dataPtr, timeoutMs );
        
        (*env)->ReleaseByteArrayElements(env, jData, dataPtr, 0);
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER;
    }
    
    return error;
}

/**
 *  @brief Read, with timeout, a data received in IOBuffer using variable size data
 *  @warning the outputBuffer must be using of variable Size Data type
 *  @param env reference to the java environment
 *  @param obj reference to the object calling this function
 *  @param jManagerPtr adress of the ARNETWORK_Manager_t
 *  @param[in] outputBufferID identifier of the output buffer in which the data must be read
 *  @param[out] jData ARNetworkDataRecv to store the data received
 *  @param[in] timeoutMs maximum time in millisecond to wait if there is no data to read
 *  @return error eARNETWORK_ERROR type
**/
JNIEXPORT jint JNICALL
Java_com_parrot_arsdk_arnetwork_ARNetworkManager_nativeReadVarableSizeDataWithTimeout(JNIEnv *env, jobject obj, jlong jManagerPtr, jint outputBufferID, jobject jData, jint timeoutMs)
{
    /** -- Read, with timeout, a data received in IOBuffer using variable size data -- */
    
    /** local declarations */
    jbyteArray jbyteArrayData = NULL;
    jfieldID fieldID = 0;
    int limitSize = 0;
    uint8_t *dataPtr = NULL;
    int readSize = 0;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    ARNETWORK_Manager_t *managerPtr = (ARNETWORK_Manager_t*) (intptr_t) jManagerPtr;
    
    /** get the dataRecv class reference */
    jclass dataRecv_cls = (*env)->GetObjectClass(env, jData);
    
    if(dataRecv_cls != NULL)
    {
        /** get m_data */
        fieldID = (*env)->GetFieldID( env, dataRecv_cls, "m_data", "[B" );
        jbyteArrayData = (*env)->GetObjectField(env, jData, fieldID);
        
        if(jbyteArrayData == NULL)
        {
            ARSAL_PRINT(ARSAL_PRINT_ERROR, ARNETWORK_JNIMANGER_TAG, "error jbyteArrayData" );
            error = ARNETWORK_ERROR_BAD_PARAMETER; 
        }
    }
    
    if(error == ARNETWORK_OK)
    {
        fieldID = (*env)->GetFieldID(env, dataRecv_cls, "m_limitSize", "I" );
        limitSize = (*env)->GetIntField(env, jData, fieldID);
   
        dataPtr = (*env)->GetByteArrayElements (env, jbyteArrayData, NULL);
        
        error = ARNETWORK_Manager_ReadVariableSizeDataWithTimeout( managerPtr, outputBufferID, dataPtr, limitSize, &readSize, timeoutMs );
        
        fieldID = (*env)->GetFieldID(env, dataRecv_cls, "m_readSize", "I" );
        (*env)->SetIntField(env, jData, fieldID, readSize);
        
        (*env)->ReleaseByteArrayElements( env, jbyteArrayData, dataPtr, 0 );
    }
    
    /** delete class ref */
    (*env)->DeleteLocalRef (env, dataRecv_cls);
    
    return error;
}

eARNETWORK_MANAGER_CALLBACK_RETURN ARNETWORK_JNIManger_Callback_t(int IOBufferID, uint8_t *dataPtr, void *customData, eARNETWORK_MANAGER_CALLBACK_STATUS status)
{
    /** callback */
    
    /** local declarations */
    JNIEnv* env = NULL;
    eARNETWORK_ERROR error = ARNETWORK_OK;
    eARNETWORK_MANAGER_CALLBACK_RETURN callbackReturn = ARNETWORK_MANAGER_CALLBACK_RETURN_DEFAULT;
    int retry = 0;
    jclass managerCLS =  NULL;
    jmethodID jMethodCallback = 0;
    ARNETWORK_JNIManager_CallbackData_t *callbackDataPtr = (ARNETWORK_JNIManager_CallbackData_t*) customData;
    
    /** get the environment */
    if (gARNETWORK_JNIManager_VM != NULL)
    {
        (*gARNETWORK_JNIManager_VM)->GetEnv(gARNETWORK_JNIManager_VM, (void **)&env, JNI_VERSION_1_6);
    }
    
    /** check parameters:
     *  -   env is not null
     *  -   callbackDataPtr is not null
     *  -   jManager is not null
    **/
    if(env != NULL && 
        callbackDataPtr != NULL &&
        callbackDataPtr->jManager != NULL) 
    {
        /** send the callback java of the java manager */
        
        /** get the manager class reference */
        managerCLS = (*env)->GetObjectClass(env, callbackDataPtr->jManager);
        jMethodCallback = (*env)->GetMethodID(env, managerCLS, "callback", "(ILcom/parrot/arsdk/arsal/ARNativeData;I)I" );
        
        /** free the local reference on the class manager */
        (*env)->DeleteLocalRef(env, managerCLS);
        
        callbackReturn = (*env)->CallIntMethod(env, callbackDataPtr->jManager, jMethodCallback, IOBufferID, callbackDataPtr->jARData, status); 

        switch(status)
        {
            case ARNETWORK_MANAGER_CALLBACK_STATUS_SENT :
            case ARNETWORK_MANAGER_CALLBACK_STATUS_SENT_WITH_ACK :
            case ARNETWORK_MANAGER_CALLBACK_STATUS_FREE :
                
                ARNETWORK_JNIManager_FreeCallbackData (env, &callbackDataPtr);
                
                break;
            
            case ARNETWORK_MANAGER_CALLBACK_STATUS_TIMEOUT :
            
                break;
            
            default:
                
                break;
        }
    }
    else
    {
        error = ARNETWORK_ERROR_BAD_PARAMETER; 
    }
    
    return callbackReturn;
}

void ARNETWORK_JNIManager_FreeCallbackData (JNIEnv* env, ARNETWORK_JNIManager_CallbackData_t** callbackDataPtrAddr)
{
    /** -- free the global references of the callback -- */
    
    /** local declarations */
    ARNETWORK_JNIManager_CallbackData_t *callbackDataPtr = NULL;
    
    if(callbackDataPtrAddr != NULL)
    {
        callbackDataPtr = *callbackDataPtrAddr;
        
        if(callbackDataPtr != NULL)
        {
            /** delete the java ARNativeData object reference */
            (*env)->DeleteGlobalRef( env, callbackDataPtr->jARData );
            
            /** delete the java manager object reference */
            (*env)->DeleteGlobalRef( env, callbackDataPtr->jManager );
            
            /** the callback data */
            free(callbackDataPtr);
            callbackDataPtr = NULL;
        }
        *callbackDataPtrAddr = NULL;
    }
}
