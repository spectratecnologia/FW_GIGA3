/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FUNCTIONS_H
#define __FUNCTIONS_H

/* Includes ------------------------------------------------------------------*/
#ifdef USE_STDPERIPH_DRIVER
#include "stm32f10x.h"
#else
#include "stdint.h"
inline uint32_t Crc32(uint32_t Crc, uint32_t Data){
  int i;
  Crc = Crc ^ Data;
  for(i=0; i<32; i++)
    if (Crc & 0x80000000)
      Crc = (Crc << 1) ^ 0x04C11DB7; // Polynomial used in STM32
    else
      Crc = (Crc << 1);
  return(Crc);
}

inline uint32_t Crc32Fast(uint32_t Crc, uint32_t Data){
  static const uint32_t CrcTable[16] = { // Nibble lookup table for 0x04C11DB7 polynomial
    0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
    0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD };

  Crc = Crc ^ Data; // Apply all 32-bits
  // Process 32-bits, 4 at a time, or 8 rounds
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; // Assumes 32-bit reg, masking index to 4-bits
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28]; //  0x04C11DB7 Polynomial used in STM32
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  Crc = (Crc << 4) ^ CrcTable[Crc >> 28];
  return(Crc);
}

inline uint32_t CRC_CalcBlockCRC(uint32_t* pBuffer, uint32_t bufferLength){
    uint32_t crc = 0xFFFFFFFF;
    for(uint32_t i=0;i<bufferLength;i++){
        crc = Crc32Fast(crc, *(pBuffer+i));
    }
    return crc;
}
#endif

/* Exported types ------------------------------------------------------------*/
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */


typedef struct{
	/* Beep vars */
	uint16_t beepPeriodInMs;
	uint16_t numberOfBeeps;
	/* Enough for 136 years */
	uint32_t startTime;
}BlinkPeriod;

#define TypeSize uint8_t

/* Enum definitions */
typedef enum  {
    FUNCTION_TYPE_ACTIVATE_ON_CONDITION = 1
} FunctionType;

typedef enum  {
    INPUT_TYPE_IO = 1, INPUT_TYPE_KEY = 2
} InputType;

typedef enum  {
    OUTPUT_TYPE_IO = 1, OUTPUT_TYPE_WARNING_LIGHTS = 2,
    OUTPUT_TYPE_BEEP = 3, OUTPUT_TYPE_NEXT_VIRTUAL_KEY = 4,
    OUTPUT_TYPE_MEMORY = 5, OUTPUT_TYPE_LCD_MENU = 6,
    OUTPUT_TYPE_TIMER = 7,
} OutputType;

typedef enum {
	CONDITION_TYPE_INPUT = 1, CONDITION_TYPE_OUTPUT = 2,
	CONDITION_TYPE_MEMORY = 3, CONDITION_TYPE_TIMER = 4
} ConditionType;

#define COMPARE_VALEU_IS_INDEX 0x80

typedef enum {
	OPERATION_TYPE_AND = 1, OPERATION_TYPE_OR = 2, OPERATION_TYPE_LESS = 3, OPERATION_TYPE_LESS_OR_EQUAL = 4,
	OPERATION_TYPE_GREATER = 5, OPERATION_TYPE_GREATER_OR_EQUAL = 6, OPERATION_TYPE_EQUAL = 7,
	/* MEMORY OUPUT ONLY */
	OPERATION_COPY_INPUT_TO_MEMORY = 64,
	OPERATION_COPY_VALUE_TO_MEMORY = 65
}OperationType;

typedef enum {
	PORT_TYPE_PUSH_PULL_10A = 1, PORT_TYPE_BIDI_10A = 2, PORT_TYPE_BIDI_2A = 3,
	PORT_TYPE_LO_DIGITAL_INPUT = 4
}PortType;

#define LOG_MASK_TYPE 0xF0000000
/* Log Mask to Mpx port type */
#define LOG_MASK_MPX_LAST_VALUE 0x0000F000
#define LOG_MASK_MPX_NUMBER 0x000000F00
#define LOG_MASK_PORT_NUMBER 0x000000FF

typedef enum {
	LOG_TYPE_UNUSED 			= 0x00,
	LOG_TYPE_MPX_PORT 			= 0x01,
	LOG_TYPE_TIME_CHANGE        = 0x02,
	LOG_TYPE_CAN2_DATA 			= 0x03,
	LOG_TYPE_USB_DOWNLOAD 		= 0x04,
	LOG_TYPE_MEMORY_VALUE       = 0x05,
	/* Empty log types identifier */
	LOG_TYPE_BIT6_RESERVED 		= 0x40,
	LOG_TYPE_EVENT_DRIVEN_MASK 	= 0x80
}LogTypeMasks;

