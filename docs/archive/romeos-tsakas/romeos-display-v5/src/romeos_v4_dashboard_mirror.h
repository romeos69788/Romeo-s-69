#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** Μετά `ui_init`: ρυθμίσεις κουμπιών πλοήγησης + έτοιμο για `poll`. */
void romeos_v4_dashboard_mirror_init(void);

/** Αντιγραφή ρολογιού / θερμοκρασίας / υγρασίας από Screen1 στις 2–6 (~4 Hz). */
void romeos_v4_dashboard_mirror_poll(void);

#ifdef __cplusplus
}
#endif
