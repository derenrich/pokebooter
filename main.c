void write_string(int color, const char *string)
{
    volatile char *video = (volatile char*)0xB8000;
    while(*string != 0 )
    {
        *video =  *string;
        string++;
        video++;
        *video = color;
        video++;
    }
}

void main() {
    write_string(0xf0, "Hello, World! Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!Hello, World!");//, "Hello, World!");

    __asm__("cli");
    __asm__("hlt");

}