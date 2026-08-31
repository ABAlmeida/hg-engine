/* Overlay 130 */
OUTPUT_ARCH(arm)
MEMORY {

        /* Physical capacity is 0x14000. Keep the final 0x1F4 bytes free. */
        rom     : ORIGIN = 0x023C4000, LENGTH = 0x13E0C
        ewram   : ORIGIN = 0x023C4000, LENGTH = 0x13E0C
}

SECTIONS {
        .text : {

                FILL (0xABCD)

                __text_start = . ;
                *(.init)
                *(.text)
                *(.ctors)
                *(.dtors)
                *(.rodata)
                *(.fini)
                *(COMMON)
                __text_end  = . ;

                __bss_start__ = . ;
                *(.bss)
                __bss_end__ = . ;
        _end = __bss_end__ ;
        __end__ = __bss_end__ ;
        } >rom = 0xff
}
