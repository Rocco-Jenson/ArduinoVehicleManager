#include "vehicle_manager.h"

#define ORIGIN_MILEAGE 0

String input(const char* question, int status=1) noexcept {
  if (status) {
    Serial.println();
    Serial.println(question);
    Serial.println();
  }
  while (Serial.available() == 0) {}
  return Serial.readStringUntil('\n');
}

void PRINT_SUCCESS() noexcept {
  Serial.println("COMMAND SUCCESS");
}

std::vector<FluidState> FluidStateArray{FluidState::CreateFluidStates(ORIGIN_MILEAGE)};
VehicleManager _object;

void setup() {
  Serial.begin(115200);

  delay(2500);

  Serial.println("Starting setup...");

  _object.initializeFromEEPROM();

  if (_object.checkNotInitialized()) {
    Serial.println("Object not initialized. Filling with initial data...");
    _object.initVehicleManager("Ford Ranger 2006 3.0L V6", ORIGIN_MILEAGE, FluidStateArray);
    _object.saveToEEPROM();
    Serial.println("Initial data filled and saved to EEPROM.");
    Serial.println();
  } else {
    Serial.println("Object already initialized. Loading from EEPROM...");
    Serial.println("Data loaded from EEPROM.");
  }

  _object.checkStatus();
  _object.displayState();
  Serial.println();
  Serial.println("Setup completed.");
}
void loop() {
  String user_in = input("Input A Command -> UMS/Update Mileage Status, AN/Add Note, RN/Remove Note, SE/Save to EEPROM, CSP/Change State Parameters, DS/ Display State");
  user_in.trim();

  if ("UMS" == user_in) {
    String value = input("Update Origin Mileage Status");
    value.trim();
    _object.updateMileageStatus(value.toInt());
    _object.checkStatus();
    PRINT_SUCCESS();
  } else if ("AN" == user_in) {
    String value = input("Add a Note");
    value.trim();
    const char* cstr = value.c_str();
    _object.addNote(cstr);
    PRINT_SUCCESS();
  } else if ("RN" == user_in) {
    _object.removeTopNote();
    PRINT_SUCCESS();
  } else if ("SE" == user_in) {
    _object.saveToEEPROM();
    PRINT_SUCCESS();
  } else if ("CSP" == user_in) {
    String fluid_index = input("Insert Fluid Index");
    String origin_mileage = input("Insert New Origin Mileage");
    String new_mileage = input("Insert New Mileage");
    _object.changeStateParameters(fluid_index.toInt(), new_mileage.toInt(), origin_mileage.toInt());
    PRINT_SUCCESS();
  } else if ("DS" == user_in) {
    _object.displayState();
    PRINT_SUCCESS();
  } else {
    Serial.print("Invalid Input");
  }

  _object.checkStatus();
}
