#ifndef __ctrlayer_h__
#define __ctrlayer_h__

#ifdef __cplusplus
extern "C" {
#endif

//extern bool gQuitGame;
void backlightEnable(bool enable, unsigned long screen);
void ctr_clear_console();
int ctr_swkbd(const char *hintText, const char *inText, char *outText);

#ifdef __cplusplus
}
#endif
extern bool gQuitGame;
#endif // __ctrlayer_h__

