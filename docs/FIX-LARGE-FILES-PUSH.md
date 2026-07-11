# Push απέτυχε — μεγάλα αρχεία

## Το πρόβλημα

```
romeos-display-v5.zip  → 110 MB  (όριο GitHub: 100 MB)
romeos-display-v5.rar  →  96 MB  (προειδοποίηση > 50 MB)
```

Η αντιγραφή **πέτυχε** (2661 αρχεία τοπικά). Το push **απορρίφθηκε**.

Τα `.zip` / `.rar` **μένουν τοπικά** στο `D:\ROMEOS 69` — δεν χρειάζονται στο GitHub.

---

## Διόρθωση (πλήρης — μετά από merge + rejected push)

```powershell
cd "D:\ROMEOS 69"
git pull origin main
powershell -ExecutionPolicy Bypass -File ".\docs\scripts\fix-push-final.ps1"
```

Αυτό: abort merge → backup archive → reset → restore → commit **χωρίς** zip/rar → push.

---

## Παλιό script (αν δεν υπάρχει merge)

## Χειροκίνητα (αν προτιμάς)

```powershell
cd "D:\ROMEOS 69"
git pull origin main
git reset --soft HEAD~1

git reset HEAD docs\archive\romeos-tsakas\romeos-display-v5.zip
git reset HEAD docs\archive\romeos-tsakas\romeos-display-v5.rar

git add .gitignore docs\archive\romeos-tsakas
git commit -m "Full copy Romeos Tsakas archive (exclude large zip/rar)"
git push origin main
```

---

## Μετά το push

Πες «έλεγξε» — θα δω τα docs στο cloud (συζήτηση, fan motor, HP board).

Τα `.zip`/`.rar` παραμένουν στο PC σου στο `docs\archive\romeos-tsakas\`.
