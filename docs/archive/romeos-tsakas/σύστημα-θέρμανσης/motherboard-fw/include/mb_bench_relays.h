#pragma once

#ifdef ROMEOS_MB_BENCH_RELAYS_ON
/** Κλείσιμο ρελέ bench κατά τη διάρκεια 1-Wire (τροφοδοσία CN4). */
void mb_bench_relays_quiet_enter();
void mb_bench_relays_quiet_leave();
#else
inline void mb_bench_relays_quiet_enter() {}
inline void mb_bench_relays_quiet_leave() {}
#endif
