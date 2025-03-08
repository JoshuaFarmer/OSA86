ENTRY(init)

SECTIONS {
	. = 0x10000;
	.text ALIGN(16) : {
		*(.text)
	}

	.rodata ALIGN(16) : {
		*(.rodata)
	}

	.data ALIGN(16) : {
		*(.data)
	}

	.bss ALIGN(16) : {
		*(.bss)
	}

	/DISCARD/ : {
		*(.eh_frame)
	}
}