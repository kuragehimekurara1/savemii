#include <language.h>
#include <savemng.h>

#include <jansson.h>

#include <coreinit/memdefaultheap.h>
#include <coreinit/memory.h>

struct MSG;
typedef struct MSG MSG;
struct MSG {
    uint32_t id;
    const char *msgstr;
    MSG *next;
};

static MSG *baseMSG = NULL;

#define HASHMULTIPLIER 31 // or 37

Swkbd_LanguageType sysLang;

void loadLanguage(Swkbd_LanguageType language) {
    switch (language) {
        /*case Swkbd_LanguageType__Japanese:
			gettextLoadLanguage("romfs:/japanese.json");
            break;
		case Swkbd_LanguageType__English:
			gettextLoadLanguage("romfs:/english.json");
            break;*/
        /*case Swkbd_LanguageType__French:
			gettextLoadLanguage("romfs:/french.json");
            break;
		case Swkbd_LanguageType__German:
			gettextLoadLanguage("romfs:/german.json");
            break;*/
        case Swkbd_LanguageType__Italian:
            gettextLoadLanguage("romfs:/italian.json");
            break;
        case Swkbd_LanguageType__Spanish:
            gettextLoadLanguage("romfs:/spanish.json");
            break;
        /*case Swkbd_LanguageType__Chinese1:
			gettextLoadLanguage("romfs:/chinese1.json");
            break;
        */
        case Swkbd_LanguageType__Korean:
            gettextLoadLanguage("romfs:/korean.json");
            break;
        /*
		case Swkbd_LanguageType__Dutch:
			gettextLoadLanguage("romfs:/dutch.json");
            break;
		case Swkbd_LanguageType__Potuguese:
			gettextLoadLanguage("romfs:/portuguese.json");
            break;*/
        case Swkbd_LanguageType__Russian:
            gettextLoadLanguage("romfs:/russian.json");
            break; /*
		case Swkbd_LanguageType__Chinese2:
			gettextLoadLanguage("romfs:/chinese2.json");
            break;*/
        default:
            //gettextLoadLanguage("romfs:/english.json");
            break;
    }
}

Swkbd_LanguageType getSystemLanguage() {
    UCHandle handle = UCOpen();
    if (handle < 1)
        sysLang = Swkbd_LanguageType__English;

    UCSysConfig *settings = (UCSysConfig *) MEMAllocFromDefaultHeapEx(sizeof(UCSysConfig), 0x40);
    if (settings == nullptr) {
        UCClose(handle);
        sysLang = Swkbd_LanguageType__English;
    }

    strcpy(settings->name, "cafe.language");
    settings->access = 0;
    settings->dataType = UC_DATATYPE_UNSIGNED_INT;
    settings->error = UC_ERROR_OK;
    settings->dataSize = sizeof(Swkbd_LanguageType);
    settings->data = &sysLang;

    UCError err = UCReadSysConfig(handle, 1, settings);
    UCClose(handle);
    MEMFreeToDefaultHeap(settings);
    if (err != UC_ERROR_OK)
        sysLang = Swkbd_LanguageType__English;
    return sysLang;
}

// Hashing function from https://stackoverflow.com/a/2351171
static inline uint32_t hash_string(const char *str_param) {
    uint32_t hash = 0;

    while (*str_param != '\0')
        hash = HASHMULTIPLIER * hash + *str_param++;

    return hash;
}

static inline MSG *findMSG(uint32_t id) {
    for (MSG *msg = baseMSG; msg; msg = msg->next)
        if (msg->id == id)
            return msg;

    return NULL;
}

static void setMSG(const char *msgid, const char *msgstr) {
    if (!msgstr)
        return;

    uint32_t id = hash_string(msgid);
    MSG *msg = (MSG *) MEMAllocFromDefaultHeap(sizeof(MSG));
    msg->id = id;
    msg->msgstr = strdup(msgstr);
    msg->next = baseMSG;
    baseMSG = msg;
    return;
}

void gettextCleanUp() {
    while (baseMSG) {
        MSG *nextMsg = baseMSG->next;
        MEMFreeToDefaultHeap((void *) (baseMSG->msgstr));
        MEMFreeToDefaultHeap(baseMSG);
        baseMSG = nextMsg;
    }
}

bool gettextLoadLanguage(const char *langFile) {
    uint8_t *buffer;
    int32_t size = loadFile(langFile, &buffer);
    if (buffer == nullptr)
        return false;

    bool ret = true;
    json_t *json = json_loadb((const char *) buffer, size, 0, nullptr);
    if (json) {
        size = json_object_size(json);
        if (size != 0) {
            const char *key;
            json_t *value;
            json_object_foreach(json, key, value) if (json_is_string(value))
                    setMSG(key, json_string_value(value));
        } else {
            ret = false;
        }

        json_decref(json);
    } else {
        ret = false;
    }

    MEMFreeToDefaultHeap(buffer);
    return ret;
}

const char *gettext(const char *msgid) {
    MSG *msg = findMSG(hash_string(msgid));
    return msg ? msg->msgstr : msgid;
}