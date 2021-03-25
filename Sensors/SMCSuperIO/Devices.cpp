//                                                   
//  Devices.cpp                                      
//  SMCSuperIO                                       
//                                                   
//  Copyright © 2016-2019 joedm. All rights reserved.
//                                                   
//  This is an autogenerated file!                   
//  Please avoid any modifications!                  
//                                                   

#include "Devices.hpp"
#include "NuvotonDevice.hpp"
#include "FintekDevice.hpp"
#include "ITEDevice.hpp"
#include "WinbondDevice.hpp"

class GeneratedNuvotonDevice_0 : public Nuvoton::NuvotonDevice {
public:
	uint8_t getTachometerCount() override {
		return 3;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead6776(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[3] = {
		"SYSFAN",
		"CPUFAN",
		"AUXFAN0",
	};
public:
	uint8_t getVoltageCount() override {
		return 10;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead6775(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[10] = {
		"CPUVCORE",
		"VIN1",
		"AVSB",
		"3VCC",
		"VIN0",
		"VIN2",
		"VIN4",
		"3VSB",
		"VBAT",
		"VTT",
	};

};

class Device_0xB470 final : public GeneratedNuvotonDevice_0 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0xB470)
			return new Device_0xB470();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6771F";
	}

};

class GeneratedFintekDevice_1 : public Fintek::FintekDevice {
public:
	uint8_t getTachometerCount() override {
		return 3;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[3] = {
		"FAN1",
		"FAN2",
		"FAN3",
	};
public:
	uint8_t getVoltageCount() override {
		return 9;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead71808E(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[9] = {
		"VCC3V",
		"Vcore",
		"V2",
		"V3",
		"V4",
		"V5",
		"Reserved",
		"VSB3V",
		"VBAT",
	};

};

class Device_0x0901 final : public GeneratedFintekDevice_1 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x0901)
			return new Device_0x0901();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "Fintek F71808E";
	}

};

class GeneratedWinbondDevice_2 : public Winbond::WinbondDevice {
public:
	uint8_t getTachometerCount() override {
		return 3;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[3] = {
		"SYSFAN",
		"CPUFAN0",
		"AUXFAN0",
	};
public:
	uint8_t getVoltageCount() override {
		return 7;
	}

	float updateVoltage(uint8_t index) override {
		return voltageReadVrmCheck(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[7] = {
		"CPUVCORE",
		"VIN0",
		"AVCC",
		"3VCC",
		"VIN1",
		"VSB",
		"VBAT",
	};

};

class Device_0x5217 final : public GeneratedWinbondDevice_2 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x5217)
			return new Device_0x5217();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83627HF";
	}

};

class Device_0x523A final : public GeneratedWinbondDevice_2 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x523A)
			return new Device_0x523A();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83627HF";
	}

};

class Device_0x5241 final : public GeneratedWinbondDevice_2 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x5241)
			return new Device_0x5241();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83627HF";
	}

};

class Device_0x8280 final : public GeneratedWinbondDevice_2 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0x8280)
			return new Device_0x8280();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83627THF";
	}

};

class Device_0x8541 final : public GeneratedWinbondDevice_2 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8541)
			return new Device_0x8541();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83687THF";
	}

};

class GeneratedITEDevice_3 : public ITE::ITEDevice {
public:
	uint8_t getTachometerCount() override {
		return 3;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead8bit(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[3] = {
		"FAN1",
		"FAN2",
		"FAN3",
	};
public:
	uint8_t getVoltageCount() override {
		return 9;
	}

	float updateVoltage(uint8_t index) override {
		return voltageReadOld(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[9] = {
		"VIN0",
		"VIN1",
		"VIN2",
		"VIN3",
		"VIN4",
		"VIN5",
		"VIN6",
		"VIN7",
		"VBAT",
	};

};

class Device_0x8705 final : public GeneratedITEDevice_3 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8705)
			return new Device_0x8705();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8705F";
	}

};

class Device_0x8712 final : public GeneratedITEDevice_3 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8712)
			return new Device_0x8712();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8712F";
	}

};

