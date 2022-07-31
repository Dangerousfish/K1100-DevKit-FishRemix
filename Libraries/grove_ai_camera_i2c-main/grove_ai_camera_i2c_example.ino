#include "Seeed_Arduino_GroveAI.h"
#include <Wire.h>

GroveAI ai(Wire);
uint8_t state = 0;
void setup()
{
   Wire.begin();
  Serial.begin(115200);
  delay(5000);
  
  while (!Serial)
  {
    /* code */
  }
   Serial.println("begin");
  if (ai.begin(ALGO_OBJECT_DETECTION, MODEL_EXT_INDEX_1)) // Object detection and pre-trained model 1
  //if (ai.begin(ALGO_IMAGE_CLASSIFICATION, MODEL_EXT_INDEX_2)) // image classifacation and customized model 2
  //if (ai.begin(ALGO_OBJECT_COUNT, MODEL_EXT_INDEX_3)) // object count and customized model 3
  {
   
//    Serial.printf("Version: %04X\n", ai.version());
//    Serial.printf("ID: %04X\n", ai.id());
//    Serial.printf("Algo: %d\n", ai.algo());
//    Serial.printf("Model: %d\n", ai.model());
//    Serial.printf("Confidence: %d\n", ai.confidence());
    Serial.print("Version: ");
    Serial.println(ai.version());
    Serial.print("ID: ");
    Serial.println( ai.id());
    Serial.print("Algo: ");
    Serial.println( ai.algo());
    Serial.print("Model: ");
    Serial.println(ai.model());
    Serial.print("Confidence: ");
    Serial.println(ai.confidence());
    state = 1;
  }
  else
  {
    Serial.println("Algo begin failed.");
    
  }
}

void loop()
{
  if (state == 1)
  {
    uint32_t tick = millis();
    if (ai.invoke()) // begin invoke
    {
      while (1) // wait for invoking finished
      {
        CMD_STATE_T ret = ai.state(); 
        if (ret == CMD_STATE_IDLE)
        {
          break;
        }
        delay(20);
      }

     uint8_t len = ai.get_result_len(); // receive how many people detect
   //Serial.printf("Invoke Success: %d, len: %d\n", millis() - tick, len);
     int time1 = millis() - tick; 
     Serial.print("Invoke Success: ");
     Serial.println(time1);
     Serial.print("people: ");
     Serial.println(len);
      object_detection_t data;       //get data
    //  image_classification_t data; //get data
    //  object_count_t data;         //get data
        
      for (int i = 0; i < len; i++)
      {
        Serial.println("result:detected");
        Serial.print("Detecting and calculating: ");
        Serial.println(i+1);
        ai.get_result(i, (uint8_t*)&data, sizeof(object_detection_t)); //get result
    //  Serial.print(data.target);
        Serial.print("confidence:");
       Serial.print(data.confidence);
        Serial.println();

        
      }
    }
    else
    {
      delay(1000);
      Serial.println("Invoke Failed.");
    }
  }
  else
  {
    state == 0;
  }
}
