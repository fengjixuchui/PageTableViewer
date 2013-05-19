#ifndef __CUI_MAIN_H__
#define __CUI_MAIN_H__

void SwitchExecutionMode(int argc, char** argv);

BOOL CheckOption(int argc, char** argv, char* option);
BOOL GetOptionString(int argc, char** argv, char* option, char* buf, int size);

#endif /* __CUI_MAIN_H__ */
