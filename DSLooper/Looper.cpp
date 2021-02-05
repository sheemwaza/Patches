#include "daisy_patch.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;

DaisyPatch hw;

const int sampleLength = 48000*60;		//one minute of loop time should be enough for anyone
float DSY_SDRAM_BSS buffer[sampleLength];
int offset = 0; 



uint32_t triggerTime;
uint32_t interval;
uint32_t now;
bool triggered = false;

//void updateOLED();

int pulseCount =0;

//pulses per quarter note from clock
int ppq = 2;

int loopLength = 16;

int loopState = 0;

void updateOLED();

void AudioCallback(float **in, float **out, size_t size)
{
	hw.ProcessDigitalControls();

	//check for triggers
	now = System::GetNow();

	bool gateState = hw.gate_input[0].State();

    if (gateState == true){
        if (!triggered){
            interval = now-triggerTime;
            triggerTime = now;

			pulseCount++;

			//reset the count if it has been more than a second since beats
			if (interval > 1000){
				pulseCount = 0;
				offset = 0;	
			}


			
			if (pulseCount >= loopLength){
				
				pulseCount = 0;
				offset = 0;

				//if we are armed, start recording on the first beat
				if (loopState == 1){
					loopState = 2;
					
				}
				//if we are recording, start playback on the first beat
				else if (loopState == 2){
					loopState = 3;
					
				}

			}
        }
    }

	if (hw.encoder.RisingEdge()){
		
		//arm the loop recorder
		loopState = 1;
	}

	triggered = gateState;

	for (size_t i = 0; i < size; i++)
	{
		
		//if recording, write to the buffer
		if (loopState == 2){
			//copy line 1 to the buffer
			buffer[offset] = in[0][i];

		}
		
		offset++;
		if (offset >= sampleLength){
			offset = 0;
		}	

		
		//if in playback loopmode, mix the buffer into the output
		if (loopState == 3){
			out[0][i] = in[0][i] + buffer[offset];
		}
		else{
			out[0][i] = in[0][i];
		}
		
		//mirror inputs to outputs
		
		out[1][i] = in[1][i];
		out[2][i] = in[2][i];
		out[3][i] = in[3][i];
	}

	//if (updateDisplay == 1) updateOLED();
}



void updateOLED()
{
    hw.display.Fill(false);

	float arcLength = 360/loopLength;

	hw.display.DrawArc(32,32,20,0,360,1);
	hw.display.DrawArc(32,32,16,360 + 90 - arcLength/2 - 360*pulseCount/loopLength,arcLength,1);

    
	
	hw.display.SetCursor(32-5,32-5);
    std::string str  = "Looper";
    char *      cstr = &str[0];
    str = std::to_string(loopLength);
	//str = std::to_string(hw.controls[0].Process());
	hw.display.WriteString(cstr, Font_7x10, true);

	
	/*
    hw.display.SetCursor(0, 15);
    str = "BPM: ";
    hw.display.WriteString(cstr, Font_7x10, true);

    hw.display.SetCursor(70, 15);
    str = std::to_string((1000*60)/(2*interval));
    hw.display.WriteString(cstr, Font_7x10, true);

    hw.display.SetCursor(0, 30);
    str = "Count: ";
    hw.display.WriteString(cstr, Font_7x10, true);

    hw.display.SetCursor(70, 30);
    str = std::to_string(pulseCount);
    hw.display.WriteString(cstr, Font_7x10, true);

    hw.display.SetCursor(0, 45);
    str = "State: ";
    hw.display.WriteString(cstr, Font_7x10, true);

    hw.display.SetCursor(70, 45);
    str = std::to_string(loopState);
    hw.display.WriteString(cstr, Font_7x10, true);
*/
    hw.display.Update();
}


int main(void)
{
	hw.Init();
	hw.StartAdc();
	hw.StartAudio(AudioCallback);
	
	for (int i=0; i < sampleLength; i++){
		buffer[i] =0;
	}
	
	while(1) {
		
		hw.ProcessAnalogControls();
		loopLength = 33 * hw.controls[0].Process();
		updateOLED();
	}
}