class GeneratedITEDevice_4 : public ITE::ITEDevice {
public:
	uint8_t getTachometerCount() override {
		return 5;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[5] = {
		"FAN1",
		"FAN2",
		"FAN3",
		"FAN4",
		"FAN5",
	};
public:
	uint8_t getVoltageCount() override {
		return 9;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[9] = {
		"VIN0",
		"VIN1",
		"VIN2",
		"VIN3",
		"VIN4",
		"VIN5",
		"VIN6",
		"VIN7",
		"VBAT",
	};

};

class Device_0x8721 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8721)
			return new Device_0x8721();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8721F";
	}

};

class Device_0x8726 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8726)
			return new Device_0x8726();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8726F";
	}

};

class Device_0x8620 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8620)
			return new Device_0x8620();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8620E";
	}

};

class Device_0x8628 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8628)
			return new Device_0x8628();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8628E";
	}

};

class Device_0x8686 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8686)
			return new Device_0x8686();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8686E";
	}

};

class Device_0x8728 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8728)
			return new Device_0x8728();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8728F";
	}

};

class Device_0x8752 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8752)
			return new Device_0x8752();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8752F";
	}

};

class Device_0x8771 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8771)
			return new Device_0x8771();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8771E";
	}

};

class Device_0x8772 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8772)
			return new Device_0x8772();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8772E";
	}

};

class Device_0x8792 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8792)
			return new Device_0x8792();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8792E";
	}

};

class Device_0x8688 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8688)
			return new Device_0x8688();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8688E";
	}

};

class Device_0x8795 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8795)
			return new Device_0x8795();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8795E";
	}

};

class Device_0x8665 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8665)
			return new Device_0x8665();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8665E";
	}

};

class Device_0x8613 final : public GeneratedITEDevice_4 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8613)
			return new Device_0x8613();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8613E";
	}

};

class GeneratedNuvotonDevice_5 : public Nuvoton::NuvotonDevice {
public:
	uint8_t getTachometerCount() override {
		return 5;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[5] = {
		"SYSFAN",
		"CPUFAN",
		"AUXFAN0",
		"AUXFAN1",
		"AUXFAN2",
	};
public:
	uint8_t getVoltageCount() override {
		return 15;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[15] = {
		"CPUVCORE",
		"VIN1",
		"AVSB",
		"3VCC",
		"VIN0",
		"VIN8",
		"VIN4",
		"3VSB",
		"VBAT",
		"VTT",
		"VIN5",
		"VIN6",
		"VIN2",
		"VIN3",
		"VIN7",
	};

};

class Device_0xC560 final : public GeneratedNuvotonDevice_5 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0xC560)
			return new Device_0xC560();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6779D";
	}

};

class GeneratedNuvotonDevice_6 : public Nuvoton::NuvotonDevice {
	void onPowerOn() override {
		onPowerOn679xx();
	}

public:
	uint8_t getTachometerCount() override {
		return 7;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[7] = {
		"SYSFAN",
		"CPUFAN",
		"AUXFAN0",
		"AUXFAN1",
		"AUXFAN2",
		"AUXFAN3",
		"AUXFAN4",
	};
public:
	uint8_t getVoltageCount() override {
		return 16;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[16] = {
		"CPUVCORE",
		"VIN1",
		"AVSB",
		"3VCC",
		"VIN0",
		"VIN8",
		"VIN4",
		"3VSB",
		"VBAT",
		"VTT",
		"VIN5",
		"VIN6",
		"VIN2",
		"VIN3",
		"VIN7",
		"VIN9",
	};

};

class Device_0xD423 final : public GeneratedNuvotonDevice_6 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0xD423)
			return new Device_0xD423();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6796D";
	}

};

class Device_0xD451 final : public GeneratedNuvotonDevice_6 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0xD451)
			return new Device_0xD451();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6797D";
	}

};

class Device_0xD428 final : public GeneratedNuvotonDevice_6 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0xD428)
			return new Device_0xD428();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6798D";
	}

};

