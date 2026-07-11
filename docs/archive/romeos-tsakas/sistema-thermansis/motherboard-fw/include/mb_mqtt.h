#pragma once

/** MQTT προς cloud broker — τηλεχειρισμός από κινητό (βλ. README + platformio.ini). */
void mb_mqtt_begin();
void mb_mqtt_poll();
/** True όταν το MQTT είναι συνδεδεμένο (για πολιτική «remote wins» έναντι UDP οθόνης). */
bool mb_mqtt_is_connected();
