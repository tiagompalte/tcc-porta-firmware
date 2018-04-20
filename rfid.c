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
 * Comunicação com MFRC522, Card MIFARE S50 e ISO 14443
 * v1.0
 * 2017
 * ************************************************************** */

#include "driverlib/rfid.h"

#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/spi.h"
#include "string.h"

uint32_t g_ulDelayms = (120000000 / 3000);

/*
 * Inicializa o chip MFRC522
 */
void MFRC522Init()
{
    MAP_GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 0x01);    //Pino RST em HIGH

    MFRC522Reset();

    //Timer: TPrescaler*TreloadVal/6.78MHz = 24ms
    MFRC522Write(TModeReg, 0x8D);       //Tauto=1; f(Timer) = 6.78MHz/TPreScaler
    MFRC522Write(TPrescalerReg, 0x3E);  //TModeReg[3..0] + TPrescalerReg
    MFRC522Write(TReloadRegL, 30);
    MFRC522Write(TReloadRegH, 0);

    MFRC522Write(TxAutoReg, 0x40);      //100%ASK
    MFRC522Write(ModeReg, 0x3D);        // CRC valor inicial de 0x6363

    MFRC522AntennaOn();                 //Liga a antena
}

/*
 * Ecxecuta um soft reset no chip MFRC522 e aguarda por ele estar pronto novamente
 */
void MFRC522Reset()
{
    MFRC522Write(CommandReg, PCD_RESETPHASE);   //Emite um comando de soft reset
}

/*
 * Escreve um byte em um resgistrador específico do chip MFRC522
 * A interfece é descrita na seção 8.1.2 do datasheet
 * inputs: addr-> endereço do registrador
 *         val-> byte a ser escrito
 */
void MFRC522Write(unsigned char addr, unsigned char val)
{
    MAP_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0x00);    // SDA = 0 -> Seleciona o Slave
    SPITransfer((((addr<<1)&0x7E)<<8)|val);
    MAP_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0x04);    // SDA = 1 -> Libera o Slave
}

/*
 * Lê um byte em um resgistrador específico do chip MFRC522
 * A interfece é descrita na seção 8.1.2 do datasheet
 * inputs: addr-> endereço do registrador
 * return: byte lido
 */
unsigned char MFRC522Read(unsigned char addr)
{
    unsigned char val;
    MAP_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0x00);        // SDA = 0 -> Seleciona o Slave
    val = SPITransfer(((0x80|(addr<<1))<<8)|0x00);
    MAP_GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_2, 0x04);        // SDA = 1 -> Libera o Slave
    return val;
}

/*
 * Liga a antena habilitando os pinos TX1 e TX2
 */
void MFRC522AntennaOn(void)
{
    unsigned char temp;

    temp = MFRC522Read(TxControlReg);
    if (!(temp & 0x03))
    {
        MFRC522SetBitMask(TxControlReg, 0x03);
    }
}

/*
 * Seta os bits passados em uma máscara (mask) no registrados reg
 * input: reg -> registrador a ser setado
 *        mask -> mascará para a configuração dos bits do registrador
 */
void MFRC522SetBitMask(unsigned char reg, unsigned char mask)
{
    unsigned char tmp;
    tmp = MFRC522Read(reg);
    MFRC522Write(reg, tmp | mask);  // set bit mask
}

/*
 * Limpa (zera) os bits passados em uma máscara (mask) no registrados reg
 * input: reg -> registrador a ser setado
 *        mask -> mascará para a configuração dos bits do registrador
 */
void MFRC522ClearBitMask(unsigned char reg, unsigned char mask)
{
    unsigned char tmp;
    tmp = MFRC522Read(reg);
    MFRC522Write(reg, tmp & (~mask));  // clear bit mask
}

/*
 * Retorna true se um PICC responde ao PICC_REQIDL
 * Somente "novos" cartões em estado IDLE são "convidados". Cartões no estado HALT são ignorados
 */
bool MFRC522IsCard()
{
    unsigned char status = false;
    unsigned char str[MAX_LEN];

    status = MFRC522Request(PICC_REQIDL, str);
    if (status == MI_OK) {
        return true;
    } else {
        return false;
    }
}

/*
 * Retorna true se o ID do cartão pode ser lido
 */
bool MFRC522ReadCardSerial(){

    unsigned char status;
    unsigned char str[MAX_LEN];

    status = MFRC522Anticoll(str);      //str recebe a senha do cartão
    memcpy(serNum, str, 5);

    if (status == MI_OK) {
        return true;
    } else {
        return false;
    }
}

/*
 * Utiliza o coprocessador CRC no MFRC522 para calcular um CRC_A
 */
void MFRC522CalculateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
    unsigned char i, n;

    MFRC522ClearBitMask(DivIrqReg, 0x04);          //Limpa o bit de requisição de interrupção CRCIrq = 0
    MFRC522SetBitMask(FIFOLevelReg, 0x80);         //FlushBuffer = 1, FIFO initialization

    //Escrever dados no FIFO
    for (i=0; i<len; i++)
    {
        MFRC522Write(FIFODataReg, *(pIndata+i));
    }
    MFRC522Write(CommandReg, PCD_CALCCRC);

    // Esperar a finalização do cálculo do CRC
    i = 0xFF;
    do
    {
        n = MFRC522Read(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));            //CRCIrq = 1

    //Lê o cálculo do CRC
    //Transfereo resultado dos registradores para o buffer
    pOutData[0] = MFRC522Read(CRCResultRegL);
    pOutData[1] = MFRC522Read(CRCResultRegM);
}

