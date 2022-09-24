
#include <RH_RF95.h>
#include <CRC8.h>

RH_RF95 radio(RFM95_CS, RFM95_INT);
bool radioInit = false;
CRC8 crc;

bool isRadioInitialized()
{
  return radioInit;
}

// configure and test LoRa radio
bool setupRadio()
{
  // setup and test radios
  pinMode(RFM95_CS, INPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(500);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  uint8_t count = 0;
  while (!radio.init())
  {
    if (count++ > 10)
    {
      return false;
    }

    delay(100);
  }

  // setup freq and power
  radio.setFrequency(RF95_FREQ);
  radio.setTxPower(23, false);

  radioInit = true;
  return true;
}

// check for available data and then try to read messages
// (validates message fields `protoId` and `crc`; `crc` should be last byte)
template <typename Message>
uint8_t recvRadioMsg(Message &_msg, uint32_t _protoId)
{
  if (radio.available() == true)
  {
    uint8_t n = sizeof(Message);
    bool bSuccess = radio.recv((uint8_t*)&_msg, &n);
    if (bSuccess == true && _msg.protoId == _protoId)
    {
      // success on 0 CRC for debugging
      if (!_msg.crc)
      {
        return n;
      }
      // check CRC
      else
      {
        crc.reset();
        crc.add((uint8_t*)&_msg, n-1);
        if (_msg.crc == crc.getCRC())
        {
          return n;
        }
      }
    }
  }
      
  return 0;
}

// sends message
// (creates CRC; `crc` field should be last byte)
template <typename Message>
void sendRadioMsg(Message &_msg)
{
  // calc CRC
  crc.reset();
  crc.add((uint8_t*)&_msg, sizeof(Message)-1);
  _msg.crc = crc.getCRC();
  
  // send message
  radio.send((uint8_t*)&_msg, sizeof(Message));
}


