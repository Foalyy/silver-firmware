#include <stdint.h>

// Sections provided by the linker script. See common.ld.
// Note that SECTION_TEXT_START is not a pointer to the start of the .text section,
// it *is* the start of the section (the first byte). The address is &SECTION_TEXT_START.
// The same applies for the other symbols.
extern uint32_t SECTION_TEXT_START;
extern uint32_t SECTION_TEXT_END;
extern uint32_t SECTION_DATA_START;
extern uint32_t SECTION_RELOCATE_START;
extern uint32_t SECTION_RELOCATE_END;
extern uint32_t SECTION_BSS_START;
extern uint32_t SECTION_BSS_END;
extern uint32_t SECTION_STACK_START;
extern uint32_t SECTION_STACK_END;

// main() is *declared* here but should be *defined* by the user
int main();

// Initialization function for the libc standard library.
// It takes cares of tasks such as calling constructors for global variables (objects).
extern "C" void __libc_init_array(void);

// Reset handler, called when the microcontroller is started
void ResetHandler(void) {
    // Copy data from flash to RAM. This includes things such as raw character strings and global
    // variables initialized to a non-zero value.
    const int SECTION_RELOCATE_LENGTH = &SECTION_RELOCATE_END - &SECTION_RELOCATE_START; // number of 4-bytes words
    for (int i = 0; i < SECTION_RELOCATE_LENGTH; i++) {
        *(&SECTION_RELOCATE_START + i) = *(&SECTION_DATA_START + i);
    }

    // The BSS is the segment containing all global variables initialized to 0.
    // It is not stored in the program in flash to save space, and must therefore be 
    // initialized at runtime.
    const int SECTION_BSS_LENGTH = &SECTION_BSS_END - &SECTION_BSS_START; // number of 4-bytes words
    for (int i = 0; i < SECTION_BSS_LENGTH; i++) {
        *(&SECTION_BSS_START + i) = 0;
    }

    // Initialize the standard library
    __libc_init_array();

    // Call the main function defined by the user
    main();

    // The main shouldn't return, but if it does, loop indefinitely here
    // to prevent the CPU from executing data beyond the end of the handler
    while (true);
}

// Default dummy handlers for the other exceptions. The library will override them in Core::init().

void NMIHandler() {
    while (true);
}

void HardFaultHandler() {
    while (true);
}

void MemManageHandler() {
    while (true);
}

void BusFaultHandler() {
    while (true);
}

void UsageFaultHandler() {
    while (true);
}

void SVCHandler() {
    while (true);
}

void DebugMonHandler() {
    while (true);
}

void PendSVHandler() {
    while (true);
}

void SysTickHandler() {
    while (true);
}

// Vector of pointers to exception handlers, loaded by the linker script at the beginning of the flash
// See ARMv7-M Architecture Reference Manual, B1.5.2 "Exception number definition", page 581
__attribute__ ((section(".vectors")))
uint32_t exceptionVector[] = {
    (uint32_t)&SECTION_STACK_END,
    (uint32_t)ResetHandler,
    (uint32_t)NMIHandler,
    (uint32_t)HardFaultHandler,
    (uint32_t)MemManageHandler,
    (uint32_t)BusFaultHandler,
    (uint32_t)UsageFaultHandler,
    (uint32_t)nullptr,
    (uint32_t)nullptr,
    (uint32_t)nullptr,
    (uint32_t)nullptr,
    (uint32_t)SVCHandler,
    (uint32_t)DebugMonHandler,
    (uint32_t)nullptr,
    (uint32_t)PendSVHandler,
    (uint32_t)SysTickHandler,
};