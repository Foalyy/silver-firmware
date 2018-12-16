#ifndef _TRIGGER_H_
#define _TRIGGER_H_



void warningHandler(Error::Module module=Error::Module::CUSTOM, int userModule=0, Error::Code code=0);
void criticalHandler(Error::Module module=Error::Module::CUSTOM, int userModule=0, Error::Code code=0);

#endif