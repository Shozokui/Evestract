#ifndef TEXT_H
#define TEXT_H

uint32_t GetEventMessageSize(const uint8_t* rawMessage, uint32_t rawMessageSize, uint8_t mask);

const char* GetPrintableText(const uint8_t* rawMessage, uint32_t rawMessageSize);
int FreePrintableText(const char* text);

#endif
