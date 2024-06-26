//
//  SMIState.h
//  WMIDellSensors
//
//

#ifndef SMIState_hpp
#define SMIState_hpp

#include <stdatomic.h>

/**
 *  Aggregated Fan Info information
 */
struct FanInfo {
	/**
	 *  Unknown value
	 */
	static constexpr int32_t ValueUnknown = -1;
	
	// GET_TEMP_TYPE result codes
	enum FanType {
		Unsupported = ValueUnknown,
		CPU		= 0,
		System,
		GPU,
		PSU,
		Chipset,
		Other,
		Last
	};
	
	atomic_int		index   	 = ValueUnknown;
	atomic_int		status  	 = ValueUnknown;
	_Atomic(FanType)type    	 = Unsupported;
	atomic_int		stopOffset	 = 0;
	atomic_int		minSpeed	 = ValueUnknown;
	atomic_int		maxSpeed	 = ValueUnknown;
	atomic_int		targetSpeed  = 0;
	atomic_int		speed   	 = 0;
	atomic_uint     wmi_addr     = 0;
	atomic_bool     smm          = false;
};

/**
 *  Aggregated Temperature sensor information
 */
struct TempInfo {
	
	static constexpr int32_t ValueUnknown = -1;
	
	// GET_TEMP_TYPE result codes
	enum SMMTempSensorType {
		Unsupported = ValueUnknown,
		CPU      	= 0,
		GPU      	= 1,
		Memory   	= 2,
		Misc     	= 3,
		Ambient  	= 4,
		Other    	= 5,
		Last
	};
	
	atomic_int					index = ValueUnknown;
	_Atomic(SMMTempSensorType)	type  = Unsupported;
	atomic_int					temp  = ValueUnknown;
};

/**
 *  Aggregated SMI state
 */
struct SMIState {
	/**
	 *  A total maximum of fans supported
	 */
	static constexpr uint8_t MaxFanSupported = 6;

	/**
	 *  A total maximum of temperature sensors supported
	 */
	static constexpr uint8_t MaxTempSupported = 6;

	/**
	 * Set of fan infos
	 */
	FanInfo fanInfo[MaxFanSupported] {};

	/**
	 * Set of temp sensor infos
	 */
	TempInfo tempInfo[MaxTempSupported] {};
};

typedef enum { FAN_PWM_TACH, FAN_RPM, PUMP_PWM, PUMP_RPM, FAN_PWM_NOTACH, EMPTY_PLACEHOLDER } FanType;

typedef enum {
	LEFT_LOWER_FRONT, CENTER_LOWER_FRONT, RIGHT_LOWER_FRONT,
	LEFT_MID_FRONT,   CENTER_MID_FRONT,   RIGHT_MID_FRONT,
	LEFT_UPPER_FRONT, CENTER_UPPER_FRONT, RIGHT_UPPER_FRONT,
	LEFT_LOWER_REAR,  CENTER_LOWER_REAR,  RIGHT_LOWER_REAR,
	LEFT_MID_REAR,    CENTER_MID_REAR,    RIGHT_MID_REAR,
	LEFT_UPPER_REAR,  CENTER_UPPER_REAR,  RIGHT_UPPER_REAR
} LocationType;

static constexpr int32_t DiagFunctionStrLen = 12;

typedef struct fanTypeDescStruct {
	UInt8 type 		{FAN_PWM_TACH};
	UInt8 ui8Zone 	{1};
	UInt8 location 	{LEFT_MID_REAR};
	UInt8 rsvd		{0}; // padding to get us to 16 bytes
	char  strFunction[DiagFunctionStrLen];
} FanTypeDescStruct;

#endif /* SMIState_hpp */
