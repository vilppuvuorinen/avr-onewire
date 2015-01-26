/**
 * @def F_CPU
 *
 * F_CPU needs to be defined to time the read delays.
 * If F_CPU is defined elsewhere, add configuration
 * header here.
 */
#define F_CPU 8000000UL

/**
 * @def OW_PORT
 *
 *  Defines port used for bus.
 */
#define OW_PORT				PORTC

/**
 * @def OW_PIN
 *
 * Defines pin for port used for bus.
 */
#define OW_PIN				PINC

/**
 * @def OW_DIRECTION
 *
 * Defines direction register for port used for bus.
 */
#define OW_DIRECTION			DDRC

/**
 * @def OW_BIT
 *
 * Defines I/O pin in port used for bus.
 */
#define OW_BIT				5

/**
 * @def OW_MAX_ROMS
 *
 * Maximum number of connected devices.
 */
#define OW_MAX_ROMS			5

//#define OW_INTERNAL_PULLUP
//#define OW_BLOCK_INTERRUPTS
//#define OW_BLOCK_INTERRUPTS_BITLEVEL
