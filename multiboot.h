
struct multiboot_header
{
        unsigned long magic;
        unsigned long flags;
        unsigned long checksum;
        unsigned long header_addr;
        unsigned long load_addr;
        unsigned long load_end_addr;
        unsigned long bss_end_addr;
        unsigned long entry_addr;
};

struct multiboot_entry
{

};

struct elf_section_header_table
{
        unsigned long num;
        unsigned long size;
        unsigned long addr;
        unsigned long shndx;
};

struct multiboot_info
{
        unsigned long flags;
        unsigned long mem_lower;
        unsigned long mem_upper;
        unsigned long boot_device;
        unsigned long cmdline;
        unsigned long mods_count;
        unsigned long mods_addr;
        struct elf_section_header_table elf_sec_hdr;
        unsigned long mmap_length;
        unsigned long mmap_addr;
};

