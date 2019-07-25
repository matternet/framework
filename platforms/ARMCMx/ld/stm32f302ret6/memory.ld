MEMORY
{
    bl (rx) :             ORIGIN = 0x08000000,            LENGTH = 12K
    app (rx) :            ORIGIN = 0x08000000+12K,        LENGTH = 256K-12K-4K
    param1 (rx) :         ORIGIN = 0x08000000+252K,       LENGTH = 2K
    param2 (rx) :         ORIGIN = 0x08000000+254K,       LENGTH = 2K
    ram (rwx) :           ORIGIN = 0x20000000,            LENGTH = 64K-256
    app_bl_shared (rwx) : ORIGIN = 0x20000000+(64K-256),  LENGTH = 256
}
