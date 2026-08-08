# Alpha ↔ Beta — τεστ ρελέ μέσω UART

1. **Flash Beta** (μετακίνησε USB από Alpha → Beta · συνήθως πάλι COM7):
   ```powershell
   cd "D:\ROMEOS 69\board-link-test"
   pio run -e beta_relay -t upload --upload-port COM7
   ```
   Αν flash fail (GPIO12): βγάλε προσωρινά το ESP της Beta από τις βάσεις, flash, ξαναβάλε.

2. **Βγάλε USB από Beta** (μένει με 12→5). Βάλε USB πίσω στην **Alpha COM7**.

3. **Flash Alpha**:
   ```powershell
   pio run -e alpha_cmd -t upload --upload-port COM7
   pio device monitor -e alpha_cmd
   ```

4. Και οι δύο με 12 V · **χωρίς 230 V** στα contacts των ρελέ.

Μετά το power-up η Beta κάνει **chase K1→K8** μόνη της. Μετά: `PULSE` από Alpha · monitor: `sent` + `got: ACK n`.

**Map:** διπλά modules έχουν IN1↔IN2 σταυρωτά ως προς silk LED-Kx — το FW κάνει swap ανά ζεύγος ώστε μαρκαδόρος ρελέ N = LED-KN = PULSE n.

**Idle LED πλακέτας:** κύκλωμα `5V→R→LED→GPIO`· με GPIO=HIGH το LED έμενε αμυδρά ON. Idle = Hi-Z (INPUT) για να σβήνει.
