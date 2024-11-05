/*===================================================================*/
/*                                                                   */
/*  InfoNES_System.h : The function which depends on a system        */
/*                                                                   */
/*  2000/05/29  InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_SYSTEM_H_INCLUDED
#define InfoNES_SYSTEM_H_INCLUDED

/* Get a joypad state */
void FASTCODE NOFLASH(InfoNES_PadState)(DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem);

/* memcpy */
inline void *InfoNES_MemoryCopy(void *dest, const void *src, int count)
{
    __builtin_memcpy(dest, src, count);
    return dest;
}

/* memset */
inline void *InfoNES_MemorySet(void *dest, int c, int count)
{
    __builtin_memset(dest, c, count);
    return dest;
}

/* Sound Initialize */
void InfoNES_SoundInit(void);

#endif /* !InfoNES_SYSTEM_H_INCLUDED */