typedef struct {
	uint8_t mode;
	uint8_t duty;
	uint8_t fastSoftUp;
	uint8_t fastSoftDown;
	uint8_t ton;
	uint8_t toff;
	uint8_t slowSoftUp;
	uint8_t slowSoftDown;
} PortParameter;

#define NUM_PORTS 48
#define STRING_DESCRIPTION_SIZE 16

#define INPUT_HIGH_MASK 0x01
#define INPUT_LOW_MASK 0x02
#define INPUT_STATUS_MASK 0x04
#define INPUT_OPEN_MASK 0x08
#define INPUT_SHORT_MASK 0x10

#define FLAG_IGNORE_OPEN_MASK 0x01
#define FLAG_IGNORE_SHORT_MASK 0x02

/* Struct definitions */
typedef struct{
	uint8_t mpxId;
	uint8_t numPorts;

	char mpxName[STRING_DESCRIPTION_SIZE];
	char portName[NUM_PORTS][STRING_DESCRIPTION_SIZE];
	uint8_t portType[NUM_PORTS];

	PortParameter portOutput[NUM_PORTS];
	uint8_t outputChanged[NUM_PORTS];
	PortParameter portEmeIgn[NUM_PORTS];
	PortParameter portEmeNIgn[NUM_PORTS];

	uint8_t portInput[NUM_PORTS];

	/* Mudar contagem de erro */
	uint8_t lastPortInput[NUM_PORTS];

	uint8_t portFlags[NUM_PORTS];
	/* Count MPX timeout */
	uint64_t lastTimeSeen;

	/* Used to configure MPX emergency */
	uint8_t ackIndex;
	bool ackReceived;
}Mpx;


typedef struct  {
    TypeSize inputType;
    union{
    	struct{
    		uint8_t keyNum;
			uint8_t keyState;
    	};
    	struct{
    		uint8_t mpxIndex;
    		uint8_t portNum;
    		uint8_t portValue;
    	};
	};
} Input;


enum LocationToOutput {LOCATION_BOTH_MASK=0x03, LOCATION_MASTER_MASK=0x02,LOCATION_SLAVE_MASK=0x01};

/*
 * Highest Priority - PRIORITY_0
 * Lowest Priority - PRIORITY_15
 */
enum PriorityToDisplayMsg {PRIORITY_0=0x00, PRIORITY_1=0x10, PRIORITY_2=0x20, PRIORITY_3=0x30, PRIORITY_4=0x40, PRIORITY_5=0x50,
							PRIORITY_6_IGNORE_ALL_NO_ERROR_MESSAGES=0x60, PRIORITY_7=0x70, PRIORITY_8=0x80, PRIORITY_9=0x90, PRIORITY_10=0xA0, PRIORITY_11=0xB0,
							PRIORITY_12=0xC0, PRIORITY_13=0xD0, PRIORITY_14=0xE0, PRIORITY_15=0xF0};

typedef struct  {
    TypeSize operationType;
    union{
    	/* CONDITION_TYPE_MEMORY */
    	struct{
    		uint16_t memoryLeftOperandIndex;
    		int32_t compareValue;
    	};
    	struct {
    		uint16_t timerIndex;
    		int32_t timeout;
    	};
    	/* OUTPUT_TYPE_MEMORY */
    	struct{
    		uint16_t memoryDestIndex;
    		uint8_t mpxIndex;
    		uint8_t portNum;
    	};
    	struct{
			uint16_t memoryIndex;
			int32_t memoryValue;
		};
    };
} MemoryOperation;


typedef struct {
    TypeSize outputType;
	union{
		struct{
			uint8_t mpxIndex;
			uint8_t portNum;
			PortParameter portParameter;
		};
		struct{
			uint8_t ptcIndex;
			uint8_t warningLight;
			uint8_t warningLightValue;
		};
		struct {
		    /* Beep vars */
			uint16_t beepPeriodInMs;
			uint16_t numberOfBeeps;

			uint8_t locationToBeep;
		};
		struct {
			uint8_t nextVirtualKeyNum;
		};
		struct {
			uint32_t LCD_vState;
		};
		struct {
			/*
			 * startTimer == 0 - Stop Timer
			 * startTimer == 1 - Start Timer
			 */
			uint8_t startTimer;
			uint8_t timerIndex;
		};
		MemoryOperation memoryOutput;
	};
	uint32_t delayToExecute;
} Output;


typedef struct {
    TypeSize conditionType;
    TypeSize operationType;
	uint8_t negate;
	union{
	    Input input;
	    Output output;
	    MemoryOperation memoryCondition;
	};
} Condition;

#define WARNING_MSG_PRIORITY_MASK 0xF0
#define WARNING_MSG_LOCATION_MASK 0x03

