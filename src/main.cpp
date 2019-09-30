#include <Arduino.h>

#define twi_sda 4
#define twi_scl 5

#define SDA_LOW()   (GPES = (1 << twi_sda)) //Enable SDA (becomes output and since GPO is 0 for the pin, it will pull the line low)
#define SDA_HIGH()  (GPEC = (1 << twi_sda)) //Disable SDA (becomes input and since it has pullup it will go high)
#define SDA_READ()  ((GPI & (1 << twi_sda)) != 0)
#define SCL_LOW()   (GPES = (1 << twi_scl))
#define SCL_HIGH()  (GPEC = (1 << twi_scl))
#define SCL_READ()  ((GPI & (1 << twi_scl)) != 0)

#define CON_ADDR 0x81


static void ICACHE_RAM_ATTR twi_delay(unsigned char v){
  unsigned int i;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  unsigned int reg;
  for (i = 0; i < v; i++) {
    reg = GPI;
  }
  (void)reg;
#pragma GCC diagnostic pop
}

unsigned short ReadKey(void)
{
  unsigned char bitnum, temp, addr;
  unsigned int key2byte;
	uint8_t bit_temp;

  addr = CON_ADDR;
  key2byte = 0xffff;

  SDA_LOW(); // 拉低SDA 端口送出START 信号
  twi_delay(5);
  //发送8 位地址字节(A[6:0]+RWB)
  for (bitnum = 0; bitnum < 8; bitnum++)
  {
    SCL_LOW();
    temp = addr & 0x80;
    if (temp == 0x80)
      SDA_HIGH();
    else
      SDA_LOW();
    addr = addr << 1;
    twi_delay(5);
    SCL_HIGH();
    twi_delay(5);
  }
  SDA_HIGH(); //释放SDA 端口,将SDA 设置为输入端口
  SCL_LOW();
  twi_delay(5);
  SCL_HIGH();
  twi_delay(5);
  bit_temp = SDA_READ();
  if (bit_temp) //读ack 回应
    return 0;
  //读16 位按键数据字节(D[15:0])
  for (bitnum = 0; bitnum < 16; bitnum++)
  {
    SCL_LOW();
    twi_delay(5);
    SCL_HIGH();
    twi_delay(5);
    bit_temp = SDA_READ();
    if (bit_temp)
    {
      key2byte = key2byte << 1;
      key2byte = key2byte | 0x01;
    }
    else
    {
      key2byte = key2byte << 1;
    }
  }
  SCL_LOW();
  SDA_HIGH();
  twi_delay(5);
  SCL_HIGH();
  twi_delay(5);
  SCL_LOW();
  SDA_LOW(); //发送NACK 信号
  twi_delay(5);
  SCL_HIGH();
  twi_delay(5);
  SDA_HIGH(); //释放SDA 端口,将SDA 设置为输入端口
  key2byte = key2byte ^ 0xffff;
  return (key2byte); //数据位为1，说明相应按键被触摸。例如，返回值为0x0500 说明按
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600); /* begin serial for debug */
  //Wire.begin(4, 5); /* join i2c bus with SDA=GPIO4 and SCL=GPIO5 */
  pinMode(twi_sda, INPUT_PULLUP); //SDA
  pinMode(twi_scl, INPUT_PULLUP); //SCL
}

void loop()
{
  // put your main code here, to run repeatedly:
  unsigned short key;

  //delay(500);
  key = ReadKey();
  if (key)
    Serial.printf("Get data 0x%0X", key);
}