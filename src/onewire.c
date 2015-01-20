/**
 * @file onewire.c
 *
 * @author Vilppu Vuorinen
 *
 * @mainpage
 *
 * @section intro_sec Introduction
 * <p>This is an universal 1-Wire master library for AVR MCUs based on unfinished ds1820 library by Ilari Nummila, Olli-Pekka Korpela and Jukka Pitkänen.</p>
 * <p>The rom search function supports up to 31 conflicts which results in guaranteed support for 32 devices. The theoretical maximum amount of devices is 2^32 but the actual amount is up to luck. This will not cause any problems due to signal integrity restrictions. OW_MAX_ROMS defines the maximum amount of devices. Memory is always reserved for the maximum amount of devices.</p>
 * @section Connections
 * <p>1-Wire bus can be connected with external pull-up resistor to Vcc or using internal pull-up. Internal pull-ups cannot power devices operating on parasitic power or drive a bus with multiple externally powered devices. Internal pull-ups are selected by defining constant OW_INTERNAL_PULLUP. I/O pin is selected with OW_PORT, OW_PIN, OW_DIRECTION and OW_BIT.</p>
 * @section Usage
 * <p>OW_MAX_ROMS and F_CPU in header file have to be set up according to application. OwInit() is called to initialize the bus. If multiple devices are connected OwSearchRom() function is called to search device roms. This allows addressing devices with rom index.</p>
 * <p>From this point writing commands to devices with OwWriteByteTo() and reading responses with OwReadByte() is fairly straight forward procedure.</p>
 * @section int_comp Interrupt compatibility
 * <p>1-Wire read and write operations are time-sensitive and prone to failure if interrupts are being handled simultaneously with either type of operation. By default Interrupt Service Routines written in C tend to free a couple of registers and store SREG before even executing any user-written code. This sums up to 16 cycles (2 us with 8 MHz clock) of PUSH, POP, IN, OUT and CLR calls without the user-written interrupt handling. The shortest delay used in this library is 5 us long. This basically rules out all interrupts.</p>
 * <p> Interrupt handling during reset, read and write can be prevented by defining constant OW_BLOCK_INTERRUPTS which disables interrupts for the duration of the operation. OW_BLOCK_INTERRUPTS_BITLEVEL allows interrupts between separate bits and outside the wait time for presence pulse after reset pulse.</p>
 */

#include <avr/io.h>
#include "conf.h"
#include <util/delay.h>
#include "onewire.h"

/**
 * @fn static inline void OwWriteBusHigh(void)
 * @brief A static function for writing the bus to high state. Basically just releases the bus. If OW_INTERNAL_PULLUP is defined the internal pull-up resistor is used.
 */
static inline void OwWriteBusHigh(void) {

	OW_DIRECTION &= ~(1 << OW_BIT);
	#ifdef OW_INTERNAL_PULLUP
		OW_PORT |= 1 << OW_BIT;
	#endif

}

/**
 * @fn static inline void OwWriteBusLow(void)
 * @brief A static function for writing the bus to low state.
 */
static inline void OwWriteBusLow(void) {

	OW_DIRECTION |= 1 << OW_BIT;
	#ifdef OW_INTERNAL_PULLUP
		OW_PORT &= ~(1 << OW_BIT);
	#endif

}


/**
 * @fn void OwInit(void)
 * @brief initializes the bus. Used to call the static function OwWriteBusHigh() in non-static way.
 */
void OwInit(void) {

	OwWriteBusHigh();
        OW_PORT &= ~(1 << OW_BIT);

}

/**
 * @fn uint8_t OwReset(void)
 * @brief A function that writes reset pulse to the bus and checks for presence pulse.
 *
 * @return 0x01 if presence pulse is detected or 0x00 if no presence pulse is detected.
 */