typedef struct {
    size_t outputs_count;
    Output outputs[5];

    size_t conditions_count;
    Condition conditions[5];

    /* 0bxxxx_xxx1 - Enable slave
     * 0bxxxx_xx1x - Enable Master
     *
     * 0b0000_xxxx - Highest priority
     * 0b1111_xxxx - Lowest priority
     */
    uint8_t warningMessageConfig;
    char warningMessage[STRING_DESCRIPTION_SIZE];

    bool lastConditionValue;

    /* This flags changes execution triggered by condition edge to triggered by condition signal */
    bool alwaysExecuteOnCondition;
} ActivateOnCondition;

typedef struct {
    TypeSize functionType;
    union {
		ActivateOnCondition activateOnCondition;
	};    
    /* Logical group - Used only in Multiplex Manager */
    uint16_t logicalGroup;
} Function;


#define NUM_MPX 6
#define NUM_FUNCTIONS 120

#define NUM_PTC_KEY 64
#define NUM_PTC_WARNING_LIGHTS 16

#define NUM_PTC_MEMORY_CELL 512

#define NUM_ANALOG_MPX_VALUES 16

#define MPX_ANALOG_VBB1_OFFSET 0
#define MPX_ANALOG_VBB2_OFFSET 1
#define MPX_ANALOG_VBB3_OFFSET 2
#define MPX_ANALOG_NTC_OFFSET 3
#define MPX_ANALOG_INTERNEL_TEMP_OFFSET 4

#define TEMPERATURE_HALF_DEADBAND 1

#define NUM_LOG_SLOTS 50

typedef enum{
	MEMORY_INDEX_D_PLUS=0, MEMORY_INDEX_PLUS_15=1, MEMORY_INDEX_PLUS_30=2,
	MEMORY_INDEX_PLUS_50=3, MEMORY_INDEX_PLUS_58=4,
	MEMORY_INDEX_VELOCITY=5, MEMORY_INDEX_PLUS_56A=6,

	/* Keep Calefacion block memory sequential */
	MEMORY_INDEX_CALEFACION_0_TURN_ON = 7,
	MEMORY_INDEX_CALEFACION_0_TURN_OFF = 8,
	MEMORY_INDEX_CALEFACION_1_TURN_ON = 9,
	MEMORY_INDEX_CALEFACION_1_TURN_OFF = 10,
	/* End of Calefacion block memory */

	/* Tempreature System parameters */
	MEMORY_INDEX_COUPLED_TEMPERATURE = 11,
	MEMORY_INDEX_NUM_TEMPERATURE_ON_DISPLAY = 12,
	MEMORY_INDEX_MPX_SENSOR_TEMP_0 = 13,
	MEMORY_INDEX_MPX_SENSOR_TEMP_1 = 14,
	/* End of temperature system parameters */

	/* 16 Timers */
	/* TIMER == 0 indicates timer stopped. */
	MEMORY_INDEX_TIMER_0		= 15,
	MEMORY_INDEX_TIMER_0_MSB	= 16,
	MEMORY_INDEX_TIMER_1 		= 17,
	MEMORY_INDEX_TIMER_1_MSB 	= 18,
	MEMORY_INDEX_TIMER_2 		= 19,
	MEMORY_INDEX_TIMER_2_MSB 	= 20,
	MEMORY_INDEX_TIMER_3 		= 21,
	MEMORY_INDEX_TIMER_3_MSB 	= 22,
	MEMORY_INDEX_TIMER_4 		= 23,
	MEMORY_INDEX_TIMER_4_MSB 	= 24,
	MEMORY_INDEX_TIMER_5 		= 25,
	MEMORY_INDEX_TIMER_5_MSB 	= 26,
	MEMORY_INDEX_TIMER_6 		= 27,
	MEMORY_INDEX_TIMER_6_MSB 	= 28,
	MEMORY_INDEX_TIMER_7 		= 29,
	MEMORY_INDEX_TIMER_7_MSB 	= 30,
	MEMORY_INDEX_TIMER_8 		= 31,
	MEMORY_INDEX_TIMER_8_MSB 	= 32,
	MEMORY_INDEX_TIMER_9 		= 33,
	MEMORY_INDEX_TIMER_9_MSB 	= 34,
	MEMORY_INDEX_TIMER_10 		= 35,
	MEMORY_INDEX_TIMER_10_MSB 	= 36,
	MEMORY_INDEX_TIMER_11 		= 37,
	MEMORY_INDEX_TIMER_11_MSB	= 38,
	MEMORY_INDEX_TIMER_12 		= 39,
	MEMORY_INDEX_TIMER_12_MSB 	= 40,
	MEMORY_INDEX_TIMER_13 		= 41,
	MEMORY_INDEX_TIMER_13_MSB 	= 42,
	MEMORY_INDEX_TIMER_14 		= 43,
	MEMORY_INDEX_TIMER_14_MSB 	= 44,
	MEMORY_INDEX_TIMER_15 		= 45,
	MEMORY_INDEX_TIMER_15_MSB 	= 46,
	MEMORY_INDEX_TIMER_16	 	= 47,
	MEMORY_INDEX_TIMER_16_MSB 	= 48,
    /* End of Timers */

	MEMORY_INDEX_TEST_STEP = 49,

	MEMORY_INDEX_MPX_NUM_PORTS_ON_START = 128,
	MEMORY_INDEX_MPX_NUM_PORTS_ON_END = MEMORY_INDEX_MPX_NUM_PORTS_ON_START + NUM_MPX,

	MEMORY_INDEX_MPX_START = 256,
	MEMORY_INDEX_MPX_END = MEMORY_INDEX_MPX_START + NUM_MPX*NUM_ANALOG_MPX_VALUES
}MemoryIndex;

