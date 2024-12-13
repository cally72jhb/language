#ifndef WAVE_LANGUAGE_MAIN
#define WAVE_LANGUAGE_MAIN

// Includes

#include "common/constants.h"
#include "common/error_codes.h"

// Program Main Functions

error_code program_startup(void);
error_code program_shutdown_request(void);
error_code program_cleanup(void);
error_code program_close(void);

error_code program_main(void);

#endif
