# Panel hub — κείμενο για AI background + περιεχόμενο detail

**Ημερομηνία:** 2026-07-19  
**Σκοπός:** να δοθεί σε AI image generator για background / εικονίδια · **όχι** κλείδωμα οπτικού  
**Οθόνη:** Viewe 7″ · 800×480 · λεβητοστάσιο (όχι θερμοστάτης χώρου)

---

## Α. Ονόματα πλαισίων (πρόταση — OK να αλλάξουν)

| # | Όνομα στην κύρια | Αγγλικά (για AI) |
|---|------------------|------------------|
| 1 | **ΝΕΡΟ** | WATER |
| 2 | **ΑΝΤΛΙΑ** | HEAT PUMP |
| 3 | **ΕΞΟΔΟΙ** | OUTPUTS |
| 4 | **ΜΠΟΪΛΕΡ** | BOILER / BUFFER |
| 5 | **ΣΥΣΤΗΜΑ** | SYSTEM |
| 6 | **ΔΙΚΤΥΟ** | NETWORK |

> Άλλαξα το «ΣΥΝΔΕΣΕΙΣ» → **ΔΙΚΤΥΟ** (πιο κοντό στο εικονίδιο · ίδιο νόημα: Wi‑Fi / links). Αν προτιμάς ΣΥΝΔΕΣΕΙΣ, κράτα το.

---

## Β. Τι περιέχει κάθε δεύτερη οθόνη (για σένα + για το AI)

### 1 — ΝΕΡΟ
Θερμοκρασίες νερού υδραυλικού κυκλώματος: προσαγωγή / επιστροφή ενδοδαπέδιας, είσοδος / έξοδος εναλλάκτη θερμότητας, και όποιος άλλος αισθητήρας νερού υπάρχει στο δίκτυο σωληνώσεων (όχι μπόιλερ — αυτό στο #4). Λίστα ενδείξεων °C.

### 2 — ΑΝΤΛΙΑ
Λειτουργία αντλίας θερμότητας: κατάσταση ON/OFF ή mode, απόψυξη αν υπάρχει, κατανάλωση ρεύματος / ισχύς, σχετικά σήματα της μονάδας HP (όχι η λίστα όλων των ρελέ — αυτά στο #3).

### 3 — ΕΞΟΔΟΙ
Οι ηλεκτρικές έξοδοι των μητρικών (ρελέ): κυκλοφορητές, τρίοδη βάνα, εντολή HP, αντίσταση 4 kW, spare — κατάσταση ON/OFF για κάθε έξοδο. Όχι φώτα σπιτιού, όχι ρολά.

### 4 — ΜΠΟΪΛΕΡ
Μπόιλερ και διαχωριστής / buffer: θερμοκρασία πάνω μέρους μπόιλερ, κάτω μέρους, θερμοκρασία διαχωριστή, σχετικά με αποθήκευση ζεστού νερού.

### 5 — ΣΥΣΤΗΜΑ
Υγεία / συναγερμοί εγκατάστασης: βλάβες, συναγερμοί, defrost flag, γενική κατάσταση συστήματος λεβητοστασίου.

### 6 — ΔΙΚΤΥΟ
Επικοινωνίες: αν η μητρική Alpha είναι στο Wi‑Fi, αν μιλάει με τον θερμοστάτη χώρου, αν μιλάει με τη δεύτερη μητρική (Beta), κατάσταση καλωδίου οθόνης panel.

---

## Γ. Κείμενο έτοιμο για επικόλληση σε AI (Ελληνικά)

```
Φτιάξε background για οθόνη βιομηχανικού πίνακα λεβητοστασίου (θερμανση νερού), ανάλυση 800x480, οριζόντια. Στυλ: καθαρό, μοντέρνο, τεχνικό, σκούρο μπλε-γκρι μεταλλικό φόντο με διακριτική υφή σωλήνων/νερού χωρίς να κλέβει τα εικονίδια. Χωρίς κείμενο στο background. Χωρίς φώτα σπιτιού, χωρίς ρολά, χωρίς σαλόνι. Ατμόσφαιρα μηχανοστασίου / λεβητοστασίου.

Η κύρια οθόνη έχει 6 μεγάλα ισομεγέθη πλακίδια σε πλέγμα 2 γραμμές × 3 στήλες:

1 ΝΕΡΟ — θερμοκρασίες νερού σωληνώσεων και εναλλάκτη
2 ΑΝΤΛΙΑ — αντλία θερμότητας, λειτουργία και κατανάλωση
3 ΕΞΟΔΟΙ — κυκλοφορητές και ηλεκτρικές έξοδοι ρελέ
4 ΜΠΟΪΛΕΡ — μπόιλερ και διαχωριστής/buffer
5 ΣΥΣΤΗΜΑ — συναγερμοί και κατάσταση συστήματος
6 ΔΙΚΤΥΟ — Wi‑Fi και επικοινωνίες μεταξύ συσκευών

Άφησε κενό χώρο στο κέντρο κάθε πλακιδίου για εικονίδιο. Περιθώρια ομοιόμορφα. Όχι φωτορεαλιστικά πρόσωπα. Flat / soft 3D technical UI aesthetic.
```

---

## Δ. English prompt (συχνά καλύτερο αποτέλεσμα σε AI εικόνας)

```
Create a UI background for a boiler-room control panel touchscreen, 800x480 landscape. Style: clean modern industrial HMI, dark blue-grey metal, subtle pipe/water technical texture, not busy. No text baked into the image. No home lighting, no blinds, no living room. Plant room / mechanical room mood.

Main screen layout: six equal large tiles in a 2x3 grid for these sections:
1 WATER — hydronic water temperatures, floor heating supply/return, heat exchanger in/out
2 HEAT PUMP — heat pump status, operation, power consumption
3 OUTPUTS — circulator pumps and relay outputs (not household lights)
4 BOILER — hot water cylinder and buffer/separator tank
5 SYSTEM — alarms, faults, system health
6 NETWORK — Wi‑Fi and device communication links (motherboard, room thermostat, second board)

Leave clear empty center space in each tile for an icon. Even margins and gaps. Flat soft-technical UI aesthetic, no people, no photoreal clutter.
```

---

## Ε. Προαιρετικά — ξεχωριστά prompts ανά εικονίδιο πλακιδίου

| Tile | Prompt ιδέα εικονιδίου |
|------|------------------------|
| ΝΕΡΟ | thermometer + water drop + pipe |
| ΑΝΤΛΙΑ | outdoor heat pump unit / compressor |
| ΕΞΟΔΟΙ | circulator pump or toggle switches row |
| ΜΠΟΪΛΕΡ | vertical cylinder tank with levels |
| ΣΥΣΤΗΜΑ | warning triangle / shield status |
| ΔΙΚΤΥΟ | wifi + linked nodes / cable link |

---

*Για χρήση στο EEZ · οπτικό κλείδωμα μόνο με ρητή εντολή χρήστη.*
