#ifndef TEXT_H
#define TEXT_H

const char* GetPrintableText(const uint8_t* rawMessage, uint32_t rawMessageSize);
int FreePrintableText(const char* text);

#endif
