#ifndef VEHICLE_MANAGER
#define VEHICLE_MANAGER

/* EEPROM Supported Arduino Boards */
#if defined(ARDUINO_AVR_UNO) || \
    defined(ARDUINO_AVR_MEGA2560) || \
    defined(ARDUINO_AVR_NANO) || \
    defined(ARDUINO_AVR_LEONARDO) || \
    defined(ARDUINO_AVR_PRO) || \
    defined(ARDUINO_SAM_DUE) || \
    defined(ARDUINO_SAMD_ZERO) || \
    defined(ARDUINO_SAMD_MKR) || \
    defined(ARDUINO_ARCH_ESP8266) || \
    defined(ARDUINO_ARCH_ESP32)

#include <cstddef>
#include <cstring>
#include <vector>
#include <EEPROM.h>

namespace VehicleManagerNameSpace {

    /* clangd intellisense depreciated bypass */
    #if defined (__clang__)
    #define _CRT_SECURE_NO_WARNINGS
    #endif

    #define MAX_FLUID_BUFFER 7
    #define FLUID_NAME_BUFFER 31
    #define EEPROM_START_ADDRESS 0
    #define DEFAULT_ORIGIN_MILEAGE 0
    #define GENERIC_CMD "Input A Command:\nUOMS/Update Origin Mileage Status,\nUNMS Update New Mileage Status,\nAN/Add Note,\nRN/Remove Note,\nSE/Save to EEPROM,\nCSP/Change State Parameters,\nDS/ Display State,\nCE/Clear All EEPROM"

    constexpr const char* FLUID_TYPES[MAX_FLUID_BUFFER] = {
        "Engine Oil",
        "Brake Fluid",
        "Transmission Fluid",
        "Power Steering Fluid",
        "Differential Fluid",
        "Coolant",
        "Windshield Washer Fluid",
    };

    /* Reused Definition For setFluidState*/
    void panic(const char* str) noexcept {
        Serial.println(str);
        while(true){};
    }

    struct FluidState {
    private:
        const char* fluid_name;
        size_t new_mileage;

    public:
        void setFluidState(const char* fluid_name, size_t origin_mileage) noexcept {
            this->fluid_name = fluid_name;

            if (strcmp(fluid_name, "Engine Oil") == 0) {
                this->new_mileage = origin_mileage + 15000;
            } else if (strcmp(fluid_name, "Brake Fluid") == 0) {
                this->new_mileage = origin_mileage + 40000;
            } else if (strcmp(fluid_name, "Transmission Fluid") == 0) {
                this->new_mileage = origin_mileage + 80000;
            } else if (strcmp(fluid_name, "Power Steering Fluid") == 0) {
                this->new_mileage = origin_mileage + 80000;
            } else if (strcmp(fluid_name, "Differential Fluid") == 0) {
                this->new_mileage = origin_mileage + 50000;
            } else if (strcmp(fluid_name, "Coolant") == 0) {
                this->new_mileage = origin_mileage + 80000;
            } else if (strcmp(fluid_name, "Windshield Washer Fluid") == 0) {
                this->new_mileage = origin_mileage + 5000;
            } else {
                panic("fluid_name is not a valid fluid type");
            }
        }

        [[nodiscard]] static std::vector<FluidState> CreateFluidStates(size_t origin_mileage) noexcept {
            std::vector<FluidState> _struct;
            for (const auto& type : FLUID_TYPES) {
                FluidState new_struct;
                new_struct.setFluidState(type, origin_mileage);
                _struct.push_back(new_struct);
            }
            return _struct;
        }

        void changeNewMileage(size_t new_mileage) {
            this->new_mileage = new_mileage;
        }

        [[nodiscard]] size_t getNewMileage() const noexcept {
            return new_mileage;
        }

        [[nodiscard]] const char* getFluidfluid_name() const noexcept {
            return fluid_name;
        }

