#pragma once

#include <time.h>

/** Κλήθηκε από το loop thread με τοπική ώρα (TZ) μετά από επιτυχή SNTP. */
using romeos_ntp_synced_fn = void (*)(const struct tm *tm);

void romeos_ntp_init(romeos_ntp_synced_fn on_synced_wall);
void romeos_ntp_poll(void);
