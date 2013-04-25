/**
 * @file ARNETWORK_Error.c
 * @brief informations about libARNetwork errors
 * @date 04/18/2013
 * @author nicolas.brulez@parrot.com
 */

#include <libARNetwork/ARNETWORK_Error.h>

//
// To generate the function using emacs regexp syntax
// - remove all cases (except for default)
// - copy the content (no braces) of eARNETWORK_ERROR enum
// - replace rexexp (M-x replace-regexp) with the following parameters
//   - Source: [\ \t]*\([A-Z_]*\)[-0-9\ \t,=]*/\*\*<\ \(.*\)\ \*/
//   - Dest  : case \1:^Jreturn "\2";^J;break;
// - Reindent all lines in file
//
// Note: ^J character is the newline character, C-q C-j in emacs
//

char* ARNETWORK_Error_ToString (eARNETWORK_ERROR error)
{
    switch (error)
    {
    case ARNETWORK_OK:
        return "No error";
        break;
    case ARNETWORK_ERROR:
        return "Unknown generic error";
        break;
    case ARNETWORK_ERROR_ALLOC:
        return "Memory allocation error";
        break;
    case ARNETWORK_ERROR_BAD_PARAMETER:
        return "Bad parameters";
        break;
    case ARNETWORK_ERROR_ID_UNKNOWN:
        return "Given IOBuffer identifier is unknown";
        break;
    case ARNETWORK_ERROR_BUFFER_SIZE:
        return "Insufficient free space in the buffer";
        break;
    case ARNETWORK_ERROR_BUFFER_EMPTY:
        return "Buffer is empty, nothing was read";
        break;
    case ARNETWORK_ERROR_SEMAPHORE:
        return "Error when using a semaphore";
        break;
    case ARNETWORK_ERROR_MANAGER:
        return "Unknown ARNETWORK_Manager error";
        break;
    case ARNETWORK_ERROR_MANAGER_NEW_IOBUFFER:
        return "IOBuffer creation error";
        break;
    case ARNETWORK_ERROR_MANAGER_NEW_SENDER:
        return "Sender creation error";
        break;
    case ARNETWORK_ERROR_MANAGER_NEW_RECEIVER:
        return "Receiver creation error";
        break;
    case ARNETWORK_ERROR_NEW_BUFFER:
        return "Buffer creation error";
        break;
    case ARNETWORK_ERROR_NEW_RINGBUFFER:
        return "RingBuffer creation error";
        break;
    case ARNETWORK_ERROR_IOBUFFER:
        return "Unknown IOBuffer error";
        break;
    case ARNETWORK_ERROR_IOBUFFER_BAD_ACK:
        return "Bad sequence number for the acknowledge";
        break;
    case ARNETWORK_ERROR_SOCKET:
        return "Unknown socket error";
        break;
    case ARNETWORK_ERROR_SOCKET_PERMISSION_DENIED:
        return "Permission denied on a socket";
        break;
    case ARNETWORK_ERROR_RECEIVER:
        return "Unknown Receiver error";
        break;
    case ARNETWORK_ERROR_RECEIVER_BUFFER_END:
        return "Receiver buffer too small";
        break;
    case ARNETWORK_ERROR_RECEIVER_BAD_FRAME:
        return "Bad frame content on network";
        break;
    default:
        return "Unknown error";
        break;
    }
    return "Unknown error";
}