        void saveToEEPROM(int& address) const {
            size_t fluid_name_len = strlen(fluid_name) + 1;
            EEPROM.put(address, fluid_name_len);
            address += sizeof(fluid_name_len);
            for (size_t i = 0; i < fluid_name_len; ++i) {
                EEPROM.write(address++, fluid_name[i]);
            }
            EEPROM.put(address, (void*)new_mileage);
            address += sizeof(new_mileage);
        }

        void loadFromEEPROM(int& address) {
            size_t fluid_name_len;
            EEPROM.get(address, fluid_name_len);
            address += sizeof(fluid_name_len);
            char* temp_fluid_name = new char[fluid_name_len];
            for (size_t i = 0; i < fluid_name_len; ++i) {
                temp_fluid_name[i] = EEPROM.read(address++);
            }
            fluid_name = temp_fluid_name;
            EEPROM.get(address, (void*)new_mileage);
            address += sizeof(new_mileage);
        }
    };

    class VehicleManager {
    private:
        char vehicle_name[31];
        size_t origin_mileage;
        std::vector<FluidState> fluid_array;
        std::vector<const char*> notes_array;

    public:
        VehicleManager() : notes_array({}) {};

        void initVehicleManager(const char* fluid_name, size_t origin, const std::vector<FluidState>& arr) {
            strncpy(vehicle_name, fluid_name, sizeof(vehicle_name) - 1);
            vehicle_name[sizeof(vehicle_name) - 1] = '\0';
            origin_mileage = origin;
            fluid_array = arr;
        }

        void initializeFromEEPROM() {
            int address = EEPROM_START_ADDRESS;

            EEPROM.get(address, vehicle_name);
            address += sizeof(vehicle_name);

            EEPROM.get(address, origin_mileage);
            address += sizeof(origin_mileage);

            size_t fluid_array_size;
            EEPROM.get(address, fluid_array_size);
            address += sizeof(fluid_array_size);

            fluid_array.clear();
            for (size_t i = 0; i < fluid_array_size; ++i) {
                FluidState fluid_state;
                fluid_state.loadFromEEPROM(address);
                fluid_array.push_back(fluid_state);
            }

            size_t notes_array_size;
            EEPROM.get(address, notes_array_size);
            address += sizeof(notes_array_size);
            notes_array.clear();
            for (size_t i = 0; i < notes_array_size; ++i) {
                size_t note_len;
                EEPROM.get(address, note_len);
                address += sizeof(note_len);
                char* note = new char[note_len];
                for (size_t j = 0; j < note_len; ++j) {
                    note[j] = EEPROM.read(address++);
                }
                notes_array.push_back(note);
            }
        }

        void saveToEEPROM() const {
            int address = EEPROM_START_ADDRESS;

            EEPROM.put(address, vehicle_name);
            address += sizeof(vehicle_name);

            EEPROM.put(address, origin_mileage);
            address += sizeof(origin_mileage);

            size_t fluid_array_size = fluid_array.size();
            EEPROM.put(address, fluid_array_size);
            address += sizeof(fluid_array_size);

            for (const auto& fluid_state : fluid_array) {
                fluid_state.saveToEEPROM(address);
            }

            size_t notes_array_size = notes_array.size();
            EEPROM.put(address, notes_array_size);
            address += sizeof(notes_array_size);
            for (const auto& note : notes_array) {
                size_t note_len = strlen(note) + 1;
                EEPROM.put(address, note_len);
                address += sizeof(note_len);
                for (size_t i = 0; i < note_len; ++i) {
                    EEPROM.write(address++, note[i]);
                }
            }
        }

        void displayState() const noexcept {
            Serial.println(this->vehicle_name);
            Serial.println();
            Serial.println("FLUID STATUS:");
            
            for (const auto& _struct : this->fluid_array) {
                Serial.print(_struct.getFluidfluid_name());
                Serial.print(" Changed at: ");
                Serial.print(this->origin_mileage);
                Serial.print(" Change fluid at: "); 
                Serial.println(_struct.getNewMileage());
            }

            if (!this->notes_array.empty()) {
                Serial.println(); 
                Serial.println("FLUID STATUS:");
                Serial.println();
            }

            for (const auto& note : this->notes_array) {
                Serial.println(note);
            }
        }

