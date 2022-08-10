/*
 * Copyright (c) 2022 Felix Gohla, Konrad Hanff, Tobias Kantusch,
 *                    Quentin Kuth, Felix Roth. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "fido.h"
#include "gpio.h"

#define SAMPLES 20

// See scripts/gen_aes_gcm.py
static const uint8_t key[] = { 0x7d, 0x1a, 0x22, 0xe8, 0x33, 0x8c, 0xf9, 0x8c, 0x18, 0x4a, 0x36, 0xd7, 0x1f, 0x53, 0x57, 0x06, 0xe7, 0x61, 0xab, 0x8c, 0xe9, 0xe3, 0xca, 0xde, 0x0e, 0x18, 0x60, 0xdf, 0xa9, 0xb2, 0x6f, 0x36 };
static const uint8_t associated_data[12] = "fidoonmicros";
static const uint8_t nonce[] = { 0xb7, 0x3a, 0x1b, 0x33, 0x1e, 0xe7, 0xef, 0x59, 0x44, 0xf8, 0x50, 0x73 };
static const uint8_t ciphertext[] = { 0x20, 0x21, 0x93, 0x59, 0x35, 0x51, 0xec, 0xaa, 0x5c, 0x63, 0xe2, 0x31, 0xbb, 0x1b, 0x12, 0x29, 0x1d, 0x12, 0x24, 0xe5, 0x63, 0x28, 0xfd, 0xcd, 0x8a, 0x3a, 0xee, 0x45, 0x9c, 0x22, 0x32, 0xcb, 0xba, 0x34, 0x2e, 0x18, 0xad, 0xe5, 0xd0, 0xfa, 0x8f, 0x4f, 0xfe, 0x3d, 0xfb, 0xe6, 0xd0, 0x84, 0x4f, 0xcd, 0x1d, 0xfd, 0x34, 0xec, 0x75, 0x7f, 0xef, 0x86, 0x71, 0x37, 0x3c, 0x4b, 0xf1, 0x68, 0x64, 0x21, 0x70, 0x65, 0x82, 0x2c, 0xe6, 0xfb, 0x81, 0x55, 0xfa, 0x87, 0x12, 0xd5, 0x43, 0xfa, 0x41, 0x75, 0x36, 0x47, 0xf3, 0xfe, 0x94, 0x8e, 0xb9, 0x6d, 0xa4, 0x9d, 0x84, 0x7e, 0xb0, 0x81, 0x45, 0x19, 0x78, 0xa2, 0x79, 0x91, 0x41, 0x45, 0x08, 0x04, 0x37, 0xf5, 0xa2, 0x1c, 0xdd, 0x24, 0x90, 0xda, 0xab, 0x76, 0x4e, 0xfb, 0xdc, 0x4f, 0x59, 0x3b, 0x55, 0xd8, 0x30, 0x50, 0xcf, 0x66, 0xe9, 0x7e, 0x42, 0x79, 0xe0, 0x5d, 0x95, 0x8a, 0x59, 0x94, 0xe3, 0x02, 0xfa, 0xcf, 0xf1, 0x10, 0x0e, 0xd1, 0x9d, 0xef, 0x01, 0x53, 0x51, 0x1d, 0xeb, 0xc8, 0x5c, 0x88, 0x74, 0xb5, 0xce, 0x8d, 0x64, 0x9b, 0x55, 0x45, 0xd2, 0xcb, 0xbf, 0xce, 0x62, 0xc2, 0x86, 0x91, 0x05, 0xca, 0xca, 0x63, 0xa3, 0x56, 0x48, 0x0d, 0x14, 0x27, 0x20, 0x0a, 0xa2, 0xfe, 0x14, 0x3b, 0x15, 0x90, 0x83, 0x97, 0xa4, 0x98, 0x07, 0xeb, 0x23, 0xa2, 0x36, 0x16, 0x2c, 0x8d, 0xb6, 0x38, 0xb5, 0xa8, 0x46, 0x1d, 0xde, 0x6e, 0x6e, 0xbd, 0xeb, 0x31, 0xd1, 0x8a, 0xa9, 0x3d, 0xee, 0x10, 0x79, 0xc7, 0x2a, 0xbc, 0xa4, 0x83, 0xe7, 0xae, 0xd9, 0xaa, 0xdd, 0xef, 0x3d, 0x37, 0x1d, 0x73, 0x73, 0x19, 0x4f, 0x94, 0x2d, 0xb1, 0xa6, 0x24, 0x70, 0x36, 0x35, 0xb9, 0x10, 0x2b, 0x81, 0x2e, 0x21, 0x8c, 0x41, 0xef, 0x7b, 0x76, 0xd1, 0x49, 0x10, 0x0d, 0x18, 0x8f, 0x1e, 0x9d, 0xb6, 0x40, 0xef, 0x76, 0xf5, 0xb6, 0xc1, 0xf8, 0xc5, 0x25, 0x1c, 0x34, 0x90, 0xa2, 0xb5, 0x38, 0xa8, 0x7e, 0x27, 0x44, 0xe6, 0x21, 0xb5, 0x4b, 0xca, 0x47, 0x60, 0x42, 0x40, 0xb2, 0x61, 0x3c, 0xb7, 0x10, 0xa6, 0xaa, 0x96, 0x91, 0xc9, 0x91, 0xdc, 0xbd, 0x79, 0xdd, 0xa4, 0xf7, 0xa7, 0x5b, 0x6c, 0x41, 0x89, 0x9e, 0x82, 0xbd, 0x13, 0x01, 0x8b, 0x5b, 0xf0, 0x2f, 0xd7, 0x96, 0xa0, 0x13, 0xad, 0x07, 0x75, 0x09, 0xc7, 0x4a, 0x87, 0xe1, 0x5e, 0x5e, 0xd3, 0xb5, 0xd1, 0xd1, 0xf1, 0xc6, 0xd6, 0x9c, 0x95, 0x77, 0xa6, 0x45, 0x4c, 0x2e, 0xce, 0x12, 0xd5, 0x90, 0xbe, 0x5c, 0x12, 0x76, 0xc3, 0x3c, 0x0d, 0x49, 0x66, 0x04, 0x0a, 0xc2, 0x5e, 0x7a, 0x35, 0xd4, 0xec, 0x7c, 0x2d, 0xc1, 0xba, 0x1f, 0x6d, 0x5b, 0x3d, 0x78, 0x09, 0xf2, 0xdc, 0x6b, 0x06, 0x20, 0x2c, 0x3b, 0x57, 0x5e, 0xb2, 0xec, 0x96, 0x87, 0x27, 0x07, 0x62, 0x51, 0xa6, 0x66, 0xd0, 0x61, 0xaf, 0x8f, 0x6d, 0xea, 0x67, 0xef, 0xcb, 0xb7, 0x99, 0xf4, 0xe6, 0xe5, 0x42, 0x30, 0x44, 0xf6, 0x9f, 0xc2, 0x27, 0x0d, 0x55, 0x4b, 0xc2, 0x97, 0x15, 0x68, 0x9b, 0x13, 0x93, 0x47, 0xf5, 0x0b, 0xd0, 0x28, 0xe5, 0x3a, 0x10, 0x2f, 0xb6, 0x8d, 0xc3, 0x9c, 0x2e, 0x10, 0x35, 0xc6, 0x9c, 0x23, 0xa0, 0xf1, 0x84, 0x21, 0x9f, 0x40, 0x78, 0x67, 0x6b, 0xf8, 0x3c, 0xeb, 0x3c, 0x9b, 0xa7, 0x30, 0x50, 0xcd, 0xf5, 0x5d, 0x73, 0x38, 0x7c, 0x9a, 0xfa, 0x21, 0xbf, 0xb9, 0xa4, 0xa3, 0xa3, 0x63, 0x49, 0xe5, 0x15, 0x23, 0x0d, 0x55, 0xb0, 0xf2, 0x28, 0xb3, 0xb2, 0x81, 0x47, 0x57, 0xea, 0x66, 0x08, 0x4e, 0xd0, 0x09, 0x1e, 0xf0, 0x21, 0xc2, 0x4f, 0x8b, 0x2b, 0x1f, 0x30, 0x2a, 0x48, 0x9f, 0xd5, 0x55, 0xb9, 0xde, 0x4d, 0xf6, 0xe2, 0x3a, 0xc9, 0x9b, 0xbf, 0xbe, 0xb1, 0xd4, 0x3c, 0x74, 0x09, 0x8d, 0x98, 0xa5, 0x51, 0x98, 0x5f, 0x19, 0x39, 0x82, 0xde, 0x19, 0x97, 0xa2, 0x95, 0x2a, 0x2e, 0x2f, 0x0e, 0xe3, 0x3a, 0xdb, 0x54, 0x37, 0x17, 0x84, 0xba, 0xb5, 0x44, 0xbb, 0xed, 0xfb, 0x57, 0xb4, 0xe6, 0xc4, 0x8d, 0xe4, 0x01, 0xb7, 0x84, 0x5f };
static const uint8_t tag[] = { 0x65, 0xc0, 0xe6, 0x07, 0x14, 0x30, 0x93, 0xde, 0x16, 0xc4, 0xa3, 0x28, 0x59, 0x54, 0x47, 0x09 };

#ifdef ESP_PLATFORM
int app_main(void) {
#else
int main(void) {
#endif
    // Wait until the microcontroller booted up to remove increased power consumption in the measurements at the beginning.
    delay(3000);

    uint8_t plain[576] = {0};

    setup_pin();
    pin_off();

    int e = 0;

    // Test AES GCM decryption.
    for (size_t i = 0; i < SAMPLES; ++i) {
        pin_on();
        e = fido_aes_gcm_decrypt(
            key,
            sizeof(key),
            nonce,
            sizeof(nonce),
            ciphertext,
            sizeof(ciphertext),
            associated_data,
            sizeof(associated_data),
            tag,
            plain
        );
        pin_off();

        delay(500);
    }

    delay(1000);
    pin_on();
    for (size_t i = 0; i < SAMPLES; ++i) {
        e = fido_aes_gcm_decrypt(
            key,
            sizeof(key),
            nonce,
            sizeof(nonce),
            ciphertext,
            sizeof(ciphertext),
            associated_data,
            sizeof(associated_data),
            tag,
            plain
        );
    }
    pin_off();

    return e;
}
