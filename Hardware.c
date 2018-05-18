/* ************************************************************** *
 * UTFPR - Universidade Tecnologica Federal do Paraná
 * Engenharia Eletrônica
 * Trabalho de Conclusão de Curso
 * ************************************************************** *
 * Sistema de Segurança baseado em Reconhecimento de Senha Falada
 * ************************************************************** *
 * Equipe:
 * Luiz Felipe Kim Evaristo
 * Tauan Marinho
 * Tiago Henrique Faxina
 * Tiago Mariani Palte
 * ************************************************************** *
 * Controlador Geral do Hardware
 * v1.0
 * 2017
 * ************************************************************** */

#include "communication.h"
#include "stdlib.h"
#include "string.h"

#include "driverlib/audioSample.h"
#include "driverlib/buzzer.h"
#include "driverlib/gpio.h"
#include "driverlib/Hardware.h"
#include "driverlib/interrupt.h"
#include "driverlib/keyboard.h"
#include "driverlib/lcd.h"
#include "driverlib/MyString.h"
#include "driverlib/pin_map.h"
#include "driverlib/rfid.h"
#include "driverlib/rom_map.h"
#include "driverlib/spi.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/trava_eletrica.h"
#include "driverlib/uart.h"
#include "driverlib/fir.h"
#include "utils/uartstdio.h"

#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#define ERROR                  -1
unsigned char RFIDCardPassWord[5];
unsigned char LastRFIDCardPassWord[5];
unsigned char RFIDCardPassWord1[] = {3, 67, 147, 229, 54};
unsigned char RFIDCardPassWord2[] = {117, 224, 6, 136, 27};
unsigned char RFIDCardPassWord3[] = {201, 66, 106, 123, 54};
unsigned char RFIDCardPassWord4[] = {3, 67, 147, 229, 54}; //Não autorizado
unsigned char PassWord[] = {0,0,0,0,0,'\0'};
unsigned char PassWord1[] = {1,1,1,1,1,'\0'};

static volatile bool g_bIntFlag = true;
static volatile bool TestFlag = false;
static volatile bool CardVerifFlag = false;
static volatile bool okRecord = false;

uint8_t PassWordCount = 0;
uint8_t NumTentativas = 0;
unsigned char count = 0x33;
CardStatus cardStatus = CardNotDetected;
UserStatus userStatus = EntryNotAllowed;
UserOptionsStatus userOptionsStatus = none;

void HardwareInit(char* porta)
{
    ulDelayms = (40000);

    MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x00);
    MAP_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0x04);
    MAP_GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 1);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1|GPIO_PIN_0, 2);
    MAP_SysCtlDelay(50*ulDelayms);

    LCDInit(porta);
    MFRC522Init();
    DesacionarTrava();
    BuzzerDeactivate();
    g_bIntFlag = true;
    KeyBoardIntFlag = false;
    cardStatus = CardNotDetected;
}

void LCDIntHandler(void)
{
  MAP_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
  LCDClear();
  LCDPassRFID();
  DesacionarTrava();
  MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1|GPIO_PIN_0, 2);
  MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x00);
  KeyBoardIntFlag = true;
  g_bIntFlag = true;
  cardStatus = CardNotDetected;
}

void BuzzerIntHandler(void)
{
  MAP_TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
  BuzzerDeactivate();

}

void PasswordIntHandler(void)
{
    MAP_TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    PassWordCount = 5;
    unsigned char PassWordDefaut[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, '\0'};
    memcpy(PassWord,PassWordDefaut, 5);
}

void intHandlerTimer4(void)
{
    MAP_TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
    count--;
}

uint8_t HardwareLoop()
{
    unsigned char RFIDstatus;
    unsigned char str[MAX_LEN];
    unsigned char *str_UID;
    unsigned char str_UIDRev[5] = {0, 0, 0, 0, '\0'};
    unsigned char str_UIDExt[9];
    unsigned char str_UIDFinal[12];
    str_UIDExt[8] = '\0';
    str_UIDFinal[11] = '\0';

    cardStatus = CardNotDetected;
    while(1)
    {
        if (MFRC522IsCard())
        {
            RFIDstatus = MFRC522Anticoll(str);
            memcpy(str_UID, str, 4);

           // memcpy(str_UIDRev, strrev(str_UID), 4);
            strSep(str_UID,str_UIDExt);
            hex2str(str_UIDExt, str_UIDExt);
            strInsertSpace(str_UIDExt, str_UIDFinal);
            memcpy(g_psUserInfo.sReport.rfid, str_UIDFinal, 11);
            printf("\n %s", g_psUserInfo.sReport.rfid);
            //printString(str_UIDExt);
            cardStatus = CardDetected;
        }
        printf("\n RFID nao detectado");
        if (cardStatus == CardDetected && RFIDstatus == MI_OK)
        {
            LCDSendingData();
            return 1;
        }
        MFRC522Halt();
    }
    strcpy(g_psUserInfo.sReport.log, "Erro: RFID não cadastrado");
    return 0;
}

