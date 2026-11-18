int RPM = 0;
int prev_RPM = 0;

void setup() {
  HandleOLEDSetup(); 
  HandleHallEffectSetup();
  HandleOLEDUpdate(0);
}

void loop() {
  RPM = HallEffectSample();
  if (prev_RPM != RPM)
  {
    HandleOLEDUpdate(RPM);
  }
  prev_RPM = RPM;
}
