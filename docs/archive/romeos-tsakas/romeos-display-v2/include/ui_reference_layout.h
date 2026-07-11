/**
 * @file ui_reference_layout.h
 *
 * Μεταδεδομένα διάταξης για το πλήρες background `ui_reference.png` (800x480 px).
 *
 * Σύστημα συντεταγμένων:
 *   - Μονάδα: εικονοστοιχεία (px), ακέραιοι.
 *   - Αρχή (0,0): πάνω-αριστερή γωνία της οθόνης· +X δεξιά· +Y κάτω.
 *   - Για κάθε στοιχείο: (cx, cy) = κέντρο ορθογωνίου περιβάλλοντος (bounding box).
 *   - (w, h) = πλάτος και ύψος του ίδιου πλαισίου.
 *
 * Πηγή: οπτική ανάλυση του raster `ui_reference.png` (800x480). Για αλφαριθμητικά,
 * τα per-glyph κουτιά είναι ισομερή χωρίσματα του parent box (ίσο πλάτος ανά
 * χαρακτήρα) — όταν μπει πραγματική γραμματοσειρά LVGL, αντικατέστησέ τα με
 * μέτρηση από την ίδια τη γραμματοσειρά.
 *
 * Χάρτης αναφοράς για επόμενα βήματα· δεν συνδέεται αυτόματα στο build.
 */

#ifndef UI_REFERENCE_LAYOUT_H
#define UI_REFERENCE_LAYOUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UI_LAYOUT_DISPLAY_W 800
#define UI_LAYOUT_DISPLAY_H 480

typedef struct {
    int16_t cx;
    int16_t cy;
    int16_t w;
    int16_t h;
} ui_layout_rect_t;

static inline int16_t ui_layout_left(const ui_layout_rect_t *r)
{
    return (int16_t)(r->cx - r->w / 2);
}

static inline int16_t ui_layout_top(const ui_layout_rect_t *r)
{
    return (int16_t)(r->cy - r->h / 2);
}

typedef struct {
    const char *utf8_char;
    int16_t cx;
    int16_t cy;
    int16_t w;
    int16_t h;
} ui_layout_glyph_t;

/* --- Άνω μπάρα -------------------------------------------------------------- */

static const ui_layout_rect_t kUiTopFanPill = {88, 35, 135, 45};
static const ui_layout_rect_t kUiTopCycle1Pill = {278, 35, 145, 45};
static const ui_layout_rect_t kUiTopCycle2Pill = {478, 35, 145, 45};

/* --- Κεντρική πλάκα ---------------------------------------------------------- */

static const ui_layout_rect_t kUiPlateTopLeftTemp = {108, 125, 95, 65};
static const ui_layout_rect_t kUiPlateTopLeftLabel = {108, 185, 95, 30};
static const ui_layout_rect_t kUiPlateTopRightTemp = {532, 125, 95, 65};
static const ui_layout_rect_t kUiPlateTopRightLabel = {532, 185, 95, 30};
static const ui_layout_rect_t kUiPlateMainTemp = {320, 235, 171, 135};
static const ui_layout_rect_t kUiPlateMinusBtn = {170, 355, 285, 85};
static const ui_layout_rect_t kUiPlatePlusBtn = {470, 355, 285, 85};

/* --- Κάτω μπάρα ------------------------------------------------------------ */

static const ui_layout_rect_t kUiBottomOnOff = {100, 445, 165, 55};
static const ui_layout_rect_t kUiBottomBack = {258, 445, 100, 55};
static const ui_layout_rect_t kUiBottomHome = {385, 445, 115, 55};
static const ui_layout_rect_t kUiBottomForward = {512, 445, 100, 55};

/* --- Δεξιά στήλη (3 σειρές) -------------------------------------------------- */

static const ui_layout_rect_t kUiRightR1IntelLabel = {628, 102, 88, 26};
static const ui_layout_rect_t kUiRightR1IntelValue = {628, 152, 105, 52};
static const ui_layout_rect_t kUiRightR1OutletLabel = {738, 102, 88, 26};
static const ui_layout_rect_t kUiRightR1OutletValue = {738, 152, 105, 52};

static const ui_layout_rect_t kUiRightR2IntelLabel = {628, 252, 88, 26};
static const ui_layout_rect_t kUiRightR2IntelValue = {628, 302, 105, 52};
static const ui_layout_rect_t kUiRightR2OutletLabel = {738, 252, 88, 26};
static const ui_layout_rect_t kUiRightR2OutletValue = {738, 302, 105, 52};

static const ui_layout_rect_t kUiRightR3IntelLabel = {628, 402, 88, 26};
static const ui_layout_rect_t kUiRightR3IntelValue = {628, 440, 105, 52};
static const ui_layout_rect_t kUiRightR3OutletLabel = {738, 402, 88, 26};
static const ui_layout_rect_t kUiRightR3OutletValue = {738, 440, 105, 52};