uint8_t OwReset(void) {

	uint8_t presence;

	#ifdef OW_BLOCK_INTERRUPTS
		cli();
	#endif

	// drive bus low
	OwWriteBusLow();

	_delay_us(OW_RESET_DELAY);

	#ifdef OW_BLOCK_INTERRUPTS_BITLEVEL
		cli();
	#endif
	// release bus
	OwWriteBusHigh();

	// wait for presence pulse
	_delay_us(OW_LONG_DELAY);

	// check for presence pulse
	presence = (OW_PIN & (1 << OW_BIT)) >> OW_BIT;

	#ifdef OW_BLOCK_INTERRUPTS_BITLEVEL
		sei();
	#endif

	_delay_us(OW_RESET_DELAY - OW_LONG_DELAY);

	#ifdef OW_BLOCK_INTERRUPTS
		sei();
	#endif

	// return 0x01 if presence pulse was read
	return presence ^ 0x01;
}

/**
 * @fn static inline uint8_t OwReadBit(void)
 * @brief Static function that reads a single bit from the bus.
 *
 * @return 8-bit value with the read bit as LSB
 */
static inline uint8_t OwReadBit(void) {

	#ifdef OW_BLOCK_INTERRUPTS_BITLEVEL
		cli();
	#endif

	// drive bus low
	OwWriteBusLow();
	_delay_us(OW_SHORT_DELAY);

	// release bus
	OwWriteBusHigh();
	_delay_us(OW_SAMPLE_DELAY);

	// read bit and return the value as LSB
	if(OW_PIN & (1 << OW_BIT)) {

	        _delay_us(OW_LONG_DELAY - OW_SAMPLE_DELAY);
	        #ifdef OW_BLOCK_INTERRUPTS_BITLEVEL
		        sei();
        	#endif
		return 0x01;

	} else {

	        _delay_us(OW_LONG_DELAY - OW_SAMPLE_DELAY);
	        #ifdef OW_BLOCK_INTERRUPTS_BITLEVEL
		        sei();
        	#endif
		return 0x00;

	}



}

/**
 * @fn uint8_t OwReadByte(void)
 * @brief Reads a byte from the bus.
 *
 * @return byte read from the bus
 */
uint8_t OwReadByte(void) {

	#ifdef OW_BLOCK_INTERRUPTS
		cli();
	#endif

	uint8_t i;

	uint8_t data = 0;

	// 8 consecutive bits are read from bus and arranged into a variable LSB first
	for(i = 0; i < 8; i++) {

		data |= OwReadBit() << i;

	}

	#ifdef OW_BLOCK_INTERRUPTS
		sei();
	#endif

	return data;

}

/**
 * @fn static inline void OwWriteBit(uint8_t data)
 * @brief Static function that writes a bit to the bus
 *
 * @param data		8-bit value which has the bit to be written as LSB
 */
static inline void OwWriteBit(uint8_t data) {

	#ifdef OW_BLOCK_INTERRUPTS_BITLEVEL
		cli();
	#endif

	// LSB of data value is checked
	if(data & 1) {

		// drive bus low
		OwWriteBusLow();
		_delay_us(OW_SHORT_DELAY);

		// release bus
		OwWriteBusHigh();
		_delay_us(OW_LONG_DELAY);

	} else {

		// drive bus low
		OwWriteBusLow();
		_delay_us(OW_LONG_DELAY);

		// release bus
		OwWriteBusHigh();
		_delay_us(OW_SHORT_DELAY);

	}

	#ifdef OW_BLOCK_INTERRUPTS_BITLEVEL
		sei();
	#endif

}

/**
 * @fn void OwWriteByte(uint8_t data)
 * @brief Writes a byte to the bus.
 *
 * @param data		byte to be written to the bus
 */
void OwWriteByte(uint8_t data) {

	#ifdef OW_BLOCK_INTERRUPTS
		cli();
	#endif

	uint8_t i;

	// data byte is written to the bus one bit at a time starting from LSB
	for(i = 0; i < 8; i++) {

		OwWriteBit(data >> i);

	}

	#ifdef OW_BLOCK_INTERRUPTS
		sei();
	#endif

}

