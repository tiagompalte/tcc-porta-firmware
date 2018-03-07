/* ************************************************************** *
 * UTFPR - Universidade Tecnologica Federal do Paraná
 * Engenharia Eletronica
 * Trabalho de Conclusao de Curso
 * ************************************************************** *
 * Sistema de Seguranca baseado em Reconhecimento de Senha Falada
 * ************************************************************** *
 * Equipe:
 * Luiz Felipe Kim Evaristo
 * Tauan Marinho
 * Tiago Henrique Faxina
 * Tiago Mariani Palte
 * ************************************************************** *
 * json.c - File to handle JSON formatted data from webservers.
 * ************************************************************** */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils/ustdlib.h"
#include "utils/lwiplib.h"
#include "eth_client.h"
#include "json.h"
#include "driverlib/algorithm.h"
//****************************************************************************
//
// The locally defined lwIP buffer parsing pointer.
//
//****************************************************************************
typedef struct
{
    //
    // A pbuf pointer from lwIP.
    //
    struct pbuf *psBuf;

    //
    // The current index into the pbuf pointer.
    //
    uint32_t ui32Idx;
} tBufPtr;

int i32OutIdx;

//****************************************************************************
//
// Initialize a buffer parsing pointer.
//
//****************************************************************************
static void BufPtrInit(tBufPtr *psBufPtr, struct pbuf *psBuf)
{
    psBufPtr->psBuf = psBuf;
    psBufPtr->ui32Idx = 0;
}

//****************************************************************************
//
// Get a single byte from a parsing pointer.
//
//****************************************************************************
static uint8_t BufData8Get(tBufPtr *psBufPtr)
{
    return (((uint8_t *) psBufPtr->psBuf->payload)[psBufPtr->ui32Idx]);
}

//*****************************************************************************
//
// Increment a parsing pointer by a given value.
//
//*****************************************************************************
uint32_t BufPtrInc(tBufPtr *psBufPtr, uint32_t ui32Inc)
{
    psBufPtr->ui32Idx += ui32Inc;

    if (psBufPtr->ui32Idx >= psBufPtr->psBuf->len)
    {
        psBufPtr->ui32Idx = 0;

        psBufPtr->psBuf = psBufPtr->psBuf->next;

        if (psBufPtr->psBuf == 0)
        {
            ui32Inc = 0;
        }
    }
    return (ui32Inc);
}

//*****************************************************************************
//
// Compare a string at the current location of a parsing pointer.
//
//*****************************************************************************
uint32_t CompareString(tBufPtr *psBufPtr, const char *pcField,
                       uint32_t ui32Size)
{
    uint32_t ui32Idx;

    for (ui32Idx = 0; ui32Idx < ui32Size; ui32Idx++)
    {
        if (BufData8Get(psBufPtr) == pcField[ui32Idx])
        {
            if (BufPtrInc(psBufPtr, 1) != 1)
            {
                break;
            }
        }
        else
        {
            break;
        }
    }

    if (ui32Idx == ui32Size)
    {
        return (0);
    }

    return (1);
}

//*****************************************************************************
//
// This function searches for a field from the JSON data.  The field should be
// quoted in the data and immediately follow a '{' character.  This can be used
// to also search for sub data in another field if the pointer to the data for
// the parent field.
//
//*****************************************************************************
static uint32_t GetField(char *pcField, tBufPtr *psBufPtr)
{
    uint32_t ui32Curly, ui32Quote;
    int32_t i32NewItem;

    ui32Curly = 0;
    i32NewItem = 0;
    ui32Quote = 0;

    while (1)
    {
        if (BufData8Get(psBufPtr) == '{')
        {
            ui32Curly++;
        }
        else if (BufData8Get(psBufPtr) == '}')
        {
            ui32Curly--;

            if (ui32Curly == 1)
            {
                ui32Quote = 0;
            }
        }
        else if (BufData8Get(psBufPtr) == ',')
        {
            if ((ui32Curly == 1) && ((ui32Quote & 1) == 0))
            {
                ui32Quote = 0;
            }
        }
        else if (BufData8Get(psBufPtr) == '"')
        {
            if ((ui32Curly == 1) && (ui32Quote == 0))
            {
                i32NewItem = 1;
            }
            ui32Quote++;
        }

        if (BufPtrInc(psBufPtr, 1) != 1)
        {
            break;
        }

        if (i32NewItem)
        {
            i32NewItem = 0;
            if (CompareString(psBufPtr, pcField, ustrlen(pcField)) == 0)
            {
                while (1)
                {
                    if (BufData8Get(psBufPtr) == ':')
                    {
                        if (BufPtrInc(psBufPtr, 1) != 1)
                        {
                            break;
                        }
                        return (1);
                    }

                    if (BufPtrInc(psBufPtr, 1) != 1)
                    {
                        break;
                    }
                }
                return (0);
            }
        }
    }
    return (0);
}

