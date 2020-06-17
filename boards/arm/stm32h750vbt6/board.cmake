# SPDX-License-Identifier: Apache-2.0

board_runner_args(openocd "--config=${BOARD_DIR}/support/openocd_stm32h750vbt6.cfg")

include(${ZEPHYR_BASE}/boards/common/openocd.board.cmake)
