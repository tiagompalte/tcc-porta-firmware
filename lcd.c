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
 * Controlador do LCD
 * v1.0
 * 2017
 * ************************************************************** */

#include "driverlib/lcd.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"
#include "communication.h"
#define ui32SysClock 120000000

uint8_t LCDRow = 0;
uint8_t LCDColumn = 0;
uint8_t cont = 1;
uint8_t ende = 0;
uint8_t cronometro = 5;

uint32_t ContDelayMS = (120000/3);;
uint32_t ContDelayUS = (120/3);;
char *str_PassRFID = "Passe o cartao RFID";
char *str_PortaEletronica = " PORTA ";
char *str_PortaTrancada = " FECHADA ";
char *str_semCadastro = "RFID NAO Cadastrado";
char *str_CadastreRFID = " Cadastre o RFID";
char *str_entrada = " ENTRADA";
char *str_nao = " NAO";
char *str_permitida = " PERMITIDA";
char *str_BemVindo = " BEM VINDO";
char *str_erro = " ERRO ";
char *str_RFIDNapLido = " RFID nao lido";
char *str_PassRFIDAgain = "Passe RFID novamente";
char *str_Gravando = " GRAVANDO";
char *str_Loading = " CARREGANDO ";
char *str_apaga = "                    ";
char *str_Aguarde = "Aguarde...";
char *str_TypePassWord = "Digite a Senha:";
char *str_serBlocked = "Usuario Bloqueado";
char *str_VoiceKey = "Digite: ";
char *str_Voice = " * -> VOZ";
char *str_Key = " # -> TECLADO";
char *str_sendingData = "Enviando Dados";
char* numPorta;
// Inicializa display
void LCDInit(char* porta)
{
    MAP_GPIOPinWrite(LCD_CMD, LCD_RS|LCD_RW|LCD_ENABLE, 0x00);
    MAP_GPIOPinWrite(LCD_DATA, DATA7|DATA6|DATA5|DATA4|DATA3|DATA2|DATA1|DATA0, 0x00);
    MAP_SysCtlDelay(1*ContDelayMS);


    LCDWriteCmd(FUNCTION_SET|BUS_8_BITS|DISPLAY_LINHAS|FONTE_5x8);    //8-bits,2 display lines, 5x7 font
    MAP_SysCtlDelay(20*ContDelayMS);
    LCDWriteCmd(FUNCTION_SET|BUS_8_BITS|DISPLAY_LINHAS|FONTE_5x8);
    MAP_SysCtlDelay(20*ContDelayMS);
    LCDWriteCmd(ENTRY_SET_MODE|INCREMENT|NO_SHIFT);    // incrementa cursor automaticamente para a direita
    //CheckBusyFlag();
    MAP_SysCtlDelay(20*ContDelayMS);
    LCDWriteCmd(DISPLAY_ON_CONTROL|DISPLAY_ON|CURSOR_OFF|CURSOR_BLINK_OFF);    // Liga o display e apaga o cursor
    //CheckBusyFlag();
    MAP_SysCtlDelay(20*ContDelayMS);
    LCDClear();
    MAP_SysCtlDelay(5*ContDelayMS);
    numPorta = porta;
    LCDALoading(numPorta);
}

// Limpa o display e retorna cursor para a primeira posição da primeira linha
void LCDClear()
{
    LCDWriteCmd(0x01);
    MAP_SysCtlDelay(20*ContDelayMS);
    //CheckBusyFlag();
}

void CheckBusyFlag()
{
    MAP_GPIOPinTypeGPIOInput(LCD_DATA, DATA7);
    MAP_GPIOPinWrite(LCD_CMD, LCD_RS|LCD_RW, 0x02);
    while(MAP_GPIOPinRead(LCD_DATA,BUSY_FLAG) == 0x00);
    MAP_GPIOPinTypeGPIOOutput(LCD_DATA, DATA7);
}

