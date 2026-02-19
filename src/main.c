/*
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/device.h>

#include "transport/dtm_transport.h"

int main(void)
{
	int err;
	union dtm_tr_packet cmd;

	err = dtm_tr_init();
	if (err) {
		return err;
	}

	for (;;) {
		cmd = dtm_tr_get();
		err = dtm_tr_process(cmd);
		if (err) {
			return err;
		}
	}
}
