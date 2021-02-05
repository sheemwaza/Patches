#include "daisysp.h"
#include "daisy_patch.h"
#include <math.h>
//#include <system.h>


using namespace daisy;
using namespace daisysp;

DaisyPatch patch;

uint32_t triggerTime;
uint32_t interval;
uint32_t now;
bool triggered = false;

bool bursting = false;
int burstsLeft;


uint32_t burstOn=99999999999;
uint32_t burstOff=99999999999;
/*
Parameter  cutoff_ctrl, res_ctrl, drive_ctrl;

static void AudioCallback(float **in, float **out, size_t size)
{


     //get new control values
    float cutoff = cutoff_ctrl.Process();
    float res    = res_ctrl.Process();
    float drive  = drive_ctrl.Process();

    float ctrlVal[4];
    for(int i = 0; i < 4; i++)
    {
        //Get the four control values
        ctrlVal[i] = patch.controls[i].Process();
    }
  
    //let's play oscilloscope
    patch.display.Fill(0);

    for(size_t i = 0; i < 128; i++){
        patch.display.DrawPixel(i,32 + 100.f*vals[i],1);
    }


    patch.display.Update();
}
*/


static void updateOLED()
{
    patch.display.Fill(false);

    std::string str  = "Kablurst";
    char*       cstr = &str[0];
    patch.display.SetCursor(0, 0);
    patch.display.WriteString(cstr, Font_6x8, true);

    uint32_t rand = myrand();
    str = std::to_string(rand);
    //str = std::to_string(ctrl0Val);
    
    //Cursor
    patch.display.SetCursor(0, 25);
    patch.display.WriteString(cstr, Font_7x10, true);
    

    patch.display.Update();
}

void updateControls(){
      patch.ProcessAnalogControls();
      patch.ProcessDigitalControls();


    float ctrlVal[4];
    for(int i = 0; i < 4; i++)
    {
        //Get the four control values
        ctrlVal[i] = patch.controls[i].Process();
    }

    int numBursts = (int)(ctrlVal[1]*10 + 1);

    bool gateState = patch.gate_input[0].State();
    dsy_gpio_write(&patch.gate_output,gateState);
    
    now = System::GetNow();

    if (gateState == true){
        if (!triggered){
            
            interval = now-triggerTime;
            triggerTime = now;
            
            float r = ((float) rand() / (RAND_MAX));
            if (r > ctrlVal[0]){
                burstsLeft = numBursts-1;
                burstOn = now + (interval/numBursts);
                burstOff = burstOn + 2;
            }
            updateOLED();
        }
    }

    if (now > burstOn && now <= burstOff){
        dsy_gpio_write(&patch.gate_output,1); 
        bursting = true;      
    }
    else if (bursting){
        bursting = false;
        burstsLeft--;
        if (burstsLeft > 0){
            burstOn = now + (interval/numBursts);
            burstOff = burstOn + 2;
        }
    }

    triggered = gateState;

}

int main(void)
{
    patch.Init(); // Initialize hardware (daisy seed, and patch)

    patch.StartAdc();

    float sampleRate;
    sampleRate = patch.AudioSampleRate();

     /*
    //setup controls
    cutoff_ctrl.Init(patch.controls[0], 20, 20000, Parameter::LOGARITHMIC);
    res_ctrl.Init(patch.controls[1], .3, 1, Parameter::LINEAR);
    drive_ctrl.Init(patch.controls[2], .3, 1, Parameter::LINEAR);
    */

    while(1)
    {
       
        updateControls();
    }
}
