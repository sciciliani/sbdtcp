cmd_/root/sbdtcp/sbd.ko := ld -r -m elf_i386 -T /usr/src/linux-headers-3.5.0-17-generic/scripts/module-common.lds --build-id  -o /root/sbdtcp/sbd.ko /root/sbdtcp/sbd.o /root/sbdtcp/sbd.mod.o
