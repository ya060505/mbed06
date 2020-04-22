#include "mbed.h"

#include "TextLCD.h"

#include "fsl_port.h"

#include "fsl_gpio.h"

#define UINT14_MAX        16383

// FXOS8700CQ I2C address

#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0

#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0

#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1

#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1

// FXOS8700CQ internal register addresses

#define FXOS8700Q_STATUS 0x00

#define FXOS8700Q_OUT_X_MSB 0x01

#define FXOS8700Q_OUT_Y_MSB 0x03

#define FXOS8700Q_OUT_Z_MSB 0x05

#define FXOS8700Q_M_OUT_X_MSB 0x33

#define FXOS8700Q_M_OUT_Y_MSB 0x35

#define FXOS8700Q_M_OUT_Z_MSB 0x37

#define FXOS8700Q_WHOAMI 0x0D

#define FXOS8700Q_XYZ_DATA_CFG 0x0E

#define FXOS8700Q_CTRL_REG1 0x2A

#define FXOS8700Q_M_CTRL_REG1 0x5B

#define FXOS8700Q_M_CTRL_REG2 0x5C

#define FXOS8700Q_WHOAMI_VAL 0xC7


Serial pc(USBTX, USBRX);

I2C i2c( PTD9,PTD8);

I2C i2c_lcd(D14,D15);

TextLCD_I2C lcd(&i2c_lcd, 0x4E, TextLCD::LCD16x2);

int m_addr = FXOS8700CQ_SLAVE_ADDR1;


void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);

void FXOS8700CQ_writeRegs(uint8_t * data, int len);

void lcdf(float t[3]);//////////////////////


int main() {


   pc.baud(115200);


   uint8_t who_am_i, data[2], res[6];

   int16_t acc16;

   float t[3];


   // Enable the FXOS8700Q


   FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);

   data[1] |= 0x01;

   data[0] = FXOS8700Q_CTRL_REG1;

   FXOS8700CQ_writeRegs(data, 2);


   // Get the slave address

   FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);


   pc.printf("Here is %x\r\n", who_am_i);
   
   while (true) {


      FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);


      acc16 = (res[0] << 6) | (res[1] >> 2);

      if (acc16 > UINT14_MAX/2)

         acc16 -= UINT14_MAX;

      t[0] = ((float)acc16) / 4096.0f;


      acc16 = (res[2] << 6) | (res[3] >> 2);

      if (acc16 > UINT14_MAX/2)

         acc16 -= UINT14_MAX;

      t[1] = ((float)acc16) / 4096.0f;


      acc16 = (res[4] << 6) | (res[5] >> 2);

      if (acc16 > UINT14_MAX/2)

         acc16 -= UINT14_MAX;

      t[2] = ((float)acc16) / 4096.0f;


      printf("FXOS8700Q ACC: X=%1.4f(%x%x) Y=%1.4f(%x%x) Z=%1.4f(%x%x)\r\n",\

            t[0], res[0], res[1],\

            t[1], res[2], res[3],\

            t[2], res[4], res[5]\

      );

      lcdf(t);

      //wait(1.0);

   }

}


void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {

   char t = addr;

   i2c.write(m_addr, &t, 1, true);

   i2c.read(m_addr, (char *)data, len);

}


void FXOS8700CQ_writeRegs(uint8_t * data, int len) {

   i2c.write(m_addr, (char *)data, len);

}

/////////////////////////////////////
void lcdf(float t[3]) {

    int row=0, col=0;

    pc.printf("LCD Test. Columns=%d, Rows=%d\n\r", lcd.columns(), lcd.rows());


    for (row=0; row<lcd.rows(); row++) {

      col=0;


      pc.printf("MemAddr(Col=%d, Row=%d)=0x%02X\n\r", col, row, lcd.getAddress(col, row));


      lcd.putc('0' + row);


      for (col=1; col<lcd.columns()-1; col++) {

        lcd.putc('*');

      }


      pc.printf("MemAddr(Col=%d, Row=%d)=0x%02X\n\r", col, row, lcd.getAddress(col, row));

      lcd.putc('+');


    }

    row=0;
    col=0;


// Show cursor as blinking character

    lcd.setCursor(TextLCD::CurOff_BlkOn);


// Set and show user defined characters. A maximum of 8 UDCs are supported by the HD44780.

// They are defined by a 5x7 bitpattern.

    int i[3];
    char c[3] = {'X', 'Y', 'Z'};
    for (int j=0; j<3; j++) {
        lcd.putc(c[j]);
        lcd.putc('=');
        if(t[j]<0){
            lcd.putc('-');
            t[j]=-t[j];
        }
        i[0]=t[j];
        i[1]=10*t[j];
        i[1]=i[1]%10;
        i[2]=100*t[j];
        i[2]=i[2]%10;
        lcd.putc('0'+i[0]);
        lcd.putc('.');
        lcd.putc('0'+i[1]);
        lcd.putc('0'+i[2]);
        lcd.putc(' ');
    }

    wait(1.0);
    lcd.cls();

}
////////////////////////