void LCDWriteData(unsigned char data)
{
    // Define modo escrita de dados
    MAP_GPIOPinWrite(LCD_CMD, LCD_RS|LCD_RW, 0x04);

    MAP_GPIOPinWrite(LCD_DATA, DATA7|DATA6|DATA5|DATA4|DATA3|DATA2|DATA1|DATA0, data);
    MAP_GPIOPinWrite(LCD_CMD, LCD_ENABLE, 0x00);
    MAP_SysCtlDelay(1*ContDelayMS);
    MAP_GPIOPinWrite(LCD_CMD, LCD_ENABLE, 0x01); // ENABLE = 1 -> garante o comando
    MAP_GPIOPinWrite(LCD_CMD, LCD_ENABLE, 0x00); //
}
void LCDWriteCmd(unsigned char command)
{
    // RS = 0 -> modo escrita de instruções
    MAP_GPIOPinWrite(LCD_CMD, LCD_RS|LCD_RW, 0x00);

    MAP_GPIOPinWrite(LCD_DATA, DATA7|DATA6|DATA5|DATA4|DATA3|DATA2|DATA1|DATA0, command);
    MAP_GPIOPinWrite(LCD_CMD, LCD_ENABLE, 0x00);
    MAP_SysCtlDelay(1*ContDelayMS);
    MAP_GPIOPinWrite(LCD_CMD, LCD_ENABLE, 0x01); // ENABLE = 1 -> garante o comando
    MAP_GPIOPinWrite(LCD_CMD, LCD_ENABLE, 0x00); //
}

// Coloca cursor na linha determinada
void LCDMoveCursorToXY(uint8_t line, uint8_t column)
{
    uint8_t address = 0x80;;
        switch(line)
        {
            case 1:
                address = LCD_LINE_1;
                break;
            case 2:
                address = LCD_LINE_2;
                break;
            case 3:
                address = LCD_LINE_3;
                break;
            case 4:
                address = LCD_LINE_4;
                break;
            default:
                address = LCD_LINE_1;
                break;
        }
    if(column > 0 & column < 21)
    {
        uint8_t i;
        for(i = 1; i <= column; i++)
        {
            address += 1;
        }
    }
    else
    {
        switch(line)
                {
                    case 1:
                        address = LCD_LINE_1;
                        break;
                    case 2:
                        address = LCD_LINE_2;
                        break;
                    case 3:
                        address = LCD_LINE_3;
                        break;
                    case 4:
                        address = LCD_LINE_4;
                        break;
                    default:
                        address = LCD_LINE_1;
                        break;
                }
    }
    address |= 0x80;
    LCDWriteCmd(address);
    CheckBusyFlag();
}

// Move cursor para a esquerda
void LCDMoveCursorLeft()
{
    LCDWriteCmd(CURSOR_DISPLAY_SHIFT|CURSOR_LEFT_AC_DECR);
    CheckBusyFlag();
}

// Move cursor para a direita
void LCDMoveCursorRight()
{
    LCDWriteCmd(CURSOR_DISPLAY_SHIFT|CURSOR_RIGHT_AC_INCR);
    CheckBusyFlag();
}

// Move display para a esquerda
void LCDMoveDisplayLeft()
{
    LCDWriteCmd(CURSOR_DISPLAY_SHIFT|SHIFT_DISPLAY_LEFT);
    CheckBusyFlag();
}

// Move display para a direita
void LCDMoveDisplayRight()
{
    LCDWriteCmd(CURSOR_DISPLAY_SHIFT|SHIFT_DISPLAY_RIGHT);
    CheckBusyFlag();
}

void LCDNextLine()
{
    LCDWriteCmd(0x01);
    CheckBusyFlag();
}

void LCDWriteString(char *str)
{
    uint8_t count = 1;
    while(*str && count < 20)
    {
        LCDWriteData(*str);
        str++;
        count++;
    }

}

void LCDALoading()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(3,4);
    LCDWriteString(str_Loading);
    MAP_SysCtlDelay(500*ContDelayMS);
    LCDWriteData('.');
    MAP_SysCtlDelay(500*ContDelayMS);
    LCDWriteData('.');
    MAP_SysCtlDelay(500*ContDelayMS);
    LCDWriteData('.');
    MAP_SysCtlDelay(500*ContDelayMS);
}

void LCDInicio()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(3,4);
    LCDWriteString(str_PortaTrancada);
    MAP_SysCtlDelay(1000*ContDelayMS);

    LCDPassRFID();
}

void LCDPassRFID()
 {

    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);

    LCDMoveCursorToXY(3,4);
    LCDWriteString(str_PortaTrancada);

    LCDMoveCursorToXY(4,0);
    LCDWriteString(str_PassRFID);

    //MAP_SysCtlDelay(500*ContDelayMS);

}

