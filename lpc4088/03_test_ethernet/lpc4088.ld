/* LPC4088 linker script */

MEMORY {
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 512K
    SRAM  (rwx) : ORIGIN = 0x10000000, LENGTH =  64K
}

__stack_top = 0x10000000 + 64K;

SECTIONS {
    .text : {
        KEEP(*(.vectors))
        KEEP(*(.precrp))
        . = 0x000002FC ;
        KEEP(*(.crp))
        *(.text)
        *(.text.*)
        *(.rodata)
        *(.rodata.*)
        . = ALIGN(4);
    } >FLASH

    /* initialized data */
    .data : {
        __data_load = LOADADDR (.data);
        __data_begin = .;
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        __data_end = .;
    } >SRAM AT >FLASH

    /* uninitialized data */
    .bss : {
        __bss_begin = .;
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } >SRAM
}
