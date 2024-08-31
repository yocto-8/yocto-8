#include <pico/stdio.h>
#include <platform/asupico/asupico.hpp>

#include <cstdio>
#include <emu/tinyalloc.hpp>
#include <hardware/clocks.h>
#include <hardware/structs/qmi.h>
#include <hardware/structs/xip.h>
#include <hardware/sync.h>
#include <hardware/vreg.h>
#include <pico/stdlib.h>

#include <hardwarestate.hpp>

namespace arch::pico::platform::asupico {

HardwareState hw;

void init_default_frequency() {
	// FIXME: less jank and need to respect the write thing
	qmi_hw->m[0].timing = 0x60007203;

	vreg_set_voltage(VREG_VOLTAGE_1_25);
	set_sys_clock_khz(351000, true);

	init_stdio();
}

void init_stdio() { stdio_init_all(); }

void init_buttons() {
	hw.buttons[0].init(16);
	hw.buttons[1].init(18);
	hw.buttons[2].init(17);
	hw.buttons[3].init(19);
	hw.buttons[4].init(20);
	hw.buttons[5].init(21);
}

std::size_t __no_inline_not_in_flash_func(init_psram_pimoroni)() {
	// RP2350 QMI PSRAM initialization code from CircuitPython

	gpio_set_function(PIMORONI_PICO_PLUS2_PSRAM_CS_PIN, GPIO_FUNC_XIP_CS1);
	int psram_size;
	psram_size = 0;

	uint32_t interrupt_state = save_and_disable_interrupts();

	// Try and read the PSRAM ID via direct_csr.
	qmi_hw->direct_csr =
		30 << QMI_DIRECT_CSR_CLKDIV_LSB | QMI_DIRECT_CSR_EN_BITS;
	// Need to poll for the cooldown on the last XIP transfer to expire
	// (via direct-mode BUSY flag) before it is safe to perform the first
	// direct-mode operation
	while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0)
		;

