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
    FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 2M - 4K /* flash ROM memory 2MB-4KB */
    CONFIG(r) : ORIGIN = 0x101FF000, LENGTH = 4K /* device configuration 4KB */
    RAM(rwx) : ORIGIN =  0x20000000, LENGTH = 256K /* main RAM 256KB */
    SCRATCH_X(rwx) : ORIGIN = 0x20040000, LENGTH = 4k /* core 0 stack 4KB */
    BOOTLOADERRAM(rw) : ORIGIN = 0x20041000, LENGTH = 32 /* boot loader resident RAM 32B */
    SCRATCH_Y(rwx) : ORIGIN = 0x20041000+32, LENGTH = 4k-32 /* core 1 stack 4KB-32B */
}

ENTRY(_reset_handler)

SECTIONS
{
    /* Second stage bootloader is prepended to the image. It must be 256 bytes big
       and checksummed. It is usually built by the boot_stage2 target
       in the Raspberry Pi Pico SDK
    */

    .flash_begin : {
        __flash_binary_start = .;
    } > FLASH

    .boot2 : {
        __boot2_start__ = .;
        KEEP (*(.boot2))
        __boot2_end__ = .;
    } > FLASH

    ASSERT(__boot2_end__ - __boot2_start__ == 0x8000,
        "ERROR: PicoPad bootloader must be 32768 bytes in size")

    /* The second stage will always enter the image at the start of .text.
       The debugger will use the ELF entry point, which is the _entry_point
       symbol if present, otherwise defaults to start of .text.
       This can be used to transfer control back to the bootrom on debugger
       launches only, to perform proper flash setup.
    */

    .boot3 : {
        __logical_binary_start = .;
        KEEP (*(.vectors))
        KEEP (*(.picopad_info_header))
        KEEP (*(.binary_info_header))
        __binary_info_header_end = .;
        KEEP (*(.embedded_block))
        __embedded_block_end = .;
        KEEP (*(.vector_handlers))
        KEEP (*(.reset))
   } > FLASH

   .text : {
        *(.init)
        *(EXCLUDE_FILE(*libgcc.a: *libc.a:*lib_a-mem*.o *libm.a:) .text*)
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

        *(.eh_frame*)
        . = ALIGN(4);
    } > FLASH

    .rodata : {
/* We excluded nameless .rodata symbols from the .rodata section because
 they are jump tables for switch() so that they are placed in RAM. Other
 common r/o data remains because they are named .rodata.xxxx. */
        *(EXCLUDE_FILE(*libgcc.a: *libc.a:*lib_a-mem*.o *libm.a:) .rodata.*)
        . = ALIGN(4);
        *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.flashdata*)))
        . = ALIGN(4);
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    /* Machine inspectable binary information */
    . = ALIGN(4);
    __binary_info_start = .;
    .binary_info :
    {
        KEEP(*(.binary_info.keep.*))
        *(.binary_info.*)
    } > FLASH
    __binary_info_end = .;
    . = ALIGN(4);

    /* End of .text-like segments */
    __etext = .;

    .data (READONLY) : {
        __data_start__ = .;
        *(vtable)

        *(.time_critical*)

        /* remaining .text and .rodata; i.e. stuff we exclude above because we want it in RAM */
        *(.text*)
        . = ALIGN(4);
        *(.rodata*)
        . = ALIGN(4);

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
        *(.jcr)
        . = ALIGN(4);

        /* All data end */
        __data_end__ = .;
    } > RAM AT> FLASH

    .ram_vector_table (NOLOAD): {
        . = ALIGN(256);
        *(.ram_vector_table)
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

    . = ALIGN(4);
    .flash_end : {
        __flash_binary_end = .;
    } > FLASH

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

    ASSERT(picopad_info_header - __logical_binary_start == 0xc0, "Incorrect offset of Picopad Info Header!")

    ASSERT( __binary_info_header_end - __logical_binary_start <= 256, "Binary info must be in first 256 bytes of the binary")
    ASSERT( __embedded_block_end - __logical_binary_start <= 4096, "Embedded block must be in first 4096 bytes of the binary")
}
