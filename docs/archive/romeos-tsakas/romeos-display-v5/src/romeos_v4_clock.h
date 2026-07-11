#pragma once

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Μία φορά μετά το `ui_init`: εμφανίζει AM/PM (χρυσό) δίπλα σε 24ωρη ώρα/λεπτά. */
void romeos_v4_clock_prepare_ui(void);

/** Καταχώρηση SNTP· μετά το sync ενημερώνει αμέσως ώρα (24ωρη) + AM/PM. */
void romeos_v4_clock_on_ntp_sync(const struct tm *tm);

/** Κλήση από `loop()` μετά το `romeos_display_link_poll`· ενημερώνει ώρα από `time()` κάθε ~1 s. */
void romeos_v4_clock_poll(void);

#ifdef __cplusplus
}
#endif
