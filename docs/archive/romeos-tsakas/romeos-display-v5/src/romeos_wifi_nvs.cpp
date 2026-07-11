#include "romeos_wifi_nvs.h"
#include "romeos_wifi_home_defaults.h"

#include <Arduino.h>
#include <Preferences.h>
#include <cstring>

namespace {

constexpr char k_ns[] = "romeos_wifi";
constexpr char k_ssid[] = "home_ssid";
constexpr char k_pass[] = "home_pass";

constexpr size_t k_max_ssid = 32;
constexpr size_t k_max_pass = 64;

enum class Step : uint8_t { Idle, WantSsid, WantPass };
Step s_step = Step::Idle;
char s_line_buf[128];
size_t s_line_len = 0;
uint32_t s_step_deadline_ms = 0;
char s_pending_ssid[40];

constexpr uint32_t k_provision_timeout_ms = 120000;

void buf_reset()
{
    s_line_len = 0;
    s_line_buf[0] = '\0';
}

bool append_serial_to_line()
{
    while (Serial.available() > 0) {
        const int c = Serial.read();
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            if (s_line_len < sizeof(s_line_buf) - 1) {
                s_line_buf[s_line_len] = '\0';
            } else {
                s_line_buf[sizeof(s_line_buf) - 1] = '\0';
            }
            return true;
        }
        if (s_line_len + 1 < sizeof(s_line_buf)) {
            s_line_buf[s_line_len++] = static_cast<char>(c);
        }
    }
    return false;
}

void trim_inplace(char *s)
{
    if (!s) {
        return;
    }
    size_t n = std::strlen(s);
    while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t')) {
        s[--n] = '\0';
    }
    size_t i = 0;
    while (s[i] == ' ' || s[i] == '\t') {
        ++i;
    }
    if (i > 0) {
        std::memmove(s, s + i, n - i + 1);
    }
}

}  // namespace

extern "C" void romeos_wifi_nvs_init(void)
{
    Preferences p;
    if (!p.begin(k_ns, false)) {
        return;
    }
    p.end();
}

bool romeos_wifi_nvs_has_home(void)
{
    Preferences p;
    if (!p.begin(k_ns, true)) {
        return false;
    }
    if (!p.isKey(k_ssid)) {
        p.end();
        return false;
    }
    const String s = p.getString(k_ssid, "");
    p.end();
    return s.length() > 0;
}

bool romeos_wifi_nvs_load_home(char *ssid, size_t ssid_cap, char *pass, size_t pass_cap)
{
    if (!ssid || ssid_cap == 0 || !pass || pass_cap == 0) {
        return false;
    }
    ssid[0] = '\0';
    pass[0] = '\0';
    Preferences p;
    if (!p.begin(k_ns, true)) {
        return false;
    }
    if (!p.isKey(k_ssid)) {
        p.end();
        return false;
    }
    const String s = p.getString(k_ssid, "");
    const String w = p.isKey(k_pass) ? p.getString(k_pass, "") : String();
    p.end();
    if (s.length() == 0) {
        return false;
    }
    s.getBytes(reinterpret_cast<uint8_t *>(ssid), ssid_cap);
    if (ssid_cap > 0) {
        ssid[ssid_cap - 1] = '\0';
    }
    w.getBytes(reinterpret_cast<uint8_t *>(pass), pass_cap);
    if (pass_cap > 0) {
        pass[pass_cap - 1] = '\0';
    }
    return true;
}

bool romeos_wifi_nvs_save_home(const char *ssid, const char *pass)
{
    if (!ssid || ssid[0] == '\0') {
        return false;
    }
    if (!pass) {
        pass = "";
    }
    if (std::strlen(ssid) > k_max_ssid || std::strlen(pass) > k_max_pass) {
        return false;
    }
    Preferences p;
    if (!p.begin(k_ns, false)) {
        return false;
    }
    p.putString(k_ssid, ssid);
    p.putString(k_pass, pass);
    p.end();
    return true;
}