class Device_0xD42A final : public GeneratedNuvotonDevice_6 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0xD42A)
			return new Device_0xD42A();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6796D-E";
	}

};

class Device_0xD42B final : public GeneratedNuvotonDevice_6 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0xD42B)
			return new Device_0xD42B();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT679BD";
	}

};

class GeneratedWinbondDevice_7 : public Winbond::WinbondDevice {
public:
	uint8_t getTachometerCount() override {
		return 5;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[5] = {
		"SYSFAN",
		"CPUFAN0",
		"AUXFAN0",
		"CPUFAN1",
		"AUXFAN1",
	};
public:
	uint8_t getVoltageCount() override {
		return 9;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[9] = {
		"CPUVCORE",
		"VIN0",
		"AVCC",
		"3VCC",
		"VIN1",
		"VIN2",
		"VIN3",
		"VSB",
		"VBAT",
	};

};

class Device_0xA020 final : public GeneratedWinbondDevice_7 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0xA020)
			return new Device_0xA020();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83627DHG";
	}

};

class Device_0x8860 final : public GeneratedWinbondDevice_7 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0x8860)
			return new Device_0x8860();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83627EHG";
	}

};

class Device_0xB070 final : public GeneratedWinbondDevice_7 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0xB070)
			return new Device_0xB070();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83627DHGP";
	}

};

class Device_0xA510 final : public GeneratedWinbondDevice_7 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0xA510)
			return new Device_0xA510();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83667HG";
	}

};

class Device_0xB350 final : public GeneratedWinbondDevice_7 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0xB350)
			return new Device_0xB350();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83667HGB";
	}

};

class GeneratedFintekDevice_8 : public Fintek::FintekDevice {
public:
	uint8_t getTachometerCount() override {
		return 3;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[3] = {
		"FAN1",
		"FAN2",
		"FAN3",
	};
public:
	uint8_t getVoltageCount() override {
		return 9;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[9] = {
		"VCC3V",
		"Vcore",
		"V2",
		"V3",
		"V4",
		"V5",
		"V6",
		"VSB3V",
		"VBAT",
	};

};

class Device_0x0601 final : public GeneratedFintekDevice_8 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x0601)
			return new Device_0x0601();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "Fintek F71862";
	}

};

class Device_0x1106 final : public GeneratedFintekDevice_8 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x1106)
			return new Device_0x1106();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "Fintek F71868A";
	}

};

class Device_0x0814 final : public GeneratedFintekDevice_8 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x0814)
			return new Device_0x0814();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "Fintek F71869";
	}

};

class Device_0x1007 final : public GeneratedFintekDevice_8 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x1007)
			return new Device_0x1007();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "Fintek F71869A";
	}

};

class Device_0x1005 final : public GeneratedFintekDevice_8 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x1005)
			return new Device_0x1005();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "Fintek F71889AD";
	}

};

class Device_0x0909 final : public GeneratedFintekDevice_8 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x0909)
			return new Device_0x0909();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "Fintek F71889ED";
	}

};

class Device_0x0723 final : public GeneratedFintekDevice_8 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x0723)
			return new Device_0x0723();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "Fintek F71889F";
	}

};

class GeneratedITEDevice_9 : public ITE::ITEDevice {
public:
	uint8_t getTachometerCount() override {
		return 5;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[5] = {
		"FAN1",
		"FAN2",
		"FAN3",
		"FAN4",
		"FAN5",
	};
public:
	uint8_t getVoltageCount() override {
		return 9;
	}

	float updateVoltage(uint8_t index) override {
		return voltageReadOld(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[9] = {
		"VIN0",
		"VIN1",
		"VIN2",
		"VIN3",
		"VIN4",
		"VIN5",
		"VIN6",
		"VIN7",
		"VBAT",
	};

};

class Device_0x8716 final : public GeneratedITEDevice_9 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8716)
			return new Device_0x8716();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8716F";
	}

};

class Device_0x8718 final : public GeneratedITEDevice_9 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8718)
			return new Device_0x8718();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8718F";
	}

};

