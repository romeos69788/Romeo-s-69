/**
 * Wrapper: το PlatformIO μερικές φορές μεταγλωττίζει αρχεία από ../../shared/
 * χωρίς σωστά Arduino includes → Arduino.h not found. Ένα TU από src/ λύνει το θέμα.
 */
#include "../../shared/src/romeos_wifi_nvs.cpp"