/**
 * @fn void OwWriteByteTo(uint8_t data, uint8_t rom)
 * @brief Writes a Match Rom command, rom and byte to the 1-wire bus. Although this function itself calls OwReset before writing given byte it is still required to manually call OwReset before this function. This way every function has the same call sequence.
 *
 * @param data		byte to be written to the 1-wire bus
 * @param rom		index of the rom in static storage
 */
void OwWriteByteTo(OwContext* ctx, uint8_t rom, uint8_t data) {

	#ifdef OW_BLOCK_INTERRUPTS
		cli();
	#endif

	uint8_t i;

	// Match rom command is written to the bus
	OwWriteByte(OW_MATCH_ROM);

	// Bytes of the selected rom are written one by one to the bus
	for(i = 0; i < 8; i++) {
		OwWriteByte(ctx->roms[rom][i]);
	}

	// given data is written to the bus
	OwWriteByte(data);

	#ifdef OW_BLOCK_INTERRUPTS
		sei();
	#endif

}

/**
 * @fn uint8_t OwSearchRom(void)
 * @brief Search roms of all devices connected to the bus. Brute force method is used which
 * makes this procedure extremely slow. Interrupt blocking does not apply to this function
 * above bit level. Call sei() after searching roms if interrupts are used.
 *
 * @return		number of roms found.
 */
uint8_t OwSearchRom(OwContext* ctx) {

	uint8_t input = 0; 		// value read from bus
	uint8_t output = 0;		// LSB = bit to write to bus

	uint32_t diff = 0;		// difference stack
	uint8_t diff_index = 0;		// difference stack pointer

	uint8_t temp_byte;

	uint8_t i, j, k;

	// iterate 
	for(i = 0; i < OW_MAX_ROMS; i++) {

		diff_index = 0;	// reset difference pointer

		// search rom
		for(j = 0; j < 8; j++) {

			temp_byte = 0;

			for(k = 0; k < 8; k++) {

				if(j == 0 && k == 0) {	// New search sequence is started for every rom.
										// This is started on the first loop.
					OwReset();
					OwWriteByte(OW_SEARCH_ROM);

				} else {				// Following loops output selected bit to the bus.

					OwWriteBit(output);

				}

				// Address bit and its complement are read
				input = OwReadBit();
				input <<= 1;
				input |= OwReadBit();

				// Handling of the address bit and its complement
				switch(input) {

					// conflict
					case 0:
						
						// Move difference pointer forward and selection the value in
						// difference stack.
						diff_index++;

						if(diff & 1 << diff_index) {

							output = 1;
							temp_byte |= 1 << k;

						} else {

							output = 0;

						}

						break;

					// zero bit
					case 1:

						output = 0;				// Temp_byte already contains a zero in this index.
						break;

					// one bit
					case 2:

						output = 1;
						temp_byte |= 1 << k;	// The bit in temp_byte is set high.
						break;

					// no devices - error state
					// This state is reached only if no devices are connected or the bus dies during rom search
					case 3:

						return 0;				// Either there are no devices or bus won't function.
												// Zero is returned as device count to indicate this.
				}

			}

			// Byte is stored to static array
			ctx->roms[i][j] = temp_byte;

		}

		// Full rom is found at this point.
		if(diff & (1 << diff_index)) {			// If current index in difference stack contains a one
												// it is set to zero along with all adjacent ones.
			j = 0;
			
			while(diff & (1 << (diff_index - j))) {

				diff &= ~(1 << (diff_index - j));

				j++;

			}

			diff |= 1 << (diff_index - j);		// The first zero found is set to one.

		} else {								// If current index in difference stack contains a zero
												// it is set to one.
			diff |= 1 << diff_index;

		}

		if(diff & 1) {							// First bit set as one indicates finished search.

			break;

		}

	}

        // If number of connected devices is equal to or greater than OW_MAX_ROMS
        // i will be OW_MAX_ROMS + 1 at the end of this loop.
        if(i == OW_MAX_ROMS) return OW_MAX_ROMS;

	return i + 1;								// total amount of roms found equals found differences + 1.

}