//*****************************************************************************
//
// This function searches for a value in a JSON item as an integer.  These are
// non-quoted values so if the number is a quoted integer this function returns
// -1.  If the value is found then it is returned by this function.
//
//*****************************************************************************
int32_t GetFieldValueInt(tBufPtr *psBufPtr)
{
    int32_t i32Idx;
    char pcTemp[20];
    const char *pEnd;

    i32Idx = 0;

    while (1)
    {
        //
        // Find the end of this item.
        //
        if ((BufData8Get(psBufPtr) == '}') || (BufData8Get(psBufPtr) == ','))
        {
            //
            // null terminate the string and get the value from the string in
            // decimal.
            //
            pcTemp[i32Idx] = 0;
            return (ustrtoul(pcTemp, &pEnd, 10));
        }

        //
        // Should not hit a " char or something went wrong.
        //
        if (BufData8Get(psBufPtr) == '"')
        {
            break;
        }

        //
        // Copy the string to the local variable.
        //
        pcTemp[i32Idx] = BufData8Get(psBufPtr);

        if ((pcTemp[i32Idx] == '\r') || (pcTemp[i32Idx] == '\n'))
        {
            pcTemp[i32Idx] = 0;
        }
        else
        {
            i32Idx++;
        }

        if (BufPtrInc(psBufPtr, 1) != 1)
        {
            break;
        }

        //
        // Number was too large to represent.
        //
        if (i32Idx >= sizeof(pcTemp))
        {
            break;
        }
    }
    return (INVALID_INT);
}

//*****************************************************************************
//
// This function searches for a value in a JSON item as quoted value.  These
// values are quoted values so if the number is not a quoted value this
// function returns -1.  If the quoted value is found then it is returned in
// the pcDataDest array.
//
//*****************************************************************************
int32_t GetFieldValueString(tBufPtr *psBufPtr, char *pcDataDest,
                            uint32_t ui32SizeDest)
{
    int32_t i32OutIdx;

    //
    // The value should always start with a " char or something went wrong.
    //
    if (BufData8Get(psBufPtr) != '"')
    {
        return (-1);
    }

    //
    // Skip the initial " char.
    //
    if (BufPtrInc(psBufPtr, 1) != 1)
    {
        return (-1);
    }

    for (i32OutIdx = 0; i32OutIdx < ui32SizeDest;)
    {
        //
        // Either a '}', ',', or '"' ends an item.
        //
        if ((BufData8Get(psBufPtr) == '}') || (BufData8Get(psBufPtr) == ',')
                || (BufData8Get(psBufPtr) == '"'))
        {
            //
            // Null terminate the string and return.
            //
            pcDataDest[i32OutIdx] = 0;
            return (i32OutIdx);
        }

        //
        // Continue coping chars into the destination buffer.
        //
        pcDataDest[i32OutIdx] = BufData8Get(psBufPtr);

        //
        // These can occur in the response string and need to be ignored.
        //
        if ((pcDataDest[i32OutIdx] == '\r') || (pcDataDest[i32OutIdx] == '\n'))
        {
            pcDataDest[i32OutIdx] = 0;
        }
        else
        {
            i32OutIdx++;
        }

        if (BufPtrInc(psBufPtr, 1) != 1)
        {
            break;
        }
    }

    //
    // Make sure to null terminate inside the current string.
    //
    if (i32OutIdx == ui32SizeDest)
    {
        pcDataDest[i32OutIdx - 1] = 0;
        return (i32OutIdx - 1);
    }
    return (-1);
}

