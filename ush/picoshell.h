/**
 * @file picoshell.h
 * @brief client code for pico shell.
 */

#ifndef PICOSHELL_H
#define PICOSHELL_H

#include "pico/stdlib.h"
#include "ush.h"
#include "ush_node.h"
#include "ush_types.h"
#include <stdio.h>
#include <string.h>

void picoshell_init(void);
void picoshell_service(void);

#endif /* PICOSHELL_H */