class Device_0x8720 final : public GeneratedITEDevice_9 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x8720)
			return new Device_0x8720();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "ITE IT8720F";
	}

};

class GeneratedFintekDevice_10 : public Fintek::FintekDevice {
public:
	uint8_t getTachometerCount() override {
		return 4;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[4] = {
		"FAN1",
		"FAN2",
		"FAN3",
		"FAN4",
	};
public:
	uint8_t getVoltageCount() override {
		return 9;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[9] = {
		"VCC3V",
		"Vcore",
		"V2",
		"V3",
		"V4",
		"V5",
		"V6",
		"VSB3V",
		"VBAT",
	};

};

class Device_0x0541 final : public GeneratedFintekDevice_10 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x0541)
			return new Device_0x0541();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x04;
	}

	const char* getModelName() override {
		return "Fintek F71882";
	}

};

class GeneratedNuvotonDevice_11 : public Nuvoton::NuvotonDevice {
public:
	uint8_t getTachometerCount() override {
		return 5;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead6776(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[5] = {
		"SYSFAN",
		"CPUFAN",
		"AUXFAN0",
		"AUXFAN1",
		"AUXFAN2",
	};
public:
	uint8_t getVoltageCount() override {
		return 10;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead6775(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[10] = {
		"CPUVCORE",
		"VIN1",
		"AVSB",
		"3VCC",
		"VIN0",
		"VIN2",
		"VIN4",
		"3VSB",
		"VBAT",
		"VTT",
	};

};

class Device_0xC330 final : public GeneratedNuvotonDevice_11 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0xC330)
			return new Device_0xC330();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6776F";
	}

};

class GeneratedNuvotonDevice_12 : public Nuvoton::NuvotonDevice {
	void onPowerOn() override {
		onPowerOn679xx();
	}

public:
	uint8_t getTachometerCount() override {
		return 6;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[6] = {
		"SYSFAN",
		"CPUFAN",
		"AUXFAN0",
		"AUXFAN1",
		"AUXFAN2",
		"AUXFAN3",
	};
public:
	uint8_t getVoltageCount() override {
		return 15;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[15] = {
		"CPUVCORE",
		"VIN1",
		"AVSB",
		"3VCC",
		"VIN0",
		"VIN8",
		"VIN4",
		"3VSB",
		"VBAT",
		"VTT",
		"VIN5",
		"VIN6",
		"VIN2",
		"VIN3",
		"VIN7",
	};

};

class Device_0xC803 final : public GeneratedNuvotonDevice_12 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0xC803)
			return new Device_0xC803();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6791D";
	}

};

class Device_0xC911 final : public GeneratedNuvotonDevice_12 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0xC911)
			return new Device_0xC911();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6792D";
	}

};

class Device_0xD121 final : public GeneratedNuvotonDevice_12 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0xD121)
			return new Device_0xD121();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6793D";
	}

};

class Device_0xD352 final : public GeneratedNuvotonDevice_12 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0xD352)
			return new Device_0xD352();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Nuvoton NCT6795D";
	}

};

class GeneratedFintekDevice_13 : public Fintek::FintekDevice {
public:
	uint8_t getTachometerCount() override {
		return 4;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[4] = {
		"FAN1",
		"FAN2",
		"FAN3",
		"FAN4",
	};
public:
	uint8_t getVoltageCount() override {
		return 0;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[0] = {
	};

};

class Device_0x0507 final : public GeneratedFintekDevice_13 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if (deviceId == 0x0507)
			return new Device_0x0507();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x02;
	}

	const char* getModelName() override {
		return "Fintek F71858";
	}

};

class GeneratedWinbondDevice_14 : public Winbond::WinbondDevice {
public:
	uint8_t getTachometerCount() override {
		return 5;
	}

	uint16_t updateTachometer(uint8_t index) override {
		return tachometerRead(index);
	}