/* --- Glyph: "42°" (πάνω πλάκα αριστερά / δεξιά) ----------------------------- */

#define UI_GLYPH_42DEG_COUNT 3
static const ui_layout_glyph_t kUiGlyphsPlateTopLeftTemp[UI_GLYPH_42DEG_COUNT] = {
    {"4", 76, 125, 32, 65},
    {"2", 108, 125, 32, 65},
    {"\xc2\xb0", 140, 125, 31, 65},
};

static const ui_layout_glyph_t kUiGlyphsPlateTopRightTemp[UI_GLYPH_42DEG_COUNT] = {
    {"4", 500, 125, 32, 65},
    {"2", 532, 125, 32, 65},
    {"\xc2\xb0", 564, 125, 31, 65},
};

/* --- Glyph: "INTEL" (ετικέτες πλάκας αριστερά / δεξιά) ---------------------- */

#define UI_GLYPH_INTEL_5 5
static const ui_layout_glyph_t kUiGlyphsPlateTopLeftIntel[UI_GLYPH_INTEL_5] = {
    {"I", 78, 185, 19, 30},
    {"N", 97, 185, 19, 30},
    {"T", 116, 185, 19, 30},
    {"E", 135, 185, 19, 30},
    {"L", 154, 185, 19, 30},
};

static const ui_layout_glyph_t kUiGlyphsPlateTopRightIntel[UI_GLYPH_INTEL_5] = {
    {"I", 502, 185, 19, 30},
    {"N", 521, 185, 19, 30},
    {"T", 540, 185, 19, 30},
    {"E", 559, 185, 19, 30},
    {"L", 578, 185, 19, 30},
};

/* --- Κεντρικό "21°": ακριβείς μετρήσεις + ψηφία αφαιρέθηκαν από PNG -> ui_reference_main_dial_measure.h */

#define UI_GLYPH_21DEG_MAIN 3
static const ui_layout_glyph_t kUiGlyphsPlateMainTemp[UI_GLYPH_21DEG_MAIN] = {
    {"2", 263, 235, 57, 135},
    {"1", 320, 235, 57, 135},
    {"\xc2\xb0", 377, 235, 57, 135},
};

/* --- Glyph: "42°C" / "35°C" (δεξιά στήλη) ------------------------------------ */

#define UI_GLYPH_TEMP_C_4 4

static const ui_layout_glyph_t kUiGlyphsRightR1IntelValue[UI_GLYPH_TEMP_C_4] = {
    {"4", 589, 152, 26, 52},
    {"2", 615, 152, 26, 52},
    {"\xc2\xb0", 641, 152, 26, 52},
    {"C", 667, 152, 26, 52},
};

static const ui_layout_glyph_t kUiGlyphsRightR1OutletValue[UI_GLYPH_TEMP_C_4] = {
    {"3", 699, 152, 26, 52},
    {"5", 725, 152, 26, 52},
    {"\xc2\xb0", 751, 152, 26, 52},
    {"C", 777, 152, 26, 52},
};

static const ui_layout_glyph_t kUiGlyphsRightR2IntelValue[UI_GLYPH_TEMP_C_4] = {
    {"4", 589, 302, 26, 52},
    {"2", 615, 302, 26, 52},
    {"\xc2\xb0", 641, 302, 26, 52},
    {"C", 667, 302, 26, 52},
};

static const ui_layout_glyph_t kUiGlyphsRightR2OutletValue[UI_GLYPH_TEMP_C_4] = {
    {"3", 699, 302, 26, 52},
    {"5", 725, 302, 26, 52},
    {"\xc2\xb0", 751, 302, 26, 52},
    {"C", 777, 302, 26, 52},
};

static const ui_layout_glyph_t kUiGlyphsRightR3IntelValue[UI_GLYPH_TEMP_C_4] = {
    {"4", 589, 440, 26, 52},
    {"2", 615, 440, 26, 52},
    {"\xc2\xb0", 641, 440, 26, 52},
    {"C", 667, 440, 26, 52},
};

static const ui_layout_glyph_t kUiGlyphsRightR3OutletValue[UI_GLYPH_TEMP_C_4] = {
    {"3", 699, 440, 26, 52},
    {"5", 725, 440, 26, 52},
    {"\xc2\xb0", 751, 440, 26, 52},
    {"C", 777, 440, 26, 52},
};

/* "INTEL" δεξιά στήλη — 5 γράμματα × 3 σειρές */
#define UI_GLYPH_INTEL_ROW 5
static const ui_layout_glyph_t kUiGlyphsRightR1IntelLabel[UI_GLYPH_INTEL_ROW] = {
    {"I", 593, 102, 17, 26},
    {"N", 610, 102, 17, 26},
    {"T", 627, 102, 17, 26},
    {"E", 644, 102, 17, 26},
    {"L", 661, 102, 17, 26},
};