	// Exit out of QMI in case we've inited already
	qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
	// Transmit as quad.
	qmi_hw->direct_tx =
		QMI_DIRECT_TX_OE_BITS |
		QMI_DIRECT_TX_IWIDTH_VALUE_Q << QMI_DIRECT_TX_IWIDTH_LSB | 0xf5;
	while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0)
		;
	(void)qmi_hw->direct_rx;
	qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS);

	// Read the id
	qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
	uint8_t kgd = 0;
	uint8_t eid = 0;
	for (size_t i = 0; i < 7; i++) {
		if (i == 0) {
			qmi_hw->direct_tx = 0x9f;
		} else {
			qmi_hw->direct_tx = 0xff;
		}
		while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_TXEMPTY_BITS) == 0) {
		}
		while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) {
		}
		if (i == 5) {
			kgd = qmi_hw->direct_rx;
		} else if (i == 6) {
			eid = qmi_hw->direct_rx;
		} else {
			(void)qmi_hw->direct_rx;
		}
	}
	// Disable direct csr.
	qmi_hw->direct_csr &=
		~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS | QMI_DIRECT_CSR_EN_BITS);

	printf("kgd: %02x\n", kgd);
	/*if (kgd != 0x5D) {
	    common_hal_mcu_enable_interrupts();
	    reset_pin_number(CIRCUITPY_PSRAM_CHIP_SELECT->number);
	    return;
	}
	never_reset_pin_number(CIRCUITPY_PSRAM_CHIP_SELECT->number);*/

	// Enable quad mode.
	qmi_hw->direct_csr =
		30 << QMI_DIRECT_CSR_CLKDIV_LSB | QMI_DIRECT_CSR_EN_BITS;
	// Need to poll for the cooldown on the last XIP transfer to expire
	// (via direct-mode BUSY flag) before it is safe to perform the first
	// direct-mode operation
	while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) {
	}

	// RESETEN, RESET and quad enable
	for (uint8_t i = 0; i < 3; i++) {
		qmi_hw->direct_csr |= QMI_DIRECT_CSR_ASSERT_CS1N_BITS;
		if (i == 0) {
			qmi_hw->direct_tx = 0x66;
		} else if (i == 1) {
			qmi_hw->direct_tx = 0x99;
		} else {
			qmi_hw->direct_tx = 0x35;
		}
		while ((qmi_hw->direct_csr & QMI_DIRECT_CSR_BUSY_BITS) != 0) {
		}
		qmi_hw->direct_csr &= ~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS);
		for (size_t j = 0; j < 20; j++) {
			asm("nop");
		}
		(void)qmi_hw->direct_rx;
	}
	// Disable direct csr.
	qmi_hw->direct_csr &=
		~(QMI_DIRECT_CSR_ASSERT_CS1N_BITS | QMI_DIRECT_CSR_EN_BITS);

	qmi_hw->m[1].timing =
		QMI_M0_TIMING_PAGEBREAK_VALUE_1024
			<< QMI_M0_TIMING_PAGEBREAK_LSB | // Break between pages.
		3 << QMI_M0_TIMING_SELECT_HOLD_LSB | // Delay releasing CS for 3 extra
	                                         // system cycles.
		3 << QMI_M0_TIMING_COOLDOWN_LSB |
		3 << QMI_M0_TIMING_RXDELAY_LSB |
		32 << QMI_M0_TIMING_MAX_SELECT_LSB | // In units of 64 system clock
	                                         // cycles. PSRAM says 8us max. 8 /
	                                         // 0.00752 / 64 = 16.62
		14 << QMI_M0_TIMING_MIN_DESELECT_LSB | // In units of system clock
	                                           // cycles. PSRAM says 50ns.50
	                                           // / 7.52 = 6.64
		2 << QMI_M0_TIMING_CLKDIV_LSB;
	qmi_hw->m[1].rfmt =
		(QMI_M0_RFMT_PREFIX_WIDTH_VALUE_Q << QMI_M0_RFMT_PREFIX_WIDTH_LSB |
	     QMI_M0_RFMT_ADDR_WIDTH_VALUE_Q << QMI_M0_RFMT_ADDR_WIDTH_LSB |
	     QMI_M0_RFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M0_RFMT_SUFFIX_WIDTH_LSB |
	     QMI_M0_RFMT_DUMMY_WIDTH_VALUE_Q << QMI_M0_RFMT_DUMMY_WIDTH_LSB |
	     QMI_M0_RFMT_DUMMY_LEN_VALUE_24 << QMI_M0_RFMT_DUMMY_LEN_LSB |
	     QMI_M0_RFMT_DATA_WIDTH_VALUE_Q << QMI_M0_RFMT_DATA_WIDTH_LSB |
	     QMI_M0_RFMT_PREFIX_LEN_VALUE_8 << QMI_M0_RFMT_PREFIX_LEN_LSB |
	     QMI_M0_RFMT_SUFFIX_LEN_VALUE_NONE << QMI_M0_RFMT_SUFFIX_LEN_LSB);
	qmi_hw->m[1].rcmd =
		0xeb << QMI_M0_RCMD_PREFIX_LSB | 0 << QMI_M0_RCMD_SUFFIX_LSB;
	qmi_hw->m[1].wfmt =
		(QMI_M0_WFMT_PREFIX_WIDTH_VALUE_Q << QMI_M0_WFMT_PREFIX_WIDTH_LSB |
	     QMI_M0_WFMT_ADDR_WIDTH_VALUE_Q << QMI_M0_WFMT_ADDR_WIDTH_LSB |
	     QMI_M0_WFMT_SUFFIX_WIDTH_VALUE_Q << QMI_M0_WFMT_SUFFIX_WIDTH_LSB |
	     QMI_M0_WFMT_DUMMY_WIDTH_VALUE_Q << QMI_M0_WFMT_DUMMY_WIDTH_LSB |
	     QMI_M0_WFMT_DUMMY_LEN_VALUE_NONE << QMI_M0_WFMT_DUMMY_LEN_LSB |
	     QMI_M0_WFMT_DATA_WIDTH_VALUE_Q << QMI_M0_WFMT_DATA_WIDTH_LSB |
	     QMI_M0_WFMT_PREFIX_LEN_VALUE_8 << QMI_M0_WFMT_PREFIX_LEN_LSB |
	     QMI_M0_WFMT_SUFFIX_LEN_VALUE_NONE << QMI_M0_WFMT_SUFFIX_LEN_LSB);
	qmi_hw->m[1].wcmd =
		0x38 << QMI_M0_WCMD_PREFIX_LSB | 0 << QMI_M0_WCMD_SUFFIX_LSB;

	restore_interrupts(interrupt_state);

	psram_size = 1024 * 1024; // 1 MiB
	uint8_t size_id = eid >> 5;
	if (eid == 0x26 || size_id == 2) {
		psram_size *= 8;
	} else if (size_id == 0) {
		psram_size *= 2;
	} else if (size_id == 1) {
		psram_size *= 4;
	}

	// Mark that we can write to PSRAM.
	xip_ctrl_hw->ctrl |= XIP_CTRL_WRITABLE_M1_BITS;

	return psram_size;
}

void init_emulator(std::size_t psram_size) {
	heap_limit = reinterpret_cast<void *>(Y8_EXTMEM_START + psram_size);

	emu::emulator.init(
		std::span(reinterpret_cast<std::byte *>(Y8_EXTMEM_START), psram_size));
}

void init_video_ssd1351() {
	spi_inst_t *video_spi = spi0;
	spi_init(video_spi, 25 * 1000 * 1000);

	printf("SSD1351 baudrate: %d\n", spi_get_baudrate(video_spi));

	asupico::hw.ssd1351.init(
		{.spi = video_spi,
	     .pinout = {.sclk = 2, .tx = 3, .rst = 4, .cs = 5, .dc = 6}});
}

} // namespace arch::pico::platform::asupico