	const char* getTachometerName(uint8_t index) override {
		if (index < getTachometerCount()) {
			return tachometerNames[index];
		}
		return nullptr;
	}

private:
	const char* tachometerNames[5] = {
		"SYSFAN",
		"CPUFAN0",
		"AUXFAN0",
		"CPUFAN1",
		"AUXFAN1",
	};
public:
	uint8_t getVoltageCount() override {
		return 10;
	}

	float updateVoltage(uint8_t index) override {
		return voltageRead(index);
	}

	const char* getVoltageName(uint8_t index) override {
		if (index < getVoltageCount()) {
			return voltageNames[index];
		}
		return nullptr;
	}

private:
	const char* voltageNames[10] = {
		"CPUVCORE",
		"VIN0",
		"AVCC",
		"3VCC",
		"VIN1",
		"VIN2",
		"VIN3",
		"VSB",
		"VBAT",
		"VIN4",
	};

};

class Device_0x8850 final : public GeneratedWinbondDevice_14 {
public:
	static SuperIODevice *createDevice(uint16_t deviceId) {
		if ((deviceId & 0xFFF0) == 0x8850)
			return new Device_0x8850();
		return nullptr;
	}

	uint8_t getLdn() override {
		return 0x0B;
	}

	const char* getModelName() override {
		return "Winbond W83627EHF";
	}

};

SuperIODevice *createDevice(uint16_t deviceId) {
	SuperIODevice *device;
	device = Device_0xB470::createDevice(deviceId);
	if (device) return device;
	device = Device_0x0901::createDevice(deviceId);
	if (device) return device;
	device = Device_0x5217::createDevice(deviceId);
	if (device) return device;
	device = Device_0x523A::createDevice(deviceId);
	if (device) return device;
	device = Device_0x5241::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8280::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8541::createDevice(deviceId);
	if (device) return device;
	device = Device_0xC560::createDevice(deviceId);
	if (device) return device;
	device = Device_0xD423::createDevice(deviceId);
	if (device) return device;
	device = Device_0xD451::createDevice(deviceId);
	if (device) return device;
	device = Device_0xD428::createDevice(deviceId);
	if (device) return device;
	device = Device_0xD42A::createDevice(deviceId);
	if (device) return device;
	device = Device_0xD42B::createDevice(deviceId);
	if (device) return device;
	device = Device_0xA020::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8860::createDevice(deviceId);
	if (device) return device;
	device = Device_0xB070::createDevice(deviceId);
	if (device) return device;
	device = Device_0xA510::createDevice(deviceId);
	if (device) return device;
	device = Device_0xB350::createDevice(deviceId);
	if (device) return device;
	device = Device_0x0601::createDevice(deviceId);
	if (device) return device;
	device = Device_0x1106::createDevice(deviceId);
	if (device) return device;
	device = Device_0x0814::createDevice(deviceId);
	if (device) return device;
	device = Device_0x1007::createDevice(deviceId);
	if (device) return device;
	device = Device_0x1005::createDevice(deviceId);
	if (device) return device;
	device = Device_0x0909::createDevice(deviceId);
	if (device) return device;
	device = Device_0x0723::createDevice(deviceId);
	if (device) return device;
	device = Device_0x0541::createDevice(deviceId);
	if (device) return device;
	device = Device_0xC330::createDevice(deviceId);
	if (device) return device;
	device = Device_0xC803::createDevice(deviceId);
	if (device) return device;
	device = Device_0xC911::createDevice(deviceId);
	if (device) return device;
	device = Device_0xD121::createDevice(deviceId);
	if (device) return device;
	device = Device_0xD352::createDevice(deviceId);
	if (device) return device;
	device = Device_0x0507::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8850::createDevice(deviceId);
	if (device) return device;
	return nullptr;
}
SuperIODevice *createDeviceITE(uint16_t deviceId) {
	SuperIODevice *device;
	device = Device_0x8705::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8712::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8721::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8726::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8620::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8628::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8686::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8728::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8752::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8771::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8772::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8792::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8688::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8795::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8665::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8613::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8716::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8718::createDevice(deviceId);
	if (device) return device;
	device = Device_0x8720::createDevice(deviceId);
	if (device) return device;
	return nullptr;
}