static const ui_layout_glyph_t kUiGlyphsRightR2IntelLabel[UI_GLYPH_INTEL_ROW] = {
    {"I", 593, 252, 17, 26},
    {"N", 610, 252, 17, 26},
    {"T", 627, 252, 17, 26},
    {"E", 644, 252, 17, 26},
    {"L", 661, 252, 17, 26},
};

static const ui_layout_glyph_t kUiGlyphsRightR3IntelLabel[UI_GLYPH_INTEL_ROW] = {
    {"I", 593, 402, 17, 26},
    {"N", 610, 402, 17, 26},
    {"T", 627, 402, 17, 26},
    {"E", 644, 402, 17, 26},
    {"L", 661, 402, 17, 26},
};

/* "OUTLET" — 6 γράμματα */
#define UI_GLYPH_OUTLET_6 6
static const ui_layout_glyph_t kUiGlyphsRightR1OutletLabel[UI_GLYPH_OUTLET_6] = {
    {"O", 687, 102, 14, 26},
    {"U", 701, 102, 14, 26},
    {"T", 715, 102, 14, 26},
    {"L", 729, 102, 14, 26},
    {"E", 743, 102, 14, 26},
    {"T", 757, 102, 14, 26},
};

static const ui_layout_glyph_t kUiGlyphsRightR2OutletLabel[UI_GLYPH_OUTLET_6] = {
    {"O", 687, 252, 14, 26},
    {"U", 701, 252, 14, 26},
    {"T", 715, 252, 14, 26},
    {"L", 729, 252, 14, 26},
    {"E", 743, 252, 14, 26},
    {"T", 757, 252, 14, 26},
};

static const ui_layout_glyph_t kUiGlyphsRightR3OutletLabel[UI_GLYPH_OUTLET_6] = {
    {"O", 687, 402, 14, 26},
    {"U", 701, 402, 14, 26},
    {"T", 715, 402, 14, 26},
    {"L", 729, 402, 14, 26},
    {"E", 743, 402, 14, 26},
    {"T", 757, 402, 14, 26},
};

/* Κουμπιά + / − (σύμβολο μέσα στο hit area του κουμπιού) */
static const ui_layout_glyph_t kUiGlyphMinus = {"-", 170, 355, 80, 85};
static const ui_layout_glyph_t kUiGlyphPlus = {"+", 470, 355, 80, 85};

/* Άνω pills: "FAN", "CYCLE 1", "CYCLE 2" — ισομερές χώρισμα κειμένου στο pill */
#define UI_GLYPH_FAN_3 3
static const ui_layout_glyph_t kUiGlyphsTopFan[UI_GLYPH_FAN_3] = {
    {"F", 44, 35, 45, 45},
    {"A", 89, 35, 45, 45},
    {"N", 134, 35, 45, 45},
};

#define UI_GLYPH_CYCLE1_7 7
static const ui_layout_glyph_t kUiGlyphsTopCycle1[UI_GLYPH_CYCLE1_7] = {
    {"C", 216, 35, 21, 45},
    {"Y", 237, 35, 21, 45},
    {"C", 258, 35, 21, 45},
    {"L", 279, 35, 21, 45},
    {"E", 300, 35, 21, 45},
    {" ", 321, 35, 21, 45},
    {"1", 342, 35, 21, 45},
};

#define UI_GLYPH_CYCLE2_7 7
static const ui_layout_glyph_t kUiGlyphsTopCycle2[UI_GLYPH_CYCLE2_7] = {
    {"C", 416, 35, 21, 45},
    {"Y", 437, 35, 21, 45},
    {"C", 458, 35, 21, 45},
    {"L", 479, 35, 21, 45},
    {"E", 500, 35, 21, 45},
    {" ", 521, 35, 21, 45},
    {"2", 542, 35, 21, 45},
};

/* Κάτω: "ON/OFF" (5 χαρακτήρες) */
#define UI_GLYPH_ONOFF_5 5
static const ui_layout_glyph_t kUiGlyphsBottomOnOff[UI_GLYPH_ONOFF_5] = {
    {"O", 62, 445, 28, 55},
    {"N", 90, 445, 28, 55},
    {"/", 118, 445, 14, 55},
    {"O", 132, 445, 28, 55},
    {"F", 160, 445, 28, 55},
};

static const ui_layout_glyph_t kUiGlyphBottomBack = {"<", 258, 445, 45, 50};
static const ui_layout_glyph_t kUiGlyphBottomForward = {">", 512, 445, 45, 50};

#ifdef __cplusplus
}
#endif

#endif /* UI_REFERENCE_LAYOUT_H */
