#pragma once

/** Μία φορά μετά `ui_init`: δημιουργεί εικονίδια Wi‑Fi (μεγαλύτερα από το κείμενο «Συνδεδεμένο»). */
void romeos_v4_wifi_ui_init(void);

/** Κλήση από `loop()`: ενημερώνει χρώματα/κατάσταση σπιτιού και μητρικής. */
void romeos_v4_wifi_status_poll(void);