        void addNote(const char* note) noexcept {
            this->notes_array.push_back(note);
        }

        void removeTopNote() noexcept {
            this->notes_array.pop_back();
        }

        void updateOriginMileage(size_t origin_mileage) noexcept {
            this->origin_mileage = origin_mileage;
        }

        void updateNewMileage(size_t fluid_struct_index, size_t new_mileage) noexcept {
            this->fluid_array[fluid_struct_index].changeNewMileage(new_mileage);
        }

        [[nodiscard]] bool checkNotInitialized() const noexcept {
            return this->vehicle_name[0] == '\0';
        }

        /* Will be changed to only clear used EEPROM Data */
        static void clearAllEEPROM() noexcept {
            for (int i = 0; i < EEPROM.length(); i++) {
                EEPROM.write(i, 0);
            }
        }

        void checkStatus() noexcept {
            for (auto& _struct : this->fluid_array) {
                if (this->origin_mileage >= _struct.getNewMileage()) {
                    Serial.print(_struct.getFluidfluid_name());
                    Serial.println(" Reached Change Date Change Fluid");
                    Serial.println();
                    _struct.setFluidState(_struct.getFluidfluid_name(), this->origin_mileage);
                }
            }
        }

        void changeStateParameters(size_t fluid_struct_index, size_t new_mileage, size_t origin_mileage=0) noexcept {
            this->fluid_array[fluid_struct_index].changeNewMileage(new_mileage);
            if (origin_mileage != 0) {
                this->origin_mileage = origin_mileage;
            }
        }
    };

    struct UserInput {
    public:
        static void panic(const char* str) noexcept {
            Serial.println(str);
            while(true){};
        }

        [[nodiscard]] static String input(const char* question, int status=1) noexcept {
            if (status) {
                Serial.println();
                Serial.println(question);
                Serial.println();
            }
            while (Serial.available() == 0) {}
            return Serial.readStringUntil('\n');
        }

        static void PRINT_SUCCESS() noexcept {
            Serial.println("COMMAND SUCCESS");
        }

        static void VehicleManagerInit(VehicleManager& _object) noexcept {
            Serial.println("Starting setup...");

            _object.initializeFromEEPROM();

            if (_object.checkNotInitialized()) {
                Serial.println("Object not initialized. Filling with initial data...");
                _object.initVehicleManager("Ford Ranger 2006 3.0L V6", DEFAULT_ORIGIN_MILEAGE, FluidStateArray);
                _object.saveToEEPROM();
                Serial.println("Initial data filled and saved to EEPROM");
                Serial.println();
            } else {
                Serial.println("Object already initialized. Loading from EEPROM...");
                Serial.println("Data loaded from EEPROM");
            }

            _object.checkStatus();
            _object.displayState();
            Serial.println();
            Serial.println("Setup completed.");
        }

        static void VehicleManagerCMD(String user_in, VehicleManager& _object) noexcept {
            if ("UOMS" == user_in) {
                String value = input("Update Origin Mileage Status");
                value.trim();
                _object.updateOriginMileage(value.toInt());
                _object.checkStatus();
                PRINT_SUCCESS();
            } else if ("UNMS") {
                String value1 = input("Select Fluid Index: 1 = EO, 2 = BF, 3 = TF, 4 = PSF, 5 = DF, 6 = C, 7 = WWF");
                String value2 = input("Update New Mileage Status");
                value1.trim(); value2.trim();
                _object.updateNewMileage(value1.toInt(), value2.toInt());
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
            } else if ("CE") {
                VehicleManager::clearAllEEPROM();
            } else {
                Serial.print("Invalid Input");
            }
        }
    };
}
#endif
#endif
