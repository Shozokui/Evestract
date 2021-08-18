#ifndef DIALOG_H
#define DIALOG_H

struct dialog_entry_t {
    uint32_t id;
    uint32_t length;
    uint8_t* text;
};

struct dialog_t {
    struct dialog_entry_t* entries;
    uint32_t numEntries;
};

int LoadDialog(struct dialog_t** dialog, const uint8_t* buf, uint32_t length);
int UnloadDialog(struct dialog_t* dialog);

const char* GetPrintableDialogText(const struct dialog_t* dialog, uint32_t index);

#endif
