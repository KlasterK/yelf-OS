ENTRY(a_start)

SECTIONS
{
	. = 1M;

	load_begin = .;

    .text : ALIGN(4K)
    {
        KEEP( *(.m) )
        *(.text)
    }

    .rodata : ALIGN(4K)
	{
		*(.rodata)
	}

	.data : ALIGN(4K)
	{
		*(.data)
	}

	load_end = .;

	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	bss_end = .;
}
