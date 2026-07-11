#pragma once

/**
 * NVS για SSID/κωδικό home Wi‑Fi (μητρική + οθόνη).
 * Namespace Preferences: "romeos_wifi", κλειδιά "home_ssid", "home_pass".
 *
 * Πρόγραμμα από Serial (115200, newline LF):
 *   WIFI_HOME_SET
 *   <γραμμή SSID>
 *   <γραμμή κωδικού>
 *   → αποθηκεύει και εκτυπώνει [wifi_nvs] saved.
 *
 *   WIFI_HOME_CLR  → διαγράφει τα αποθηκευμένα home credentials.
 *   WIFI_HOME_SHOW → εκτυπώνει αν υπάρχουν αποθηκευμένα (χωρίς να δείχνει κωδικό).
 */

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Άπαξ στην εκκίνηση: δημιουργεί κενό namespace αν λείπει ώστε read-only open να μην ρίχνει ESP_LOGE NOT_FOUND. */
void romeos_wifi_nvs_init(void);

bool romeos_wifi_nvs_has_home(void);
/** Φόρτωση σε buffers· null-τερματισμός. Επιστρέφει true αν το SSID δεν είναι κενό. */
bool romeos_wifi_nvs_load_home(char *ssid, size_t ssid_cap, char *pass, size_t pass_cap);
bool romeos_wifi_nvs_save_home(const char *ssid, const char *pass);
bool romeos_wifi_nvs_clear_home(void);

/** Αν το NVS είναι κενό και υπάρχουν build flags ROMEOS_WIFI_HOME_*_DEFAULT, αποθηκεύει και επιστρέφει true. */
bool romeos_wifi_nvs_apply_build_defaults(void);

/**
 * Κάλεσε από loop() — μη blocking.
 * @return true αν μόλις αποθηκεύτηκαν ή διαγράφηκαν home credentials (ώστε να ξαναφορτώσει το link/Wi‑Fi).
 */
bool romeos_wifi_nvs_poll_serial(void);

#ifdef __cplusplus
}
#endif
