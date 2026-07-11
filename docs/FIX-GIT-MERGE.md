# Git: διόρθωση merge conflict + push

Αν βλέπεις **unmerged files (U)** ή **push rejected (non-fast-forward)**, η πιο απλή λύση:

## Επιλογή Α — Καθαρό clone (προτείνεται)

```powershell
cd D:\
Rename-Item "ROMEOS 69" "ROMEOS 69-backup-$(Get-Date -Format yyyyMMdd)"

git clone https://github.com/romeos69788/Romeo-s-69.git "ROMEOS 69"
cd "D:\ROMEOS 69"

git remote -v
powershell -ExecutionPolicy Bypass -File ".\docs\scripts\copy-romeos-tsakas-full.ps1"

$count = (Get-ChildItem docs\archive\romeos-tsakas -Recurse -File).Count
Write-Host "Archive files: $count"

git add docs\archive\romeos-tsakas docs\HP-BOARD-LEITOURGIA.md
git commit -m "Full copy Romeos Tsakas archive"
git push origin main
```

Τα παλιά τοπικά αρχεία μένουν στο `ROMEOS 69-backup-...`.

---

## Επιλογή Β — Διόρθωση στο τρέχον repo

```powershell
cd "D:\ROMEOS 69"

git remote set-url origin https://github.com/romeos69788/Romeo-s-69.git
git merge --abort

git fetch origin
git reset --hard origin/main
```

Προσοχή: χάνεις τοπικό commit `fbaccc6`. Μετά:

```powershell
git pull origin main
powershell -ExecutionPolicy Bypass -File ".\docs\scripts\copy-romeos-tsakas-full.ps1"
git add docs\archive\romeos-tsakas
git commit -m "Full copy Romeos Tsakas archive"
git push origin main
```

---

## Έλεγχος remote (πρέπει Romeo-s-69)

```powershell
git remote -v
```

Σωστό:
`https://github.com/romeos69788/Romeo-s-69.git`

Λάθος:
`https://github.com/romeos69788/romeos-69.git`

---

## Script syntax error

Διορθώθηκε το `copy-romeos-tsakas-full.ps1` (nested quotes). Μετά `git pull` τρέξε ξανά το script.