#define KEY_ATTRIBUTE_IGNITION_NEEDED_MASK 0x01
#define KEY_ATTRIBUTE_ONLY_ON_MASTER_MASK 0x02

typedef struct {
	uint8_t logType; //{isEventDriven, reserved, 6'b{logType}} - bit7: isEventDrive, bit6: reserved, bit[5:0]:logType
	union {
		struct {
			uint8_t mpxNum;
			uint8_t portNum;
		};
		struct {
			uint16_t can2Param;
		};
		struct {
			uint16_t memoryIdx;
		};
	};
	int32_t lastValue;
	uint32_t timeElapsed;
	int32_t sampleTimeInMs;
}Log;

typedef struct {
	uint8_t keyState[NUM_PTC_KEY];
	uint8_t numState[NUM_PTC_KEY];

	/* Not used, for future */
	uint32_t keyDeadTime[NUM_PTC_KEY];

	uint8_t warningLight[NUM_PTC_WARNING_LIGHTS];
	BlinkPeriod keyBlink[NUM_PTC_KEY];
	int32_t memory[NUM_PTC_MEMORY_CELL];

	uint8_t keyAttributes[NUM_PTC_KEY];

	/* Not used - reserved for future
	 * mapToVirtualKey[HARDWARE_KEY] = keyState index */
	uint8_t mapToVirtualKey[NUM_PTC_KEY];
	uint8_t mapToVirtualKeySlave[NUM_PTC_KEY];
}Ptc;

typedef struct {
    /* uint32_t to simplify writing to flash */
    uint32_t isMaster;
    char busName[13];
	/************* Calculated in CRC ****************/
    uint32_t multiplexConfigVersion;
	Ptc currentPtc;
	uint8_t numberOfConfiguredMpx;
	Mpx mpx[NUM_MPX];
    uint8_t numberOfConfiguredFunctions;
    BlinkPeriod beepBlink;
    /* Multiplex size has to be multiple of 4. padding is always 0 */
    char busStructDesigner[STRING_DESCRIPTION_SIZE];
    char configName[STRING_DESCRIPTION_SIZE];
    uint32_t configVersion;
    struct{
    	uint8_t day,month,year;
    }configLastDateSaved;

    Log logs[NUM_LOG_SLOTS];
    char _reserved[274];

    Function functions[NUM_FUNCTIONS];
    uint8_t padding[3];
    /************* Calculated in CRC ****************/

    uint32_t crc;

} Multiplex;

#pragma pack(pop)  /* push current alignment to stack */

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/


/* Exported functions ------------------------------------------------------- */
extern Multiplex multiplex;

static inline uint32_t crcMultiplex(Multiplex *multiplex){
	#ifdef USE_STDPERIPH_DRIVER
	CRC_ResetDR();
	#endif
    uint32_t *multiplexStart = ((uint32_t*)(multiplex))+(sizeof(multiplex->isMaster)/4);
    uint32_t multiplexCRCSize = (sizeof(Multiplex)-sizeof(multiplex->crc)-sizeof(multiplex->isMaster))/4;
    return  CRC_CalcBlockCRC(multiplexStart, multiplexCRCSize);
}

void processFunctions();

inline void nextVirtualKeyState(uint8_t key, bool beep);
inline void setVirtualKeyState(uint8_t key, uint8_t state);
inline uint8_t getVirtualKeyState(uint8_t key);

inline void processKeyBlink();
//void setBeep(uint16_t beepPeriodInMs, uint16_t numberOfBeeps);
inline void processBeepBlink();
inline void processWarningLights();
inline void sendChangedOutputsToMPXs();

void printAnalogInputs();
void printPTCanalogValuesInputs();
void printNumberOfInputOnByMPX();

static inline uint16_t getMpxAnalogMemoryAddress(uint8_t mpxIndex, uint8_t analogValue){
	return MEMORY_INDEX_MPX_START + mpxIndex * NUM_ANALOG_MPX_VALUES + analogValue;
}

void markEveryFunctionAsNotExecuted();

#endif /* __FUNCTIONS_H */
