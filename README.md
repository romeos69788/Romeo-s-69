# ROMEOS 69

**Τοπικός φάκελος project:** `D:\ROMEOS 69`  
**GitHub repo:** https://github.com/romeos69788/Romeo-s-69

Σύστημα ελέγχου θέρμανσης / αντλίας θερμότητας · **Alpha rev A** (μητρική καυστήρα).

> Όλη η δουλειά και οι συζητήσεις με Cursor βασίζονται σε **αυτόν τον φάκελο**.
> Κάθε συνέχεια = άνοιγμα `D:\ROMEOS 69` στο Cursor.

---

## Δομή φακέλου (`D:\ROMEOS 69`)

```
D:\ROMEOS 69\
├── README.md                 ← αυτό το αρχείο
├── ΣΥΖΗΤΗΣΗ.md               ← καταγραφή όλων των συζητήσεων (ενημερώνεται)
├── ΣΥνδεσμοι.md              ← GitHub, PR, agent links
├── motherboard-fw\           ← firmware Alpha rev A (PlatformIO)
├── romeos-remote\            ← web app iPhone (React + MQTT)
├── scripts\                  ← sync / setup (Windows)
└── .cursor\rules\            ← context για Cursor agents
```

---

## Πρώτη εγκατάσταση στο PC

### Α) Νέος φάκελος (κενός `D:\ROMEOS 69`)

```powershell
cd D:\
git clone https://github.com/romeos69788/Romeo-s-69.git "ROMEOS 69"
cd "ROMEOS 69"
git checkout cursor/alpha-rev-a-fw-fc5f
```

### Β) Έχεις ήδη αρχεία στο `D:\ROMEOS 69`

```powershell
cd "D:\ROMEOS 69"
git init
git remote add origin https://github.com/romeos69788/Romeo-s-69.git
git fetch origin
git checkout -b cursor/alpha-rev-a-fw-fc5f origin/cursor/alpha-rev-a-fw-fc5f
# Αν υπάρχουν τοπικά αρχεία: git status → merge χειροκίνητα ή backup πρώτα
```

### Γ) Sync μετά από κάθε session agent

```powershell
cd "D:\ROMEOS 69"
git pull origin cursor/alpha-rev-a-fw-fc5f
```

---

## Build firmware (Alpha rev A)

```powershell
cd "D:\ROMEOS 69\motherboard-fw"
pio run -e alpha_rev_a_bench
```

Hardware docs: [`motherboard-fw/docs/ALPHA-REV-A.md`](motherboard-fw/docs/ALPHA-REV-A.md)

## Remote app (iPhone)

```powershell
cd "D:\ROMEOS 69\romeos-remote"
npm install
npm run dev
```

Live URL: https://romeos69788.github.io/romeos-remote/

---

## Αρχιτεκτονική

```
┌─────────────┐   UART    ┌─────────────┐
│ Alpha (MB)  │◄─────────►│ Beta        │
│ boiler room │           │ outdoor     │
└──────┬──────┘           └─────────────┘
       │ UART / ESP-NOW
       ▼
┌─────────────┐   MQTT      ┌─────────────┐
│ Displays    │             │ romeos-     │
│ (thermostat)│             │ remote      │
└─────────────┘             └──────┬──────┘
                                   │ HiveMQ Cloud
                                   ▼
                            ┌─────────────┐
                            │ Alpha (MB)  │
                            └─────────────┘
```

## Hardware notes

- **CT:** μόνο SCT-013 στη γραμμή HP → GPIO35
- **TUYA WiFi meter** στον πίνακα καυστήρα (όχι CT)
- **RS485 / MAX485EPA:** reserved για future HP outdoor board

## Συζήτηση & ιστορικό

Όλες οι συζητήσεις: [`ΣΥΖΗΤΗΣΗ.md`](ΣΥΖΗΤΗΣΗ.md)