/*
 * Comunicação entre MFRC522 e ISO14443
 * input: command -> bits de comando do MFRC522
 *        sendData -> Dados para o cartão via MFRC522
 *        sendLen -> Comprimento dos dados enviados
 *        backData -> Dados retornados do cartão
 *        backLen- > Comprimento dos dados retornados do cartão
 * return: MI_OK caso sucesso
 */
unsigned char MFRC522ToCard(unsigned char command, unsigned char *sendData, unsigned char sendLen, unsigned char *backData, unsigned int *backLen)
{
    unsigned char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitIRq = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;

    switch (command)
    {
        case PCD_AUTHENT:       //Autenticação
        {
            irqEn = 0x12;
            waitIRq = 0x10;
            break;
        }
        case PCD_TRANSCEIVE:    //Transmissão de dados FIFO
        {
            irqEn = 0x77;
            waitIRq = 0x30;
            break;
        }
        default:
            break;
    }

    MFRC522Write(CommIEnReg, irqEn|0x80);           // Pedido de interrupção
    MFRC522ClearBitMask(CommIrqReg, 0x80);          // Limapr todos os bits de solicitação de interrupção
    MFRC522SetBitMask(FIFOLevelReg, 0x80);          //FlushBuffer=1, FIFO de inicialização

    MFRC522Write(CommandReg, PCD_IDLE);             //Sem ação. Cancela o comando atual

    //Escrever dados no FIFO
    for (i=0; i<sendLen; i++)
    {
        MFRC522Write(FIFODataReg, sendData[i]);
    }

    //Executa o camando
    MFRC522Write(CommandReg, command);
    if (command == PCD_TRANSCEIVE)
    {
        MFRC522SetBitMask(BitFramingReg, 0x80);        //StartSend=1: transmissão de dados começa
    }

    // Espera receber os dados para completar
    i = 2000;
    do
    {


        n = MFRC522Read(CommIrqReg);                    //CommIrqReg[7..0] bits são: Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitIRq));

    MFRC522ClearBitMask(BitFramingReg, 0x80);           //StartSend=0

    if (i != 0)
    {
        if(!(MFRC522Read(ErrorReg) & 0x1B))             //BufferOvfl Collerr CRCErr ProtecolErr
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }

            if (command == PCD_TRANSCEIVE)
            {
                n = MFRC522Read(FIFOLevelReg);
                lastBits = MFRC522Read(ControlReg) & 0x07;
                if (lastBits)
                {
                    *backLen = (n-1)*8 + lastBits;
                }
                else
                {
                    *backLen = n*8;
                }

                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAX_LEN)
                {
                    n = MAX_LEN;
                }

                //Lê os dados recebidos no FIFO
                for (i=0; i<n; i++)
                {
                    backData[i] = MFRC522Read(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

    return status;
}


/*
 * Procura cartão e lê o tipo de cartão
 * input: reqMode -> Modo do cartão
 *        Tagtype -> Retorna o tipo do cartão
 *              0x4400 = Mifare_UltraLight
 *              0x0400 = Mifare_One(S50)
 *              0x0200 = Mifare_One(S70)
 *              0x0800 = Mifare_Pro(X)
 *              0x4403 = Mifare_DESFire
 * return: MI_OK caso bem sucedido
 */
unsigned char  MFRC522Request(unsigned char reqMode, unsigned char *TagType)
{
    unsigned char status = MI_ERR;
    unsigned int backBits;

    MFRC522Write(BitFramingReg, 0x07);      //TxLastBists = BitFramingReg[2..0]

    TagType[0] = reqMode;
    status = MFRC522ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);

    if ((status != MI_OK) || (backBits != 0x10))
    {
        status = MI_ERR;
    }

    return status;
}

/*
 * Detecção de colisão entre cartões colocados ao mesmo tempo. Previne conflitos.
 * Lê o número de série do cartão
 * Input: setNum -> retorna 4 bytes do número serial do cartão. O quinto Byte é o byte de verificação
 * return: MI_OK caso bem sucedido
 */
unsigned char MFRC522Anticoll(unsigned char *serNum)
{
    unsigned char status = MI_ERR;
    unsigned char i;
    unsigned char serNumCheck=0;
    unsigned int unLen;

    MFRC522Write(BitFramingReg, 0x00);      //TxLastBists = BitFramingReg[2..0]

    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;
    status = MFRC522ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (status == MI_OK)
    {
        //Verifica o número de série do cartão
        for (i=0; i<4; i++)
        {
            serNumCheck ^= serNum[i];
        }
        if (serNumCheck != serNum[i])
        {
            status = MI_ERR;
        }
    }

    return status;
}

/*
 * Envia comando para os cartões en estado ATIVO entrarem em modo de suspensão (HALT)
 */
unsigned char MFRC522Halt()
{
    unsigned char status;
    unsigned int unLen;
    unsigned char buff[4];

    buff[0] = PICC_HALT;
    buff[1] = 0;
    MFRC522CalculateCRC(buff, 2, &buff[2]);

    status = MFRC522ToCard(PCD_TRANSCEIVE, buff, 4, buff,&unLen);
    return status;
}
