/**
 * @file board.h
 * @author Marko Fuček
 * @brief Definicije i tipovi vezani uz UART sučelje na apstraktnoj pločici.
 * 
 * Ova datoteka sadrži osnovne tipove i konstante za identifikaciju UART perifernih
 * sučelja koja se koriste na pločici.
 * 
 * @version 0.1
 * @date 2026-01-22
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include "stdint.h"

/**
 * @typedef BoardUartId
 * @brief Logički identifikator UART sučelja na pločici.
 * 
 */
typedef int32_t BoardUartId;

/**
 * @brief Neinicijalizirano UART sučelje.
 * 
 * Predstavlja neinicijalizirani UART ili se koristi dok još nije dodijeljen UART id.
 * 
 */
#define BOARD_UART_UNINIT ((BoardUartId)-1)

/**
 * @brief UART sučelje za konzolu.
 * 
 * Tipično korištenje je za ispis poruka, debbug i komunikaciju s korisnikom preko serijskog monitora.
 * 
 */
#define BOARD_UART_CONSOLE ((BoardUartId)0)

/**
 * @brief UART sučelje za komunikaciju protokolom.
 * 
 * Koristi se za komunikaciju s vanjskim uređajima ili modulima preko definiranog protokola.
 * 
 */
#define BOARD_UART_PROTOCOL ((BoardUartId)1)