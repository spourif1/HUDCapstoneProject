//////////////////////////////////////////////////////////////////////////////////
// Company: CMPE Capstone 2026-2027
// Engineer: Duncan Walsh
// 
// Create Date: 11/16/2026 11:40:31 PM
// Design Name: hall_effect
// Module Name: Hall Effect
// Project Name: HUD Unified
// Target Devices: Arduino Uno R3
// Tool Versions: Arduino IDE
// Description: A rough outline of how to use the GS101202 digital hall effect sensor in conjunction with the Onboard Computation Unit.
// 
// Dependencies: N/A
// 
// Revision: 3
// Revision 1 - File Created
// Revision 2 - Reading hall effect output
// Revision 3 - Full RPM reading functionality with edge detect and extrapolation.
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

const int hl_efct_dio_pin = 7;

int hl_efct_in;
int prev_hl_efct_in;
unsigned long prev_time_ms = 0;
unsigned long start_time = 0;
unsigned long current_time_ms = 0;
const long time_interval = 1000;
int total_lw_samples = 0;
int saved_rpm = 0;
const int TIME_SCALE_FACTOR = 1000 / time_interval;
const int SEC_TO_MIN = 60;

void HandleHallEffectSetup ()
{
  Serial.begin(9600);
  pinMode(hl_efct_dio_pin, INPUT_PULLUP);

  start_time = millis();
}

int HallEffectSample() {
  current_time_ms = millis();

  if (current_time_ms - start_time < time_interval) {
    prev_hl_efct_in = hl_efct_in;
    hl_efct_in = digitalRead(hl_efct_dio_pin);
    // POSEDGE DETECT????!!!!
    if (hl_efct_in == LOW && prev_hl_efct_in == HIGH)
    {
      total_lw_samples++;
    }  
    return saved_rpm;
  }
  else
  {
    if (total_lw_samples != 0)
      Serial.println("Total Sensor Outputs per Second: " + String(total_lw_samples) + ", Extrapolated RPM: " + String(total_lw_samples * 60));
    saved_rpm = total_lw_samples * SEC_TO_MIN * TIME_SCALE_FACTOR;
    total_lw_samples = 0;                 
    start_time = current_time_ms;   // The end time of window #1 becomes start
    return saved_rpm;
  }
}

