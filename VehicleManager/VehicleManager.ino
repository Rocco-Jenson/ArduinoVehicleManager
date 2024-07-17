#include "vehicle_manager.h"

#define DEFAULT_BAUD_RATE 115200
#define ARDUINO_DELAY 2500
#define DEFAULT_ORIGIN_MILEAGE 0

using namespace VehicleManagerNameSpace;

std::vector<FluidState> FluidStateArray{FluidState::CreateFluidStates(DEFAULT_ORIGIN_MILEAGE)};
VehicleManager _object;

void setup() {
  Serial.begin(DEFAULT_BAUD_RATE);
  delay(ARDUINO_DELAY);
  UserInput::VehicleManagerInit(_object);
}

void loop() {
  String user_in = UserInput::input(GENERIC_CMD);
  user_in.trim();
  UserInput::VehicleManagerCMD();
  _object.checkStatus();
}
