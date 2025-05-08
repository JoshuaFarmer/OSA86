ENTRY(init)

SECTIONS {
	. = 0x10000;
	.text ALIGN(32) : {
		*(.text)
	}

	.rodata ALIGN(32) : {
		*(.rodata)
	}

	.data ALIGN(32) : {
		*(.data)
	}

	.bss ALIGN(32) : {
		*(.bss)
	}

	/DISCARD/ : {
		*(.eh_frame)
	}
}