uint8_t VerificaTentativas()
{
    if(!StrCompare(LastRFIDCardPassWord, RFIDCardPassWord, 5))
    {
        memcpy(LastRFIDCardPassWord,RFIDCardPassWord, 5);
        NumTentativas = 0;
        userStatus = EntryNotAllowed;
    }
    else
    {
        NumTentativas++;
    }

    return NumTentativas;
}
uint8_t keyPass = 0x00;
void HardwarePassWordControl(int status)
{
    if(status == KEY && userStatus != UserBlocked)
    {
        MAP_SysCtlDelay(500*ulDelayms);
        PassWordCount = 0;
        LCDClear();
        LCDKeyPassword();
        MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 11 * g_ui32SysClock);
        MAP_TimerLoadSet(TIMER2_BASE, TIMER_A, 10 * g_ui32SysClock);
        MAP_TimerEnable(TIMER2_BASE, TIMER_A);
        MAP_TimerEnable(TIMER1_BASE, TIMER_A);
        while(PassWordCount < 4)
        {
            keyPass = KeyboardGetKey();
            if(keyPass != 0xFF)
            {
                PassWord[PassWordCount] = (unsigned char)keyPass;
                PassWordCount++;
                LCDWriteData('*');
                //UARTprintf(" %d ", keyPass);
                MAP_SysCtlDelay(500*ulDelayms);
            }
        }
        keyPass = 0x00;
        CardVerifFlag = true;
        userOptionsStatus = none;
        memcpy(g_psUserInfo.sReport.userKey,PassWord, 4);
        //strcpy(g_psUserInfo.sReport.userKey,"1111");
    }
    else if(status == VOICE && userStatus != UserBlocked)
    {
        uint8_t t = 0;
        count = 0x33;
        LCDRecordingSound();

        MAP_TimerEnable(TIMER4_BASE, TIMER_A);
        LCDMoveCursorToXY(3,5);
        LCDWriteString("EM ");
        while(count > 0x30)
        {
            LCDMoveCursorToXY(3,8);
            LCDWriteData(count);
        }

        MAP_TimerDisable(TIMER1_BASE, TIMER_A);
        LCDRecordingSound();
        CardVerifFlag = true;
        userOptionsStatus = none;
        LCDMoveCursorToXY(3,13);
        LCDWriteData('|');
        LCDMoveCursorToXY(3,4);
        LCDWriteData('|');

        MAP_TimerEnable(TIMER0_BASE, TIMER_A); // Habilita Interrupcao do ADC

        while(t < 8)
        {
            MAP_SysCtlDelay(250*ulDelayms);
            LCDWriteData(176);
            t++;
        }


        if(conversionEnd == 1) {
        	conversionEnd == 0;
        	FIR(bufferConversao);
        	convertAudioToHex();
        	initArray(&bufferComprimido, 1000);
        	compress();	//COMPRESSAO LZW
        	//freeArray(&bufferComprimido);
        }

//        MAP_SysCtlDelay(1000*ulDelayms);
    }
    else if(status == NONE)
    {
        CardVerifFlag = true;
        //userOptionsStatus = none;
    }
    MAP_TimerDisable(TIMER1_BASE, TIMER_A);
    MAP_TimerDisable(TIMER2_BASE, TIMER_A);
    LCDSendingData();
}

void HardwareControl(UserStatus userSta)
{
    if(userSta == EntryAllowed)
    {
        AcionarTrava();
        BuzzerActivate();
        MAP_TimerLoadSet(TIMER3_BASE, TIMER_A, g_ui32SysClock/2);

        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
        LCDAllowed(g_psUserInfo.sReport.name);
        //LCDAllowed("Joao Paulo SIlva Santos Pedroni Machado");
        MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 4 * g_ui32SysClock);
        MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x04);
        MAP_TimerEnable(TIMER1_BASE, TIMER_A);

        CardVerifFlag = false;
        g_bIntFlag = false;
        userStatus = EntryNotAllowed;
        userSta = userStatus;
    }
    else if(userSta == EntryNotAllowed)
    {
        g_bIntFlag = false;
        LCDNotAllowed();
        BuzzerActivate();
        MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 4 * g_ui32SysClock);
        MAP_TimerLoadSet(TIMER3_BASE, TIMER_A, 2 * g_ui32SysClock);
        MAP_TimerEnable(TIMER1_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
        CardVerifFlag = false;
        //userStatus = EntryNotAllowed;
    }
    else if(userSta == UserNotRegistered)
    {
        g_bIntFlag = false;
        LCDErroLog();
        BuzzerActivate();
        MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 4 * g_ui32SysClock);
        MAP_TimerLoadSet(TIMER3_BASE, TIMER_A, 2 * g_ui32SysClock);
        MAP_TimerEnable(TIMER1_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
        CardVerifFlag = false;
        //userStatus = UserNotRegistered;
    }
    else if(userSta == UserBlocked)
    {
        g_bIntFlag = false;
        LCDUserBlocked();
        BuzzerActivate();
        MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 4 * g_ui32SysClock);
        MAP_TimerLoadSet(TIMER3_BASE, TIMER_A, 2 * g_ui32SysClock);
        MAP_TimerEnable(TIMER1_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
        CardVerifFlag = false;
    }

    MAP_SysCtlDelay(5000*ulDelayms);
    // && CardVerifFlag == true && g_bIntFlag == true
}

int hardwareVoiceKey()
{
    LCDClear();
    LCDVoiceKey();
    uint8_t keySelect = 0xFF;
    MAP_TimerLoadSet(TIMER1_BASE, TIMER_A, 5 * g_ui32SysClock);
    MAP_TimerEnable(TIMER1_BASE, TIMER_A);
    KeyBoardIntFlag = false;
    while ((keySelect != '*') && (keySelect != '#') && (KeyBoardIntFlag == false))
    {
        keySelect = 0xFF;
        keySelect = KeyboardGetKey();
    }
    MAP_TimerDisable(TIMER1_BASE, TIMER_A);
    if(keySelect == '#')
    {
        return KEY;
    }
    if(keySelect == '*')
    {
        return VOICE;
    }
    return ERROR;
}