void LCDWriteRFID()
{
    char *str = " Passe RFID cartao ";
    LCDWriteString(str);
}

void LCDKeyPassword()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(2,2);
    LCDWriteString(str_TypePassWord);

    LCDMoveCursorToXY(3,4);
}

void LCDShowKeyPassword(uint8_t count)
{

    //LCDMoveCursorToXY(3,5 + count);

}

void LCDUserBlocked()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(3,2);
    LCDWriteString(str_serBlocked);

}


void LCDNotAllowed()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(2,5);
    LCDWriteString(str_entrada);

    LCDMoveCursorToXY(3,7);
    LCDWriteString(str_nao);

    LCDMoveCursorToXY(4,4);
    LCDWriteString(str_permitida);

    //MAP_SysCtlDelay(1000*ContDelayMS);
}
void LCDAllowed(char *UserName)
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(3,5);
    LCDWriteString(str_entrada);
    LCDMoveCursorToXY(4,4);
    LCDWriteString(str_permitida);
    MAP_SysCtlDelay(1000*ContDelayMS);
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(2,4);
    LCDWriteString(str_BemVindo);
    LCDMoveCursorToXY(4,1);
    LCDWriteString(UserName);
}

void LCDBlinkDisplay(uint16_t time)
{
    LCDWriteCmd(DISPLAY_ON_CONTROL|DISPLAY_OFF);
    MAP_SysCtlDelay(time*ContDelayMS);

    LCDWriteCmd(DISPLAY_ON_CONTROL|DISPLAY_ON);
    MAP_SysCtlDelay(time*ContDelayMS);

    LCDWriteCmd(DISPLAY_ON_CONTROL|DISPLAY_OFF);
    MAP_SysCtlDelay(time*ContDelayMS);

    LCDWriteCmd(DISPLAY_ON_CONTROL|DISPLAY_ON);
    MAP_SysCtlDelay(time*ContDelayMS);
}

void LCDNotRegister()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(3,1);
    LCDWriteString(str_semCadastro);

    LCDMoveCursorToXY(4,2);
    LCDWriteString(str_CadastreRFID);

   // MAP_SysCtlDelay(500*ContDelayMS);
}

uint8_t getAddress()
{
    uint8_t add;
    GPIOPinTypeGPIOInput(LCD_DATA, DATA6|DATA5|DATA4|DATA3|DATA2|DATA1|DATA0);
    GPIOPinWrite(LCD_CMD, LCD_RS|LCD_RW, 0x02);
    //GPIOPinWrite(LCD_CMD, LCD_ENABLE, 0x00);
    add = GPIOPinRead(LCD_DATA, DATA6|DATA5|DATA4|DATA3|DATA2|DATA1|DATA0);
    //GPIOPinWrite(LCD_CMD, LCD_ENABLE, 0x01); // ENABLE = 1 -> garante o comando
    //GPIOPinWrite(LCD_CMD, LCD_ENABLE, 0x00); //
    GPIOPinTypeGPIOOutput(LCD_DATA, DATA7|DATA6|DATA5|DATA4|DATA3|DATA2|DATA1|DATA0);

    MAP_SysCtlDelay(15*ContDelayMS);
    //GPIOPinTypeGPIOInput(LCD_DATA, DATA7);
    return add;
}

void LCDRecordingSound()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(2,4);
    LCDWriteString(str_Gravando);
    LCDMoveCursorToXY(3,5);
}

int LCDVoiceKey()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(2,1);
    LCDWriteString(str_VoiceKey);
    LCDMoveCursorToXY(3,1);
    LCDWriteString(str_Voice);
    LCDMoveCursorToXY(4,1);
    LCDWriteString(str_Key);
    return 1;
}

int LCDSendingData()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(3,2);
    LCDWriteString(str_sendingData);
    return 1;
}

int LCDErroLog()
{
    LCDClear();
    LCDMoveCursorToXY(1,4);
    LCDWriteString(str_PortaEletronica);
    LCDWriteString(numPorta);
    LCDMoveCursorToXY(2,1);
    LCDWriteString("ERRO:");
    LCDMoveCursorToXY(3,1);
    LCDWriteString(g_psUserInfo.sReport.log);
    return 1;
}