//*****************************************************************************
//
// Fill out the psUserReport structure from data returned from the JSON
// query for GET.
//
//*****************************************************************************
int32_t JSONParseGET(uint32_t ui32Index, tUserReport *psUserReport,
                     struct pbuf *psBuf)
{
    tBufPtr sBufPtr, sBufList, sBufTemp;
    char pcCode[4];
    //char pcTemp[4];
    int32_t i32Items, i32Code;

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Check and see if the request was valid.
    //
    if (GetField("cod", &sBufPtr) != 0)
    {
        //
        // Check for a 404 not found error.
        //
        sBufTemp = sBufPtr;

        //
        // Check for a 404 not found error.
        //
        i32Code = GetFieldValueInt(&sBufTemp);

        sBufTemp = sBufPtr;

        if (i32Code != INVALID_INT)
        {
            if (i32Code == 404)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (i32Code == 401)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
        else if (GetFieldValueString(&sBufTemp, pcCode, sizeof(pcCode)) >= 0)
        {
            //
            // Check for a 404 or 401 not found error.
            //
            if (ustrncmp(pcCode, "404", 3) == 0)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (ustrncmp(pcCode, "401", 3) == 0)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
    }

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Initialize the number of items found.
    //
    i32Items = 0;
    //
    // Save the list pointer.
    //

    sBufList = sBufPtr;

    if (GetField("data", &sBufPtr) != 0)
            {
                if (GetField("nome", &sBufPtr) != 0)
                {
                    GetFieldValueString(&sBufPtr, psUserReport->name,
                                        sizeof(psUserReport->name));
                    i32Items++;
                    psUserReport->status = 200;
                }
                else
                {
                    psUserReport->name[0] = ' ';
                    psUserReport->status = 400;
                    i32Items++;
                }
            }
            else
            {
                psUserReport->status = 404;
            }
            sBufPtr = sBufList;

    if (GetField("data", &sBufPtr) != 0)
    {
        if (GetField("audio", &sBufPtr) != 0)
        {
            //
            // The value should always start with a [ char or something went wrong.
            //
            if (BufData8Get(&sBufPtr) != '[')
            {
                return (-1);
            }

            //
            // Skip the initial [ char.
            //
            if (BufPtrInc(&sBufPtr, 1) != 1)
            {
                return (-1);
            }

            for (i32OutIdx = 0; i32OutIdx < sizeof(bufferDatabase); i32OutIdx++)
            {
                //GetFieldValueString(&sBufPtr, pcTemp, sizeof(pcTemp));
                //psUserReport->audio[0] = ustrtoul(pcTemp[0], &pEnd, 10);
                //psUserReport->audio[i32OutIdx] = GetFieldValueInt(&sBufPtr);
                bufferDatabase[i32OutIdx] = GetFieldValueInt(&sBufPtr);

                //
                // Skip the , char.
                //

                if (BufData8Get(&sBufPtr) != ',')
                {
                    return (-1);
                }

                if (BufPtrInc(&sBufPtr, 1) != 1)
                {
                    return (-1);
                }
            }

            i32Items++;
            psUserReport->status = 200;
        }
        else
        {
            //psUserReport->audio[0] = ' ';
        	bufferDatabase[DELAY_MAX] = ' ';
            psUserReport->status = 400;
            i32Items++;
        }
    }
    else
    {
        psUserReport->status = 404;
    }
    sBufPtr = sBufList;




    return (i32Items);
}

//*****************************************************************************
//
// Fill out the psUserReport structure from data returned from the JSON
// query for POST.
//
//*****************************************************************************
int32_t JSONParsePOST(uint32_t ui32Index, tUserReport *psUserReport,
                      struct pbuf *psBuf)
{
    tBufPtr sBufPtr, sBufList, sBufTemp;
    char pcCode[4];
    int32_t i32Items, i32Code;

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Check and see if the request was valid.
    //
    if (GetField("cod", &sBufPtr) != 0)
    {
        //
        // Check for a 404 not found error.
        //
        sBufTemp = sBufPtr;

        //
        // Check for a 404 not found error.
        //
        i32Code = GetFieldValueInt(&sBufTemp);

        sBufTemp = sBufPtr;

        if (i32Code != INVALID_INT)
        {
            if (i32Code == 404)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (i32Code == 401)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
        else if (GetFieldValueString(&sBufTemp, pcCode, sizeof(pcCode)) >= 0)
        {
            //
            // Check for a 404 or 401 not found error.
            //
            if (ustrncmp(pcCode, "404", 3) == 0)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (ustrncmp(pcCode, "401", 3) == 0)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
    }

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Initialize the number of items found.
    //
    i32Items = 0;

    //
    // Save the list pointer.
    //
    sBufList = sBufPtr;

    if (GetField("data", &sBufPtr) != 0)
    {
        if (GetField("token", &sBufPtr) != 0)
        {
            GetFieldValueString(&sBufPtr, psUserReport->token,
                                sizeof(psUserReport->token));
            i32Items++;
            psUserReport->status = 200;//OK
        }
        else
        {
            psUserReport->status = 400;//errorKey
        }
    }
    else
    {
        psUserReport->status = 404;
    }

    sBufPtr = sBufList;

    return (i32Items);
}

//*****************************************************************************
//
// Fill out the psUserReport structure from data returned from the JSON
// query for POST.
//
//*****************************************************************************
int32_t JSONParsePOSTKEY(uint32_t ui32Index, tUserReport *psUserReport,
                         struct pbuf *psBuf)
{
    tBufPtr sBufPtr, sBufList, sBufTemp;
    char pcCode[4];
    int32_t i32Items, i32Code;

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Check and see if the request was valid.
    //
    if (GetField("cod", &sBufPtr) != 0)
    {
        //
        // Check for a 404 not found error.
        //
        sBufTemp = sBufPtr;

        //
        // Check for a 404 not found error.
        //
        i32Code = GetFieldValueInt(&sBufTemp);

        sBufTemp = sBufPtr;

        if (i32Code != INVALID_INT)
        {
            if (i32Code == 404)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (i32Code == 401)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
        else if (GetFieldValueString(&sBufTemp, pcCode, sizeof(pcCode)) >= 0)
        {
            //
            // Check for a 404 or 401 not found error.
            //
            if (ustrncmp(pcCode, "404", 3) == 0)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (ustrncmp(pcCode, "401", 3) == 0)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
    }

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Initialize the number of items found.
    //
    i32Items = 0;

    //
    // Save the list pointer.
    //
    sBufList = sBufPtr;

    if (GetField("data", &sBufPtr) != 0)
    {
        if (GetField("mensagem", &sBufPtr) != 0)
        {
            GetFieldValueString(&sBufPtr, psUserReport->authorization,
                                sizeof(psUserReport->authorization));
            i32Items++;
            psUserReport->status = 200;
        }
        else
        {
            psUserReport->authorization[0] = ' ';
            psUserReport->status = 400;
        }
    }
    else
    {
        psUserReport->token[0] = ' ';
        psUserReport->status = 400;
    }
    sBufPtr = sBufList;

    return (i32Items);
}

//*****************************************************************************
//
// Fill out the psUserReport structure from data returned from the JSON
// query for GET.
//
//*****************************************************************************
int32_t JSONParsePOSTACCESS(uint32_t ui32Index, tUserReport *psUserReport,
                            struct pbuf *psBuf)
{
    tBufPtr sBufPtr, sBufList, sBufTemp;
    char pcCode[4];
    int32_t i32Items, i32Code;

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Check and see if the request was valid.
    //
    if (GetField("cod", &sBufPtr) != 0)
    {
        //
        // Check for a 404 not found error.
        //
        sBufTemp = sBufPtr;

        //
        // Check for a 404 not found error.
        //
        i32Code = GetFieldValueInt(&sBufTemp);

        sBufTemp = sBufPtr;

        if (i32Code != INVALID_INT)
        {
            if (i32Code == 404)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (i32Code == 401)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
        else if (GetFieldValueString(&sBufTemp, pcCode, sizeof(pcCode)) >= 0)
        {
            //
            // Check for a 404 or 401 not found error.
            //
            if (ustrncmp(pcCode, "404", 3) == 0)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (ustrncmp(pcCode, "401", 3) == 0)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
    }

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Initialize the number of items found.
    //
    i32Items = 0;

    //
    // Save the list pointer.
    //
    sBufList = sBufPtr;

    if (GetField("data", &sBufPtr) != 0)
    {
        if (GetField("mensagem", &sBufPtr) != 0)
        {
            GetFieldValueString(&sBufPtr, psUserReport->log,
                                sizeof(psUserReport->log));
            i32Items++;
            psUserReport->status = 200;
        }
        else
        {
            psUserReport->status = 400;
        }
    }
    else
    {
        psUserReport->status = 404;
    }
    sBufPtr = sBufList;

    return (i32Items);
}

//*****************************************************************************
//
// Fill out the psUserReport structure from data returned from the JSON
// query for GET.
//
//*****************************************************************************
int32_t JSONParseGETteste(uint32_t ui32Index, tUserReport *psUserReport,
                          struct pbuf *psBuf)
{
    tBufPtr sBufPtr, sBufList, sBufTemp;
    char pcCode[4];
    char pcBuffNovo[512];
    char pcBuffCodigoNovo[512];
    int32_t i32Items, i32Code;

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Check and see if the request was valid.
    //
    if (GetField("cod", &sBufPtr) != 0)
    {
        //
        // Check for a 404 not found error.
        //
        sBufTemp = sBufPtr;

        //
        // Check for a 404 not found error.
        //
        i32Code = GetFieldValueInt(&sBufTemp);

        sBufTemp = sBufPtr;

        if (i32Code != INVALID_INT)
        {
            if (i32Code == 404)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (i32Code == 401)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
        else if (GetFieldValueString(&sBufTemp, pcCode, sizeof(pcCode)) >= 0)
        {
            //
            // Check for a 404 or 401 not found error.
            //
            if (ustrncmp(pcCode, "404", 3) == 0)
            {
                psUserReport->status = 404;
                return (-1);
            }
            if (ustrncmp(pcCode, "401", 3) == 0)
            {
                psUserReport->status = 401;
                return (-1);
            }
        }
    }

    //
    // Reset the pointers.
    //
    BufPtrInit(&sBufPtr, psBuf);

    //
    // Initialize the number of items found.
    //
    i32Items = 0;
    //
    // Save the list pointer.
    //
    sBufList = sBufPtr;

    //
    // Get the audio.
    //
    if (GetField("data", &sBufPtr) != 0)
    {
        if (GetField("hash", &sBufPtr) != 0)
        {
           // GetFieldValueString(&sBufPtr, psUserReport->audio,
           //                     sizeof(psUserReport->audio));
            //GetFieldValueString(&sBufPtr, bufferDatabase,
             //                               sizeof(bufferDatabase));
            i32Items++;
            psUserReport->status = 200;
        }
        else
        {
            //psUserReport->audio[0] = ' ';
            psUserReport->status = 200;
        }
    }
    else
    {
        psUserReport->token[0] = ' ';
        psUserReport->status = 400;
    }

    //
    // Reset the pointer to the start of the list values.
    //
    sBufPtr = sBufList;

    //
    // Get the errors.
    //
    if (GetField("codigoNome", &sBufPtr) != 0)
    {
        GetFieldValueString(&sBufPtr, pcBuffNovo, sizeof(pcBuffNovo));
        //psUserReport->status = pcBuffNovo;
        //psUserReport->status = GetFieldValueInt(&sBufPtr);
        i32Items++;
    }
    else
    {
        psUserReport->status = 0;
    }

    sBufPtr = sBufList;

    //
    // Get the codigoNome.
    //
    if (GetField("codigoNome", &sBufPtr) != 0)
    {
        GetFieldValueString(&sBufPtr, pcBuffCodigoNovo,
                            sizeof(pcBuffCodigoNovo));
        //psUserReport->codigoNome = pcBuffCodigoNovo;
        //psUserReport->status = GetFieldValueInt(&sBufPtr);
        i32Items++;
    }
    else
    {
        //psUserReport->codigoNome = 0;
    }

    return (i32Items);
}
