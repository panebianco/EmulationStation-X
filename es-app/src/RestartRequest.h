#pragma once
#ifndef ES_APP_RESTART_REQUEST_H
#define ES_APP_RESTART_REQUEST_H

#include <string>

// Marca que EmulationStation debe reiniciarse (relanzar el binario)
// Útil para cambios pesados de tema (layout/variables) que conviene aplicar en limpio.
void requestRestart(const std::string& reason);

// true si se pidió reinicio
bool isRestartRequested();

// motivo (texto simple) – opcional, para log
std::string getRestartReason();

#endif // ES_APP_RESTART_REQUEST_H
