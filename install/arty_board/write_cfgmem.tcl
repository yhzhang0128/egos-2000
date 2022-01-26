write_cfgmem -format mcs -interface spix4 -size 16 -loadbit {up 0x0 arty_board/fe310_arty.bit} -loaddata {up 0x400000 earth.bin up 0x800000 disk.img} -file egos_bootROM.mcs -force
