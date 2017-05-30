/*
 * zpdc_system.h
 *
 * Created: 5/30/2017 10:40:47 AM
 *  Author: Andres Vasquez
 */ 


#ifndef ZPDC_SYSTEM_H_
#define ZPDC_SYSTEM_H_
#ifdef __cplusplus	

/* System Data */
class ZpdcSystem {
public:
	const uint32_t* const uniqueness_A = reinterpret_cast<uint32_t *>(0x0080A00C);
	const uint32_t* const uniqueness_B = reinterpret_cast<uint32_t *>(0x0080A040);
	const uint32_t* const uniqueness_C = reinterpret_cast<uint32_t *>(0x0080A044);
	const uint32_t* const uniqueness_D = reinterpret_cast<uint32_t *>(0x0080A048);
	
	ZpdcSystem() {
		/* ---- Initialize the UID ---- */
		uid = 0;
		enum status_code result_code = eeprom_emulator_init();

		if ( result_code == STATUS_ERR_NO_MEMORY ) while (true) {  }
		else if ( result_code == STATUS_ERR_BAD_FORMAT ) {
			while ( result_code == STATUS_ERR_BAD_FORMAT ) {
				eeprom_emulator_erase_memory();
				result_code = eeprom_emulator_init();
				if ( result_code != STATUS_ERR_BAD_FORMAT ) uid_setup();
			}
		} else if ( result_code == STATUS_OK ) while ((uid = read_uid()) == 0xFFFF ) uid_setup();
		else while (true) {  }	// LOCK if EEPROM error
	}

	uint16_t get_uid(void) { return uid; }
private:
	uint16_t uid;

		// Remap a pseudo-unique ID by hashing a 128-bit value to 16-bit
	void uid_setup(void) {
		volatile uint32_t id_int = 0;
		uid = 0;
		id_int += (*uniqueness_A) * 3;
		id_int += (*uniqueness_B) * 3;
		//uid = (uint16_t)((id_int % 0x0000FFFF));	// END UID HASH
		//id_int = 0;
		id_int += (*uniqueness_C) * 3;
		id_int += (*uniqueness_D) * 3;
		uid |= (uint16_t)((id_int % 0x0000FFFF));	// END UID HASH

		uint8_t page_data[EEPROM_PAGE_SIZE];
		eeprom_emulator_read_page(0, page_data);
		page_data[0] = (uint8_t)(uid & 0x00FF);
		page_data[1] = (uint8_t)(uid >> 8);
		eeprom_emulator_write_page(0,page_data);
		eeprom_emulator_commit_page_buffer();	// END UID EEPROM STORAGE
	}
	uint16_t read_uid(void) {
		uint8_t page_data[EEPROM_PAGE_SIZE];
		eeprom_emulator_read_page(0, page_data);
		return ((uint16_t)page_data[1] << 8) | (uint16_t)page_data[0];
	}
};

#endif // __cplusplus
#endif /* ZPDC_SYSTEM_H_ */