bool romeos_wifi_nvs_clear_home(void)
{
    Preferences p;
    if (!p.begin(k_ns, false)) {
        return false;
    }
    p.remove(k_ssid);
    p.remove(k_pass);
    p.end();
    return true;
}

bool romeos_wifi_nvs_apply_build_defaults(void)
{
    const char *ssid = ROMEOS_WIFI_HOME_SSID_DEFAULT;
    const char *pass = ROMEOS_WIFI_HOME_PASS_DEFAULT;
    if (!ssid || ssid[0] == '\0') {
        return false;
    }
    if (romeos_wifi_nvs_has_home()) {
        return false;
    }
    if (!romeos_wifi_nvs_save_home(ssid, pass ? pass : "")) {
        return false;
    }
    Serial.printf("[wifi_nvs] home Wi-Fi from build defaults: \"%s\"\n", ssid);
    return true;
}

bool romeos_wifi_nvs_poll_serial(void)
{
    const uint32_t now = millis();
    if (s_step != Step::Idle && now > s_step_deadline_ms) {
        Serial.println(F("[wifi_nvs] provision timeout — cancelled"));
        s_step = Step::Idle;
        buf_reset();
    }

    if (!append_serial_to_line()) {
        return false;
    }

    trim_inplace(s_line_buf);

    if (s_step == Step::WantSsid) {
        if (s_line_buf[0] == '\0') {
            Serial.println(F("[wifi_nvs] empty SSID — cancelled"));
            s_step = Step::Idle;
            buf_reset();
            return false;
        }
        std::strncpy(s_pending_ssid, s_line_buf, sizeof(s_pending_ssid) - 1);
        s_pending_ssid[sizeof(s_pending_ssid) - 1] = '\0';
        s_step = Step::WantPass;
        s_step_deadline_ms = now + k_provision_timeout_ms;
        Serial.println(F("[wifi_nvs] SSID ok — send password line (may be empty)"));
        buf_reset();
        return false;
    }

    if (s_step == Step::WantPass) {
        bool saved = false;
        if (romeos_wifi_nvs_save_home(s_pending_ssid, s_line_buf)) {
            Serial.println(F("[wifi_nvs] home Wi‑Fi saved to NVS"));
            saved = true;
        } else {
            Serial.println(F("[wifi_nvs] save failed (length or NVS open)"));
        }
        s_step = Step::Idle;
        buf_reset();
        return saved;
    }

    /* Idle: εντολές μίας γραμμής */
    if (strcmp(s_line_buf, "WIFI_HOME_SET") == 0) {
        s_step = Step::WantSsid;
        s_step_deadline_ms = now + k_provision_timeout_ms;
        Serial.println(F("[wifi_nvs] send SSID line, then password line"));
        buf_reset();
        return false;
    }
    if (strcmp(s_line_buf, "WIFI_HOME_CLR") == 0) {
        bool cleared = false;
        if (romeos_wifi_nvs_clear_home()) {
            Serial.println(F("[wifi_nvs] home Wi‑Fi cleared from NVS"));
            cleared = true;
        } else {
            Serial.println(F("[wifi_nvs] clear failed"));
        }
        buf_reset();
        return cleared;
    }
    if (strcmp(s_line_buf, "WIFI_HOME_SHOW") == 0) {
        char ssid[40];
        char pass[68];
        if (romeos_wifi_nvs_load_home(ssid, sizeof(ssid), pass, sizeof(pass))) {
            Serial.printf("[wifi_nvs] stored SSID: \"%s\"  password: %s\n",
                          ssid,
                          pass[0] ? "(set, hidden)" : "(empty)");
        } else {
            Serial.println(F("[wifi_nvs] no home Wi‑Fi stored"));
        }
        buf_reset();
        return false;
    }
    if (strcmp(s_line_buf, "WIFI_HOME_HELP") == 0) {
        Serial.println(F("[wifi_nvs] WIFI_HOME_SET → then line SSID → then line password"));
        Serial.println(F("[wifi_nvs] WIFI_HOME_SHOW | WIFI_HOME_CLR | WIFI_HOME_HELP"));
        buf_reset();
        return false;
    }

    buf_reset();
    return false;
}
