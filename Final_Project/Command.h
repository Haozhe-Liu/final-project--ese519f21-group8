char key_value = '\0';
char key_flag = '\0';
char key_mode = 0;
char prev_key_mode = 0;
unsigned long key_mode_time = 0;

void keyValue()
{
  if (millis() - key_mode_time > 500)
  {
    key_mode_time = millis();

    key_mode++;

    if (key_mode >= 5)
    {
      key_mode = 0;
    }
  }
}

bool getKeyValue()
{
  if (prev_key_mode != key_mode)
  {
    prev_key_mode = key_mode;
    switch (key_mode)
    {
    case 0:
      key_value = 's';
      function_mode = IDLE;
      break;
    case 1:
      key_value = '1';
      break;
    case 2:
      key_value = '2';
      break;
    case 3:
      key_value = '0';
      break;
    case 4:
      key_value = '3';
      break;
    default:
      break;
    }
    return true;
  }
  return false;
}

void keyInit()
{
  pinMode(KEY_MODE, INPUT_PULLUP);
  attachPinChangeInterrupt(KEY_MODE, keyValue, FALLING);
}

//bool getBluetoothData()
//{
//  if (Serial.available())
//  {
//    char c = Serial.read();
//    if (c != '\0' && c != '\n')
//    {
//      key_value = c;
//      if (c == 'f' || c == 'b' || c == 'l' || c == 'i')
//      {
//        function_mode = BLUETOOTH;
//      }
//      if (c == 's')
//      {
//        function_mode = IDLE;
//      }
//      if (key_value == 'f' || key_value == 'b' || key_value == 'l' || key_value == 'i' || key_value == 's' ||
//          key_value == '0' || key_value == '1' || key_value == '2' || key_value == '3' || key_value == '4' ||
//          key_value == '5' || key_value == '6' || key_value == '7' || key_value == '8' || key_value == '9' ||
//          key_value == '*' || key_value == '#')
//      {
//        return true;
//      }
//    }
//  }
//  return false;
//}

bool getBluetoothData()
{
//   int pin_D1 = digitalRead(11);
//   int pin_D2 = digitalRead(13);

    pinMode(TRIG_PIN, INPUT);
    int data;
    int temp1, temp2;
    temp1 = digitalRead(11);
    temp2 = digitalRead(13);
    

    Serial.print(temp1);Serial.print("\n");
    Serial.print(temp2);Serial.print("\n");
    
    if(temp1== 1 && temp2 == 1) data = 3;
    else if(temp1 == 1 && temp2 == 0) data = 2;
    else if(temp1 == 0 && temp2 == 1) data = 1;
    else data = 0;


 //   if(pin_D2 ==1 ) data = 1;
 //  else data = 0;
    Serial.print("data:");Serial.println(data);
    if (data != '\0' && data != '\n')
    { 
      switch (data)
      {

        case 1: 
        Serial.println("1 is FORWARD mode");
        key_value = 'f';
        function_mode = BLUETOOTH;
        break;

        case 0: 
        Serial.println("2 is BACKWARD mode");
        key_value = 'b';
        function_mode = BLUETOOTH;
        break;

        case 3: Serial.println("3 is STANDBY mode");
        key_value = 's';
        function_mode = IDLE;
        break;

      }
    }

    if(data == 2)
      return false;
    else if(key_value == 'f' || key_value == 'b' || key_value == 'l' || key_value == 'i' || key_value == 's' ||
          key_value == '0' || key_value == '1' || key_value == '2' || key_value == '3' || key_value == '4' ||
          key_value == '5' || key_value == '6' || key_value == '7' || key_value == '8' || key_value == '9' ||
          key_value == '*' || key_value == '#')
      {
        return true;
      }

    pinMode(TRIG_PIN, OUTPUT);
}
