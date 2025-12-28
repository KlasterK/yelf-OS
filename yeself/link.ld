ENTRY(a_start)

SECTIONS
{
	. = 1M;

	load_begin = .;

    .text BLOCK(4K) : ALIGN(4K)
    {
        KEEP( *(.m) )
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	load_end = .;

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	bss_end = .;
}
