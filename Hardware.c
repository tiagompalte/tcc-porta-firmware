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
#include "stdlib.h"
#include "driverlib/Hardware.h"
#include "string.h"
#include "driverlib/MyString.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include"driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "inc/hw_ssi.h"
#include "driverlib/ssi.h"
#include "driverlib/spi.h"
#include "driverlib/rfid.h"
#include "driverlib/lcd.h"
#include "driverlib/buzzer.h"
#include "driverlib/MyString.h"
#include "driverlib/trava_eletrica.h"
#include "driverlib/keyboard.h"
#include "communication.h"

unsigned char RFIDCardPassWord[5];
unsigned char LastRFIDCardPassWord[5];
unsigned char RFIDCardPassWord1[] = {3, 67, 147, 229, 54};
unsigned char RFIDCardPassWord2[] = {117, 224, 6, 136, 27};
unsigned char RFIDCardPassWord3[] = {201, 66, 106, 123, 54};
unsigned char RFIDCardPassWord4[] = {3, 67, 147, 229, 54}; //Não autorizado
unsigned char PassWord[] = {0,0,0,0,0,'\0'};
unsigned char PassWord1[] = {1,1,1,1,1,'\0'};
uint8_t PassWordCount = 0;
uint8_t NumTentativas = 0;
CardStatus cardStatus = CardNotDetected;
UserStatus userStatus = EntryNotAllowed;
UserOptionsStatus userOptionsStatus = none;
static volatile bool g_bIntFlag = true;
static volatile bool TestFlag = false;
static volatile bool KeyBoardIntFlag = false;
static volatile bool CardVerifFlag = false;

void HardwareInit()
{

    ulDelayms = (40000);
    uint32_t initialData = 0;

    MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x00);
    MAP_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0x04);
    MAP_GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 1);
    MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1|GPIO_PIN_0, 2);
    MAP_SysCtlDelay(50*ulDelayms);

    LCDInit();
    MFRC522Init();
    DesacionarTrava();
    BuzzerDeactivate();
    g_bIntFlag = true;
    KeyBoardIntFlag = false;
    cardStatus = CardNotDetected;
}

void LCDIntHandler(void)
{
  MAP_TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
  LCDClear();
  LCDInicio();
  DesacionarTrava();
  MAP_GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_1|GPIO_PIN_0, 2);
  MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x00);
  KeyBoardIntFlag = true;
  g_bIntFlag = true;
  cardStatus = CardNotDetected;
}

void BuzzerIntHandler(void)
{
  MAP_TimerIntClear(TIMER4_BASE, TIMER_TIMA_TIMEOUT);
  BuzzerDeactivate();

}

void PasswordIntHandler(void)
{
    MAP_TimerIntClear(TIMER5_BASE, TIMER_TIMA_TIMEOUT);
    PassWordCount = 5;
    unsigned char PassWordDefaut[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, '\0'};
    memcpy(PassWord,PassWordDefaut, 5);
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
    int i = 0;
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
            return str;
        }
        /*VerificaTentativas();
        HardwarePassWordControl();
        HardwareControl();*/
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

   /* if(NumTentativas > 3)
    {
        userStatus = UserBlocked;
        userOptionsStatus = none;
    }
    */
    return NumTentativas;
}
uint8_t keyPass = 0x00;
void HardwarePassWordControl(int status)
{
    if(status == KEY && userStatus != UserBlocked)
    {
        PassWordCount = 0;
        LCDClear();
        LCDKeyPassword();
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
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
        //PasswordValidate();
        keyPass = 0x00;
        CardVerifFlag = true;
        userOptionsStatus = none;
        strcpy(g_psUserInfo.sReport.userKey,PassWord);
    }
    else if(status == VOICE && userStatus != UserBlocked)
    {
        uint8_t t = 0;
        LCDRecordingSound();
        CardVerifFlag = true;
        userOptionsStatus = none;
        //userStatus = EntryAllowed;
        LCDMoveCursorToXY(3,14);
        //LCDWriteData(t+48);
        LCDWriteData('|');
        LCDMoveCursorToXY(3,5);
        LCDWriteData('|');

        MAP_TimerEnable(TIMER0_BASE, TIMER_A); // Habilita Interrupcao do ADC

        while(t < 8)
        {
            MAP_SysCtlDelay(250*ulDelayms);
            LCDWriteData(176);
            t++;
        }

//        MAP_SysCtlDelay(1000*ulDelayms);
    }
    else if(status == NONE)
    {
        CardVerifFlag = true;
        //userOptionsStatus = none;
    }
}

void HardwareControl(UserStatus userSta)
{
    if(userSta == EntryAllowed && CardVerifFlag == true && g_bIntFlag == true)
    {
        LCDAllowed(g_psUserInfo.sReport.name);
        AcionarTrava();
        BuzzerActivate();
        MAP_GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0x04);
        MAP_TimerLoadSet(TIMER4_BASE, TIMER_A, 0.5 * g_ui32SysClock);
        MAP_TimerEnable(TIMER4_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
        CardVerifFlag = false;
        g_bIntFlag = false;
        userStatus = EntryNotAllowed;
        userSta = userStatus;
    }
    else if(userSta == EntryNotAllowed && CardVerifFlag == true && g_bIntFlag == true)
    {
        g_bIntFlag = false;
        LCDNotAllowed();
        BuzzerActivate();
        MAP_TimerLoadSet(TIMER4_BASE, TIMER_A, 1 * g_ui32SysClock);
        MAP_TimerEnable(TIMER4_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
        CardVerifFlag = false;
        //userStatus = EntryNotAllowed;
    }
    else if(userSta == UserNotRegistered && CardVerifFlag == true && g_bIntFlag == true)
    {
        g_bIntFlag = false;
        LCDErroLog();
        BuzzerActivate();
        MAP_TimerLoadSet(TIMER4_BASE, TIMER_A, 1 * g_ui32SysClock);
        MAP_TimerEnable(TIMER4_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
        CardVerifFlag = false;
        //userStatus = UserNotRegistered;
    }
    else if(userSta == UserBlocked && CardVerifFlag == true && g_bIntFlag == true)
    {
        g_bIntFlag = false;
        LCDUserBlocked();
        BuzzerActivate();
        MAP_TimerLoadSet(TIMER4_BASE, TIMER_A, 1 * g_ui32SysClock);
        MAP_TimerEnable(TIMER4_BASE, TIMER_A);
        MAP_TimerEnable(TIMER3_BASE, TIMER_A);
        CardVerifFlag = false;
    }
    else if(CardVerifFlag == true && g_bIntFlag == true)
    {
        CardVerifFlag = false;
        //userStatus = EntryNotAllowed;
    }
    //cardStatus = CardNotDetected;
}

int hardwareVoiceKey()
{
    LCDClear();
    LCDVoiceKey();
    keyPass = 0xFF;
    MAP_TimerEnable(TIMER3_BASE, TIMER_A);
    while (keyPass != '*' && keyPass != '#' && KeyBoardIntFlag == false)
    {
        keyPass = KeyboardGetKey();
        printf("\n Lendo teclado");
    }
    if(keyPass == 0x0E)
    {
        return VOICE;
    }
    else if(keyPass == 0x0F)
    {
        return KEY;
    }
    KeyBoardIntFlag = true;
    return -1;
}
