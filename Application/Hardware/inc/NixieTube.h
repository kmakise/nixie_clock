#ifndef __NIXIETUBE_H
#define __NIXIETUBE_H

void NixieTubeInit(void);  
void NixieTubeDrive(uint8_t * num);
void NixieTubeDisplay(uint8_t * num);
void NixieTubeTest(void);
void NixieTubeControl(uint8_t cmd);
void NixieTubeFlash(void);  

#endif /*__NIXIETUBE_H*/
