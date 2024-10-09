/* Based on GCC ARM embedded samples.
   Defines the following symbols for use by code:
    __exidx_start
    __exidx_end
    __etext
    __data_start__
    __preinit_array_start
    __preinit_array_end
    __init_array_start
    __init_array_end
    __fini_array_start
    __fini_array_end
    __data_end__
    __bss_start__
    __bss_end__
    __end__
    end
    __HeapLimit
    __StackLimit
    __StackTop
    __stack (== StackTop)
*/

MEMORY
{
    RAM(rwx) : ORIGIN =  0x20000000, LENGTH = 256K /* main RAM 256KB */
    SCRATCH_X(rwx) : ORIGIN = 0x20040000, LENGTH = 4k /* core 0 stack 4KB */
    BOOTLOADERRAM(rw) : ORIGIN = 0x20041000, LENGTH = 32 /* boot loader resident RAM 32B */
    SCRATCH_Y(rwx) : ORIGIN = 0x20041000+32, LENGTH = 4k-32 /* core 1 stack 4KB-32B */
}

ENTRY(_reset_handler)

SECTIONS
{
    /* Note in NO_FLASH builds the entry point for both the bootrom, and debugger
       entry (ELF entry point), are *first* in the image, and the vector table
       follows immediately afterward. This is because the bootrom enters RAM
       binaries directly at their lowest address (preferring main RAM over XIP
       cache-as-SRAM if both are used).
    */

    .text (READONLY) : {
        __logical_binary_start = .;
        __reset_start = .;
        KEEP (*(.reset))
        __reset_end = .;
        KEEP (*(.binary_info_header))
        __binary_info_header_end = .;
        KEEP (*(.embedded_block))
        __embedded_block_end = .;
        . = ALIGN(256);
        KEEP (*(.vectors))
        KEEP (*(.picopad_info_header))
        KEEP (*(.vector_handlers))
        *(.time_critical*)
        *(.text*)
        . = ALIGN(4);
        *(.init)
        *(.fini)
        /* Pull all c'tors into .text */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)
        /* Followed by destructors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.eh_frame*)
    } > RAM

    .rodata (READONLY) : {
        *(.rodata*)
        . = ALIGN(4);
        *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.flashdata*)))
        . = ALIGN(4);
    } > RAM

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > RAM

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > RAM
    __exidx_end = .;

    /* Machine inspectable binary information */
    . = ALIGN(4);
    __binary_info_start = .;
    .binary_info :
    {
        KEEP(*(.binary_info.keep.*))
        *(.binary_info.*)
    } > RAM
    __binary_info_end = .;
    . = ALIGN(4);

    .data : {
        /* End of .text-like segments */
        __etext = .;
        __data_start__ = .;
        *(vtable)
        *(.data*)

        . = ALIGN(4);
        *(.after_data.*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__mutex_array_start = .);
        KEEP(*(SORT(.mutex_array.*)))
        KEEP(*(.mutex_array))
        PROVIDE_HIDDEN (__mutex_array_end = .);

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(SORT(.preinit_array.*)))
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        *(SORT(.fini_array.*))
        *(.fini_array)
        PROVIDE_HIDDEN (__fini_array_end = .);

        *(.jcr)
        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;
    } > RAM

    .uninitialized_data (NOLOAD): {
        . = ALIGN(4);
        *(.uninitialized_data*)
    } > RAM

    .bss (NOLOAD)  : {
        . = ALIGN(4);
        __bss_start__ = .;
        *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.bss*)))
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    .heap (NOLOAD):
    {
        __end__ = .;
        end = __end__;
        KEEP(*(.heap*))
        __HeapLimit = .;

	/* start of free RAM for memory allocator */
        . = ALIGN(16);
	__malloc_start__ = .;

    } > RAM

/* SCRATCH_X_STACK0 must be allocated on end of RAM to ensure allocation of RAM for malloc */

    /* .stack*_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */

    .stack0_dummy (COPY): /* core 0 stack (4 KB) */
    {
	/* end of free RAM for memory allocator */
        . = ALIGN(16);
	__malloc_end__ = .;

        KEEP (*(.stack0*))
    } > SCRATCH_X

    .bootloaderdata (COPY):
    {
        *(.bootloaderdata)
    } > BOOTLOADERRAM

    .stack1_dummy (COPY): /* core 1 stack (4 KB) */
    {
        KEEP (*(.stack1*))
    } > SCRATCH_Y

    /* stack limit is poorly named, but historically is maximum heap ptr */
    __StackLimit = ORIGIN(RAM) + LENGTH(RAM);
    __Stack0Top = ORIGIN(SCRATCH_X) + LENGTH(SCRATCH_X);
    __Stack1Top = ORIGIN(SCRATCH_Y) + LENGTH(SCRATCH_Y);
    __Stack0Bottom = __Stack0Top - SIZEOF(.stack0_dummy);
    __Stack1Bottom = __Stack1Top - SIZEOF(.stack1_dummy);
    PROVIDE(__stack = __Stack0Top);
    PROVIDE(__StackTop = __Stack0Top);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed")

    ASSERT(picopad_info_header - __logical_binary_start == 0x1c0, "Incorrect offset of Picopad Info Header!")

    ASSERT( __binary_info_header_end - __logical_binary_start <= 256, "Binary info must be in first 256 bytes of the binary")
    ASSERT( __embedded_block_end - __logical_binary_start <= 4096, "Embedded block must be in first 4096 bytes of the binary")
}
