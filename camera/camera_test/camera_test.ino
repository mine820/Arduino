#include <SoftwareSerial.h>

#define PIC_PKT_LEN    128        //data length of each read, dont set this too big because ram is limited
#define PIC_FMT_VGA    7  // 640x480
#define PIC_FMT_CIF    5  // 320x240
#define PIC_FMT_OCIF   3  // 160x128
#define CAM_ADDR       0
#define CAM_SERIAL     softSerial

#define PIC_FMT        PIC_FMT_OCIF

SoftwareSerial softSerial(5, 6);  //rx=D5,tx=D6

const byte cameraAddr = (CAM_ADDR << 5);  // addr
unsigned long picTotalLen = 0;            // picture length

/*********************************************************************/
void setup() {
  Serial.begin(115200);
  CAM_SERIAL.begin(9600);       //cant be faster than 9600, maybe difference with diff board.

  initialize();
}
/*********************************************************************/
void loop()
{
  preCapture();
  while(1){
    delay(1000);
    Capture();
    //Serial.print("Saving picture...");
    GetData();
  }
}
/*********************************************************************/
void sendCmd(byte cmd[], int cmd_len)
{
  CAM_SERIAL.write(cmd, cmd_len); 
}
/*********************************************************************/
int readBytes(byte *dest, int len, unsigned int timeout)
{
  int read_len = 0;
  unsigned long t = millis();
  while (read_len < len)
  {
    while (CAM_SERIAL.available()<1)
    {
      if ((millis() - t) > timeout)
      {
        //Serial.println("timeout...");
        return read_len;
      }
    }
    *(dest+read_len) = CAM_SERIAL.read();
    //Serial.write(*(dest+read_len));
    read_len++;
  }
  //Serial.println(read_len);
  return read_len;
}
/*********************************************************************/
void initialize()
{   
  byte cmd[] = {0xaa,0x0d|cameraAddr,0x00,0x00,0x00,0x00} ;  
  byte resp[6];

  //Serial.print("initializing camera...");

  while (1) 
  {
    // Send "SYNC"
    //Serial.println("Waffle[SYNC]->Camera");
    sendCmd(cmd,6);
    // Read "ACK"
    if (readBytes(resp, 6,1000) != 6)
    {
      //Serial.print(".");
      continue;
    }
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0) 
    {
      //Serial.println("Camera[ACK]->Waffle");
      // Read "SYNC"
      if (readBytes(resp, 6, 500) != 6){
        continue; 
      }
      if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0){
        //Serial.println("Camera[SYNC]->Waffle");
        break; 
      }
    }
  }  
  // Send "ACK"
  //Serial.println("Waffle[ACK]->Camera");
  cmd[1] = 0x0e | cameraAddr;
  cmd[2] = 0x0d;
  sendCmd(cmd, 6); 

  Serial.println("\nCamera initialization done.");
}
/*********************************************************************/
void clearRxBuf()
{
  while (CAM_SERIAL.available()) 
  {
    CAM_SERIAL.read(); 
  }
}
/*********************************************************************/
void preCapture()
{
  // Initial "JPEG", 160x128
  char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT };  
  unsigned char resp[6]; 
  
  while (1)
  {
    clearRxBuf();
    // Send "Initial"
    //Serial.println("Waffle[Initial]->Camera");
    sendCmd(cmd, 6);
    // Read "ACK"
    if (readBytes((char *)resp, 6, 100) != 6) continue; 
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0)
    {
      //Serial.println("Camera[ACK]->Waffle");
      break; 
    }
  }

  Serial.println("\nPre-Capture done.");
}
/*********************************************************************/
void Capture()
{
  char cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN>>8) & 0xff ,0}; 
  unsigned char resp[6];

  while (1)
  {
    clearRxBuf();
    // Send "Set Package Size"
    //Serial.println("Waffle[Set Package Size]->Camera");
    sendCmd(cmd, 6);
    // Read "ACK"
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0){
      //Serial.println("Camera[ACK]->Waffle");
      break; 
    }
  }

  cmd[1] = 0x05 | cameraAddr;
  cmd[2] = 0x01;
  cmd[3] = 0;
  cmd[4] = 0;
  cmd[5] = 0; 
  while (1)
  {
    clearRxBuf();
    // Send "Snapshot"
    //Serial.println("Waffle[Snapshot]->Camera");
    sendCmd(cmd, 6);
    // Read "ACK"
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0){
      //Serial.println("Camera[ACK]->Waffle");
      break;
    }
  }

  cmd[1] = 0x04 | cameraAddr;
  cmd[2] = 0x1;
  while (1) 
  {
    clearRxBuf();
    // Send "Get Picture"
    //Serial.println("Waffle[Get Picture]->Camera");
    sendCmd(cmd, 6);
    // Read "ACK"
    if (readBytes((char *)resp, 6, 100) != 6) continue;
    if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
    {
      //Serial.println("Camera[ACK]->Waffle");
      // Read "Data"
      if (readBytes((char *)resp, 6, 1000) != 6) continue;
      if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
      {
        //Serial.println("Camera[Data]->Waffle");
        picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16); 
        Serial.print("picTotalLen:");
        Serial.println(picTotalLen);
        break;
      }
    }
  }
}
/*********************************************************************/
void GetData()
{
  //...
}
