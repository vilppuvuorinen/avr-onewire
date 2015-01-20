/**
 * @file onewire.h
 *
 * @author Vilppu Vuorinen
 *
 * (c) 2013-2015, Vilppu Vuorinen
 *
 */

/**
 * @def OW_SEARCH_ROM
 *
 * Search rom command, 0xF0
 */
#define OW_SEARCH_ROM			0xF0
/**
 * @def OW_MATCH_ROM
 *
 * Match rom command, 0x55
 */
#define OW_MATCH_ROM			0x55
/**
 * @def OW_CONVERT_T
 *
 * Convert T command, 0x44
 */
#define OW_CONVERT_T			0x44
/**
 * @def OW_READ_SCRATCHPAD
 *
 * Read Scratchpad command, 0xBE
 */
#define OW_READ_SCRATCHPAD		0xBE
/**
 * @def OW_SKIP_ROM
 *
 * Skip rom command, 0xCC
 */
#define OW_SKIP_ROM			0xCC

/**
 * @def OW_RESET_DELAY
 *
 * Defines the length of reset pulse. Specified
 * as 500 us by Dallas.
 */
#define OW_RESET_DELAY			500
/**
 * @def OW_LONG_DELAY
 *
 * Defines the longer delay used for r/w operations.
 * Single read or write operation lasts 65 us consisting
 * of shorter 5 us and longer 60 us delay.
 */
#define OW_LONG_DELAY			60
/**
 * @def OW_SHORT_DELAY
 *
 * Defines the shorter delay used for r/w operations.
 * Single read or write operation lasts 65 us consisting
 * of shorter 5 us and longer 60 us delay.
 */
#define OW_SHORT_DELAY			5
/**
 * @def OW_SAMPLE_DELAY
 *
 * Defines sample delay for read operation. This is
 * not accurately defined but can vary from 0 us to 10 us.
 */
#define OW_SAMPLE_DELAY			5

typedef struct OwContexts {
	uint8_t roms[OW_MAX_ROMS][8];
} OwContext;

void OwInit(void);
uint8_t OwReset(void);

uint8_t OwReadByte(void);

void OwWriteByte(uint8_t data);
void OwWriteByteTo(uint8_t data, uint8_t rom);

uint8_t OwSearchRom(void);

uint8_t OwRoms(uint8_t value, uint8_t rom, uint8_t index, uint8_t operation);
